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
#include <boost/format.hpp>
#include <SDL2/SDL.h>
#include "App.hh"
#include "Surface-draw.hh"

namespace GPSstatus {

  int font_size = 14;

  App::App(std::string srcname) :
    _running(true),
    _redraw_lock(SDL_CreateMutex()),
    _redraw_cond(SDL_CreateCond()),
    _parser(srcname),
    _new_sat_data(true), _new_fix_data(false), _new_time_data(false),
    _window(NULL),
    _renderer(NULL),
    _hemisphere_surface(NULL), _snr_surface(NULL), _fix_surface(NULL), _time_surface(NULL),
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
      SDL_CondWaitTimeout(_redraw_cond, _redraw_lock, 9);
      SDL_UnlockMutex(_redraw_lock);
      if (_new_sat_data)
	render_satellites();
      if (_new_fix_data)
	render_fix();
      if (_new_time_data)
	render_time();
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

    _font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", font_size);
    if (!_font) {
      std::cerr << "Could not load font." << std::endl;
      exit(1);
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      std::cerr << "Could not initialise SDL." << std::endl;
      exit(1);
    }

    if ((_window = SDL_CreateWindow("GPSstatus", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 896, SDL_WINDOW_OPENGL)) == NULL) {
      std::cerr << "Could not create SDL window." << std::endl;
      exit(1);
    }

