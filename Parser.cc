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
#include <SDL2/SDL_timer.h>
#include "App.hh"

namespace GPSstatus {

  std::filebuf* open_filebuf(std::string filename) {
    std::filebuf *fb = new std::filebuf;
    if (fb->open(filename, std::ios::in))
      return fb;
    return NULL;
  }

  Parser::Parser(std::string filename) :
    _source(open_filebuf(filename)),
    _running(false)
  {}

  Parser::~Parser() {
    std::filebuf *fb = dynamic_cast<std::filebuf*>(_source.rdbuf());
    fb->close();
    delete fb;
  }

  void Parser::init(SDL_mutex* rm, SDL_cond* rc) {
    _redraw_lock = rm;
    _redraw_cond = rc;

  }

  void Parser::get_input(void) {
    std::string line;
    getline(_source, line);
    if (_source.eof())
      return;

    if ((line.length() > 0) && (line[line.length() - 1] == '\x0d'))
      line.resize(line.length() - 1);

    try {
      auto s = NMEA0183::parse_sentence(line);

      if (s->isa<NMEA0183::GSV>()) {
	// Assume that sentences are grouped together by type for each fix,
	// so if the previous sentence wasn't a GSV, this must be the first for the fix
	if (_prev_type != "GSV") {
	  swap(_sat_data, _wip_sat_data);

	  std::cerr << _sat_data.size() << " satellites in list for display." << std::endl;
	  SDL_LockMutex(_redraw_lock);
	  SDL_CondSignal(_redraw_cond);
	  SDL_UnlockMutex(_redraw_lock);

	  _wip_sat_data.clear();
	}

	NMEA0183::GSV *gsv = s->cast_as<NMEA0183::GSV>();
	if (gsv != NULL) {
	  for (auto sat : gsv->satellite_data())
	    _wip_sat_data.push_back(sat);
	}
      }

      _prev_type = s->type();
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
  }

  void Parser::cleanup(void) {
  }

  int Parser_runner(void* data) {
    Parser *parser = (Parser*)data;
    parser->start_running();
    while (parser->running()) {
      parser->get_input();
    }
    return 0;
  }


}; // namespace GPSstatus
