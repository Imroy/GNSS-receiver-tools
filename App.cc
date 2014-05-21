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
    _new_sat_data(false),
    _window(NULL),
    _renderer(NULL),
    _sat_surface(NULL),
    _need_redraw(false),
    _font(NULL)
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
      if (_new_sat_data)
	render_satellites();
      if (_new_fix_data)
	render_fix();
      if (_need_redraw)
	Render();
    }
    _parser.stop_running();

    SDL_WaitThread(_parser_thread, NULL);

    Cleanup();

    return 0;
  }

  void App::Init() {
    if (TTF_Init() == -1) {
      std::cerr << "Could not initialise SDL_ttf: " << TTF_GetError() << std::endl;
      exit(1);
    }

    _font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", 10);
    if (!_font) {
      std::cerr << "Could not load font." << std::endl;
      exit(1);
    }

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

    if ((_sat_surface = SDL_CreateRGBSurface(0, 768, 768, 32, 0, 0, 0, 0)) == NULL) {
      std::cerr << "Could not create SDL surface for satellites." << std::endl;
      exit(1);
    }

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

  inline void draw_pixel(SDL_Surface *surface, int x, int y, unsigned colour) {
    unsigned char *pixels = (unsigned char*)surface->pixels + (y * surface->pitch) + (x * surface->format->BytesPerPixel);
    SDL_memset4(pixels, colour, 1);
  }

  inline void draw_pixel(SDL_Surface *surface, int x, int y, SDL_Colour colour) {
    unsigned int col = SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a);
    draw_pixel(surface, x, y, col);
  }

  inline void draw_hline(SDL_Surface *surface, int x1, int x2, int y, unsigned int colour) {
    unsigned char *pixels = (unsigned char*)surface->pixels + (y * surface->pitch) + (x1 * surface->format->BytesPerPixel);
    SDL_memset4(pixels, colour, x2 - x1);
  }

  inline void draw_hline(SDL_Surface *surface, int x1, int x2, int y, SDL_Colour colour) {
    unsigned int col = SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a);
    draw_hline(surface, x1, x2, y, col);
  }

  inline void draw_vline(SDL_Surface *surface, int x, int y1, int y2, unsigned int colour) {
    unsigned char *pixels = (unsigned char*)surface->pixels + (y1 * surface->pitch) + (x * surface->format->BytesPerPixel);
    for (int y = y1; y < y2; y++, pixels += surface->pitch)
      SDL_memset4(pixels, colour, 1);
  }

  inline void draw_vline(SDL_Surface *surface, int x, int y1, int y2, SDL_Colour colour) {
    unsigned int col = SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a);
    draw_vline(surface, x, y1, y2, col);
  }

  void draw_circle(SDL_Surface *surface, double cx, double cy, double radius, SDL_Colour colour) {
    double i = 0, j = radius;
    double e = 1 - radius;
    unsigned int col = SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a);
    while (1) {
      int x1 = floor(cx - i);
      int x2 = floor(cx + i);
      int y1 = floor(cy - j);
      int y2 = floor(cy + j);

      draw_pixel(surface, x1, y1, col);
      draw_pixel(surface, x2, y1, col);
      draw_pixel(surface, x1, y2, col);
      draw_pixel(surface, x2, y2, col);

      int y3 = floor(cy - i);
      if (y1 == y3)
	break;
      int y4 = floor(cy + i);
      if (y2 == y4)
	break;
      int x3 = floor(cx - j);
      int x4 = floor(cx + j);

      draw_pixel(surface, x3, y3, col);
      draw_pixel(surface, x4, y3, col);
      draw_pixel(surface, x3, y4, col);
      draw_pixel(surface, x4, y4, col);

      i++;
      if (e < 0) {
	e += 2 * i + 1;
      } else {
	j--;
	e += 2 * (i - j + 1);
      }
    }
  }

  void draw_filled_circle(SDL_Surface *surface, double cx, double cy, double radius, SDL_Colour colour) {
    double i = 0, j = radius;
    double e = 1 - radius;
    unsigned int col = SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a);
    while (1) {
      int x1 = floor(cx - i);
      int x2 = floor(cx + i);
      int y1 = floor(cy - j);
      int y2 = floor(cy + j);
      draw_hline(surface, x1, x2, y1, col);
      draw_hline(surface, x1, x2, y2, col);

      int y3 = floor(cy - i);
      if (y1 == y3)
	break;
      int y4 = floor(cy + i);
      if (y2 == y4)
	break;
      int x3 = floor(cx - j);
      int x4 = floor(cx + j);
      draw_hline(surface, x3, x4, y3, col);
      draw_hline(surface, x3, x4, y4, col);

      i++;
      if (e < 0) {
	e += 2 * i + 1;
      } else {
	j--;
	e += 2 * (i - j + 1);
      }
    }
  }

  void App::render_satellites(void) {
    SDL_LockSurface(_sat_surface);

    SDL_Colour white = { 255, 255, 255, SDL_ALPHA_OPAQUE };
    draw_circle(_sat_surface, 384, 384, 383.5, white);
    draw_circle(_sat_surface, 384, 384, 255.5, white);
    draw_circle(_sat_surface, 384, 384, 127.5, white);
    draw_vline(_sat_surface, 384, 0, 768, white);
    draw_hline(_sat_surface, 0, 767, 384, white);

    for (auto sat : _sat_data) {
      SDL_Colour colour;
      if (sat->tracking)
	colour = { 0, 255, 0, SDL_ALPHA_OPAQUE };	// green
      else
	colour = { 255, 0, 0, SDL_ALPHA_OPAQUE };	// red

      double radius = (90 - sat->elevation) * 383.5 / 90;
      double cx = 384 + sin(sat->azimuth) * radius;
      double cy = 384 - cos(sat->azimuth) * radius;
      draw_filled_circle(_sat_surface, cx, cy, 7, colour);
    }
    SDL_UnlockSurface(_sat_surface);

    for (auto sat : _sat_data) {
      SDL_Colour colour = { 255, 255, 255, SDL_ALPHA_OPAQUE };	// white
      if (sat->tracking)
	colour.r = colour.g = 0;	// blue

      SDL_Surface *text_surface = TTF_RenderUTF8_Blended(_font, std::to_string(sat->id).c_str(), colour);
      if (text_surface) {
	double radius = (90 - sat->elevation) * 383.5 / 90;
	double cx = 384 + sin(sat->azimuth) * radius;
	double cy = 384 - cos(sat->azimuth) * radius;

	std::cerr << "cx=" << cx << ", cy=" << cy << ", w=" << text_surface->w << ", h=" << text_surface->h << std::endl;
	SDL_Rect destrect = { (int)floor(cx - (text_surface->w * 0.5)), (int)floor(cy - (text_surface->h * 0.5)), text_surface->w, text_surface->h };

	SDL_BlitSurface(text_surface, NULL, _sat_surface, &destrect);
	SDL_FreeSurface(text_surface);
      }
    }

    _need_redraw = true;
  }

  void App::Render(void) {
    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); // black
    SDL_RenderClear(_renderer);

    SDL_Texture *sat_texture = SDL_CreateTextureFromSurface(_renderer, _sat_surface);
    if (sat_texture) {
      SDL_Rect destrect = { (1024 - _sat_surface->w) / 2, (768 - _sat_surface->h) / 2, _sat_surface->w, _sat_surface->h };
      SDL_RenderCopy(_renderer, sat_texture, NULL, &destrect);
      SDL_DestroyTexture(sat_texture);
    }

    SDL_RenderPresent(_renderer);
  }

  void App::new_sat_data(std::vector<NMEA0183::SatelliteData::ptr>& sat_data) {
    std::cerr << sat_data.size() << " satellites in list for display." << std::endl;
    swap(_sat_data, sat_data);
    _new_sat_data = true;
  }

  void App::signal_redraw(void) {
    SDL_LockMutex(_redraw_lock);
    SDL_CondSignal(_redraw_cond);
    SDL_UnlockMutex(_redraw_lock);
  }

  void App::Cleanup() {
    if (_sat_surface)
      SDL_FreeSurface(_sat_surface);
    if (_fix_surface)
      SDL_FreeSurface(_fix_surface);
    if (_renderer)
      SDL_DestroyRenderer(_renderer);
    if (_window)
      SDL_DestroyWindow(_window);
    if (_font)
      TTF_CloseFont(_font);

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
