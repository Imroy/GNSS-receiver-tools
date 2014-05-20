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

  App::App() :
    _running(true),
    _parser("/dev/ttyUSB0"),
    _window(NULL),
    _renderer(NULL)
  {}

  int App::Execute() {
    Init();

    while (_running) {
      Render();

      SDL_Event Event;
      while (SDL_PollEvent(&Event)) {
	OnEvent(&Event);
      }

      Loop();
      SDL_LockMutex(_redraw_lock);
      SDL_CondWaitTimeout(_redraw_cond, _redraw_lock, 999);
      SDL_UnlockMutex(_redraw_lock);
    }
    _parser.stop_running();

    SDL_WaitThread(_parser_thread, NULL);

    Cleanup();

    return 0;
  }

  void App::Init() {
    _redraw_lock = SDL_CreateMutex();
    _redraw_cond = SDL_CreateCond();

    _parser.init(_redraw_lock, _redraw_cond);

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

    default:
      break;
    }
  }

  void App::Loop() {
  }

  void draw_circle(SDL_Renderer *renderer, double cx, double cy, double radius, int num_points) {
    std::vector<std::pair<int, int> > points(num_points);

    for (int i = 0; i < num_points; i++) {
      double angle = i * 2 * M_PI / num_points;
      int x = floor(cx + 0.5 + sin(angle) * radius);
      int y = floor(cy + 0.5 - cos(angle) * radius);
      points[i] = std::make_pair(x, y);
    }

    for (int i = 0; i < num_points - 1; i++)
      SDL_RenderDrawLine(renderer, points[i].first, points[i].second, points[i+1].first, points[i+1].second);
    SDL_RenderDrawLine(renderer, points[num_points - 1].first, points[num_points - 1].second, points[0].first, points[0].second);
  }

  void App::Render() {
    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); // black
    SDL_RenderClear(_renderer);

    SDL_SetRenderDrawColor(_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); // white
    draw_circle(_renderer, 512, 384, 383, 100);
    draw_circle(_renderer, 512, 384, 255, 67);
    draw_circle(_renderer, 512, 384, 127, 33);
    SDL_RenderDrawLine(_renderer, 512, 1, 512, 767);
    SDL_RenderDrawLine(_renderer, 128, 384, 896, 384);

    for (auto sat : _parser.satellite_data()) {
      double radius = (90 - sat->elevation) * 383.0 / 90;
      int x = floor(512 + 0.5 + sin(sat->azimuth) * radius);
      int y = floor(384 + 0.5 - cos(sat->azimuth) * radius);

      if (sat->tracking)
	SDL_SetRenderDrawColor(_renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);	// green
      else
	SDL_SetRenderDrawColor(_renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);	// red

      SDL_Rect box;
      box.x = x - 5;
      box.y = y - 5;
      box.w = box.h = 10;
      SDL_RenderFillRect(_renderer, &box);
    }

    SDL_RenderPresent(_renderer);
  }

  void App::Cleanup() {
    if (_renderer)
      SDL_DestroyRenderer(_renderer);
    if (_window)
      SDL_DestroyWindow(_window);

    _parser.cleanup();

    SDL_Quit();
  }

}; // namespace GPSstatus

int main(int argc, char* argv[]) {
  GPSstatus::App app;

  return app.Execute();
}
