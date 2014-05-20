/*
        Copyright 2014 Ian Tester

        This file is part of NavSpark tools.

        NavSpark tools is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        NavSpark tools is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with NavSpark tools.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <iostream>
#include <SDL2/SDL.h>
#include "App.hh"

namespace GPSstatus {

  App::App(std::string srcname) :
    _running(true),
    _redraw_lock(SDL_CreateMutex()),
    _redraw_cond(SDL_CreateCond()),
    _parser(srcname),
    _new_data(false),
    _window(NULL),
    _renderer(NULL)
  {}

  int App::Execute() {
    Init();
    Render();

    while (_running) {
      SDL_Event Event;
      while (SDL_PollEvent(&Event)) {
	OnEvent(&Event);
      }

      Loop();
      SDL_LockMutex(_redraw_lock);
      SDL_CondWaitTimeout(_redraw_cond, _redraw_lock, 99);
      SDL_UnlockMutex(_redraw_lock);
      if (_new_data)
	Render();
    }
    _parser.stop_running();

    SDL_WaitThread(_parser_thread, NULL);

    Cleanup();

    return 0;
  }

  void App::Init() {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      std::cerr << "Could not initialise SDL." << std::endl;
      exit(1);
    }

    if ((_window = SDL_CreateWindow("GPSstatus", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_OPENGL)) == NULL) {
      std::cerr << "Could not create SDL window." << std::endl;
      exit(1);
    }

    if ((_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
      std::cerr << "Could not create SDL renderer." << std::endl;
      exit(1);
    }
    SDL_RenderClear(_renderer);
    SDL_RenderPresent(_renderer);

    _parser.init(this);

    _parser_thread = SDL_CreateThread(Parser_runner, "Parser", (void*)&_parser);
    if (_parser_thread == NULL) {
      std::cerr << "Could not create Parser thread." << std::endl;
      exit(1);
    }
  }

  void App::OnEvent(SDL_Event* Event) {
    switch (Event->type) {
    case SDL_QUIT:
      _running = false;
      break;

    case SDL_KEYDOWN:
    case SDL_KEYUP:
      if (Event->key.keysym.scancode == SDL_SCANCODE_Q)
	_running = false;
      break;

    case SDL_WINDOWEVENT:
      switch (Event->window.event) {
      case SDL_WINDOWEVENT_CLOSE:
	_running = false;
	break;

      case SDL_WINDOWEVENT_SHOWN:
      case SDL_WINDOWEVENT_EXPOSED:
      case SDL_WINDOWEVENT_RESTORED:
	SDL_RenderPresent(_renderer);
	break;
      }


    default:
      break;
    }
  }

  void App::Loop() {
  }

  void draw_circle(SDL_Renderer *renderer, double cx, double cy, double radius) {
    double i = 0, j = radius;
    double e = 1 - radius;
    while (j > i) {
      int x1 = floor(cx - i);
      int x2 = floor(cx + i);
      int y1 = floor(cy - j);
      int y2 = floor(cy + j);

      SDL_RenderDrawPoint(renderer, x1, y1);
      SDL_RenderDrawPoint(renderer, x2, y1);
      SDL_RenderDrawPoint(renderer, x1, y2);
      SDL_RenderDrawPoint(renderer, x2, y2);

      int x3 = floor(cx - j);
      int x4 = floor(cx + j);
      int y3 = floor(cy - i);
      int y4 = floor(cy + i);

      SDL_RenderDrawPoint(renderer, x3, y3);
      SDL_RenderDrawPoint(renderer, x4, y3);
      SDL_RenderDrawPoint(renderer, x3, y4);
      SDL_RenderDrawPoint(renderer, x4, y4);

      i++;
      if (e < 0) {
	e += 2 * i + 1;
      } else {
	j--;
	e += 2 * (i - j + 1);
      }
    }
  }

  void draw_filled_circle(SDL_Renderer *renderer, double cx, double cy, double radius) {
    double i = 0, j = radius;
    double e = 1 - radius;
    while (j > i) {
      int x1 = floor(cx - i);
      int x2 = floor(cx + i);
      int y1 = floor(cy - j);
      int y2 = floor(cy + j);
      SDL_RenderDrawLine(renderer, x1, y1, x2, y1);
      SDL_RenderDrawLine(renderer, x1, y2, x2, y2);

      int x3 = floor(cx - j);
      int x4 = floor(cx + j);
      int y3 = floor(cy - i);
      int y4 = floor(cy + i);
      SDL_RenderDrawLine(renderer, x3, y3, x4, y3);
      SDL_RenderDrawLine(renderer, x3, y4, x4, y4);

      i++;
      if (e < 0) {
	e += 2 * i + 1;
      } else {
	j--;
	e += 2 * (i - j + 1);
      }
    }
  }

  void App::Render() {
    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); // black
    SDL_RenderClear(_renderer);

    SDL_SetRenderDrawColor(_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); // white
    draw_circle(_renderer, 512, 384, 383.5);
    draw_circle(_renderer, 512, 384, 255.5);
    draw_circle(_renderer, 512, 384, 127.5);
    SDL_RenderDrawLine(_renderer, 512, 0, 512, 768);
    SDL_RenderDrawLine(_renderer, 128, 384, 895, 384);

    for (auto sat : _sat_data) {
      if (sat->tracking)
	SDL_SetRenderDrawColor(_renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);	// green
      else
	SDL_SetRenderDrawColor(_renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);	// red

      double radius = (90 - sat->elevation) * 383.5 / 90;
      double x = 512 + sin(sat->azimuth) * radius;
      double y = 384 - cos(sat->azimuth) * radius;
      draw_filled_circle(_renderer, x, y, 5);
    }

    SDL_RenderPresent(_renderer);
    _new_data = false;
  }

  void App::new_sat_data(std::vector<NMEA0183::SatelliteData::ptr>& sat_data) {
    std::cerr << sat_data.size() << " satellites in list for display." << std::endl;
    swap(_sat_data, sat_data);
    _new_data = true;
  }

  void App::signal_redraw(void) {
    SDL_LockMutex(_redraw_lock);
    SDL_CondSignal(_redraw_cond);
    SDL_UnlockMutex(_redraw_lock);
  }

  void App::Cleanup() {
    if (_renderer)
      SDL_DestroyRenderer(_renderer);
    if (_window)
      SDL_DestroyWindow(_window);
    SDL_DestroyCond(_redraw_cond);
    SDL_DestroyMutex(_redraw_lock);

    _parser.cleanup();

    SDL_Quit();
  }

}; // namespace GPSstatus

int main(int argc, char* argv[]) {
  GPSstatus::App app;
  std::string inname = "/dev/ttyUSB0"; // default source of NMEA-0183 sentences
  if (argc > 1)
    inname = argv[1];

  GPSstatus::App app(inname);
  return app.Execute();
}
