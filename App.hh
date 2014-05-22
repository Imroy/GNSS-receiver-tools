/*
        Copyright 2014 Ian Tester

        This file is part of NavSpark tools.

        Photo Finish is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        Photo Finish is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with Photo Finish.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __APP_HH__
#define __APP_HH__

#include <map>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_ttf.h>
#include "Parser.hh"
#include "NMEA-0183.hh"

namespace GPSstatus {

  class App {
  private:
    bool _running;

    SDL_mutex *_redraw_lock;
    SDL_cond *_redraw_cond;

    Parser _parser;
    SDL_Thread *_parser_thread;

    // GGA data
    std::vector<NMEA0183::SatelliteData::ptr> _sat_data;
    bool _new_sat_data;

    // GSA data
    double _lattitude, _longitude, _altitude;
    std::string _fix_quality;
    int _num_sats_used;
    std::string _fix_type;
    double _pdop, _hdop, _vdop;
    bool _new_fix_data;

    // ZDA data
    double _utc_time;
    int _day, _month, _year;
    bool _new_time_data;

    SDL_Window *_window;
    SDL_Renderer *_renderer;

    SDL_Surface *_sat_surface, *_fix_surface, *_time_surface;
    bool _need_redraw;

    TTF_Font *_font;

    void Init();

    void OnEvent(SDL_Event* Event);

    void Loop();

    void render_satellites();
    void render_fix();
    void render_time();
    void Render();

    void Cleanup();

  public:
    App(std::string srcname);

    void new_sat_data(std::vector<NMEA0183::SatelliteData::ptr>& sat_data);
    void new_gga_data(double la, double lo, std::string q, int ns, double al);
    void new_gsa_data(std::string t, double p, double h, double v);
    void new_zda_data(double t, int d, int m, int y);
    void signal_redraw(void);

    int Execute();
  };

}; // namespace GPSstatus

#endif // __APP_HH__