    if ((_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
      std::cerr << "Could not create SDL renderer." << std::endl;
      exit(1);
    }
    SDL_RenderClear(_renderer);
    SDL_RenderPresent(_renderer);

    unsigned int rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    if ((_hemisphere_surface = SDL_CreateRGBSurface(0, 782, 768, 32, rmask, gmask, bmask, amask)) == NULL) {
      std::cerr << "Could not create SDL surface for hemisphere." << std::endl;
      exit(1);
    }

    if ((_snr_surface = SDL_CreateRGBSurface(0, 1024, 128, 32, rmask, gmask, bmask, amask)) == NULL) {
      std::cerr << "Could not create SDL surface for signal strengths." << std::endl;
      exit(1);
    }

    if ((_fix_surface = SDL_CreateRGBSurface(0, 384, 128, 32, rmask, gmask, bmask, amask)) == NULL) {
      std::cerr << "Could not create SDL surface for fix info." << std::endl;
      exit(1);
    }

    if ((_time_surface = SDL_CreateRGBSurface(0, 384, 128, 32, rmask, gmask, bmask, amask)) == NULL) {
      std::cerr << "Could not create SDL surface for time info." << std::endl;
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
    SDL_Delay(100);
  }

  void App::render_satellites(void) {
    SDL_FillRect(_hemisphere_surface, NULL, SDL_MapRGBA(_hemisphere_surface->format, 0, 0, 0, 0));

    SDL_LockSurface(_hemisphere_surface);

    SDL_Colour white = { 255, 255, 255, SDL_ALPHA_OPAQUE };
    draw_circle(_hemisphere_surface, 391, 383, 383, white);
    draw_circle(_hemisphere_surface, 391, 383, 255, white);
    draw_circle(_hemisphere_surface, 391, 383, 127, white);
    draw_vline(_hemisphere_surface, 391, 0, 768, white);
    draw_hline(_hemisphere_surface, 8, 774, 384, white);

    std::vector<std::pair<double, double> > circles;
    for (auto sat : _sat_data) {
      SDL_Colour colour;
      if (sat->snr > 0)
	colour = { 0, 255, 0, SDL_ALPHA_OPAQUE };	// green
      else
	colour = { 255, 0, 0, SDL_ALPHA_OPAQUE };	// red

      double radius = (90 - sat->elevation) * 383.0 / 90;
      double cx = 391 + sin(sat->azimuth) * radius;
      double cy = 383 - cos(sat->azimuth) * radius;
      circles.push_back(std::make_pair(cx, cy));
      draw_filled_circle(_hemisphere_surface, cx, cy, 7, colour);
    }
    SDL_UnlockSurface(_hemisphere_surface);

    int i = 0;
    for (auto sat : _sat_data) {
      double cx = circles[i].first, cy = circles[i].second;
      draw_text(_hemisphere_surface, _font, std::to_string(sat->id), cx, cy + 7, white, -0.5, 0);
      i++;
    }

    SDL_FillRect(_snr_surface, NULL, SDL_MapRGBA(_snr_surface->format, 0, 0, 0, 0));
    SDL_LockSurface(_snr_surface);

    double col_width = 1024.0 / _sat_data.size();
    std::vector<std::pair<int, int> > columns;
    i = 0;
    for (auto sat : _sat_data) {
      int x1 = floor(i * col_width);
      int x2 = floor((i + 1) * col_width);
      columns.push_back(std::make_pair(x1, x2));
      draw_box(_snr_surface, x1, 26, x2, 127, white);

      if (sat->snr < 0) {
	i++;
	continue;
      }

      SDL_Colour colour = { 0, 0, 0, SDL_ALPHA_OPAQUE };
      if (sat->snr < 50) {
	colour.r = 255;
	colour.g = sat->snr * 255 / 50;
      } else {
	colour.r = 255 - ((sat->snr - 50) * 255 / 50);
	colour.g = 255;
      }
      draw_filled_box(_snr_surface, x1 + 1, 127 - sat->snr, x2 - 1, 126, colour);
      i++;
    }
    SDL_UnlockSurface(_snr_surface);

    i = 0;
    for (auto sat : _sat_data) {
      int x1 = columns[i].first, x2 = columns[i].second;
      draw_text(_snr_surface, _font, std::to_string(sat->id), (x1 + x2) * 0.5, 127, white, -0.5, -1);
      i++;
    }

    _new_sat_data = false;
    _need_redraw = true;
  }

  std::string degrees_to_dms(double degrees) {
    int d = degrees;
    int m = (degrees - d) * 60;
    double s = (degrees - d - (m / 60.0)) * 3600.0;
    return std::to_string(d) + "° "
      + std::to_string(m) + "′ "
      + boost::str(boost::format("%0.3f") % s) + "″";
  }

  void App::render_fix() {
    SDL_FillRect(_fix_surface, NULL, SDL_MapRGBA(_fix_surface->format, 0, 0, 0, 0));

    SDL_Colour white = { 255, 255, 255, SDL_ALPHA_OPAQUE };
    int line_num = 0;
#define PRINT(x) draw_text(_fix_surface, _font, x, 0, line_num++ * font_size, white)
    PRINT(_fix_quality);
    PRINT("Fix: " + _fix_type);
    PRINT(std::to_string(_num_sats_used) + " used / " + std::to_string(_sat_data.size()) + " satellites");
    PRINT(degrees_to_dms(fabs(_lattitude)) + (_lattitude < 0 ? " S" : " N"));
    PRINT(degrees_to_dms(fabs(_longitude)) + (_longitude < 0 ? " W" : " E"));
    PRINT(boost::str(boost::format("%0.3f") % fabs(_altitude)) + " m " + (_altitude < 0 ? "below" : "above") + " MSL");
    PRINT("PDOP: " + boost::str(boost::format("%0.2f") % _pdop));
    PRINT("HDOP: " + boost::str(boost::format("%0.2f") % _hdop));
    PRINT("VDOP: " + boost::str(boost::format("%0.2f") % _vdop));
#undef PRINT

    _new_fix_data = false;
    _need_redraw = true;
  }

  void App::render_time() {
    SDL_FillRect(_time_surface, NULL, SDL_MapRGBA(_time_surface->format, 0, 0, 0, 0));

    SDL_Colour white = { 255, 255, 255, SDL_ALPHA_OPAQUE };
    int line_num = 0;
#define PRINT(x) draw_text(_time_surface, _font, x, 384, line_num++ * font_size, white, -1, 0)
    int h = floor(_utc_time / 3600);
    int m = floor((_utc_time - h * 3600) / 60);
    double s = _utc_time - h * 3600 - m * 60;
    PRINT(boost::str(boost::format("Time: %02d:%02d:%02.2f UTC") % h % m % s));
    PRINT(boost::str(boost::format("Date: %04d-%02d-%02d  UTC") % _year % _month % _day));
#undef PRINT

    _new_time_data = false;
    _need_redraw = true;
  }

  void App::Render(void) {
    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); // black
    SDL_RenderClear(_renderer);

    SDL_Texture *hemisphere_texture = SDL_CreateTextureFromSurface(_renderer, _hemisphere_surface);
    if (hemisphere_texture) {
      SDL_Rect destrect = { (1024 - _hemisphere_surface->w) / 2, 0, _hemisphere_surface->w, _hemisphere_surface->h };
      SDL_RenderCopy(_renderer, hemisphere_texture, NULL, &destrect);
      SDL_DestroyTexture(hemisphere_texture);
    }

    SDL_Texture *snr_texture = SDL_CreateTextureFromSurface(_renderer, _snr_surface);
    if (snr_texture) {
      SDL_Rect destrect = { (1024 - _snr_surface->w) / 2, 896 - _snr_surface->h, _snr_surface->w, _snr_surface->h };
      SDL_RenderCopy(_renderer, snr_texture, NULL, &destrect);
      SDL_DestroyTexture(snr_texture);
    }

    SDL_Texture *fix_texture = SDL_CreateTextureFromSurface(_renderer, _fix_surface);
    if (fix_texture) {
      SDL_Rect destrect = { 0, 0, _fix_surface->w, _fix_surface->h };
      SDL_RenderCopy(_renderer, fix_texture, NULL, &destrect);
      SDL_DestroyTexture(fix_texture);
    }

    SDL_Texture *time_texture = SDL_CreateTextureFromSurface(_renderer, _time_surface);
    if (time_texture) {
      SDL_Rect destrect = { 1023 - _time_surface->w, 0, _time_surface->w, _time_surface->h };
      SDL_RenderCopy(_renderer, time_texture, NULL, &destrect);
      SDL_DestroyTexture(time_texture);
    }

    SDL_RenderPresent(_renderer);
    _need_redraw = false;
  }

  void App::new_sat_data(std::vector<NMEA0183::SatelliteData::ptr>& sat_data) {
    std::cerr << sat_data.size() << " satellites in list for display." << std::endl;
    swap(_sat_data, sat_data);
    _new_sat_data = true;
  }

  void App::new_gga_data(double la, double lo, std::string q, int ns, double al) {
    _lattitude = la;
    _longitude = lo;
    _fix_quality = q;
    _num_sats_used = ns;
    _altitude = al;
    _new_fix_data = true;
  }

  void App::new_gsa_data(std::string t, double p, double h, double v) {
    _fix_type = t;
    _pdop = p;
    _hdop = h;
    _vdop = v;
    _new_fix_data = true;
  }

  void App::new_zda_data(double t, int d, int m, int y) {
    _utc_time = t;
    _day = d;
    _month = m;
    _year = y;
    _new_time_data = true;
  }

  void App::signal_redraw(void) {
    SDL_LockMutex(_redraw_lock);
    SDL_CondSignal(_redraw_cond);
    SDL_UnlockMutex(_redraw_lock);
  }

  void App::Cleanup() {
    if (_hemisphere_surface)
      SDL_FreeSurface(_hemisphere_surface);
    if (_snr_surface)
      SDL_FreeSurface(_snr_surface);
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
