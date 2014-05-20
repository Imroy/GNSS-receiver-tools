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
#ifndef __PARSER_HH__
#define __PARSER_HH__

#include <string>
#include <iostream>
#include <fstream>
#include "NMEA-0183.hh"

namespace GPSstatus {

  class App;

  class Parser {
  private:
    std::istream _source;
    bool _running;

    App *_app;

    std::string _prev_type;
    std::vector<NMEA0183::SatelliteData::ptr> _sat_data;

  public:
    Parser(std::string filename);
    ~Parser();

    inline bool running(void) const { return _running; }
    inline void start_running(void) { _running = true; }
    inline void stop_running(void) { _running = false; }

    void init(App* app);

    void get_input(void);

    void cleanup(void);

  }; // class Parser

  int Parser_runner(void* data);

}; // namespace GPSstatus

#endif // __PARSER_HH__
