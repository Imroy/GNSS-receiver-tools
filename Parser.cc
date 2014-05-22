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
    _running(false),
    _app(NULL)
  {}

  Parser::~Parser() {
    std::filebuf *fb = dynamic_cast<std::filebuf*>(_source.rdbuf());
    fb->close();
    delete fb;
  }

  void Parser::init(App* app) {
    _app = app;
  }

  void Parser::get_input(void) {
    if (_source.eof()) {
      stop_running();
      return;
    }

    std::string line;
    getline(_source, line);

    if ((line.length() > 0) && (line[line.length() - 1] == '\x0d'))
      line.resize(line.length() - 1);

    try {
      auto s = NMEA0183::parse_sentence(line);

      if (s->isa<NMEA0183::GGA>()) {
	NMEA0183::GGA *gga = s->cast_as<NMEA0183::GGA>();
	if (gga != NULL) {
	  _app->new_gga_data(gga->lattitude(), gga->longitude(), std::to_string(gga->fix_quality()), gga->num_sats_used(), gga->altitude());
	  _app->signal_redraw();
	}
      }

      if (s->isa<NMEA0183::GSA>()) {
	NMEA0183::GSA *gsa = s->cast_as<NMEA0183::GSA>();
	if (gsa != NULL) {
	  _app->new_gsa_data(std::to_string(gsa->fix_type()), gsa->PDOP(), gsa->HDOP(), gsa->VDOP());
	  _app->signal_redraw();
	}
      }

      if (s->isa<NMEA0183::GSV>()) {
	NMEA0183::GSV *gsv = s->cast_as<NMEA0183::GSV>();
	if (gsv != NULL) {
	  for (auto sat : gsv->satellite_data())
	    _sat_data.push_back(sat);
	}
      } else
	// Assume that sentences are grouped together by type for each fix,
	// so if the previous sentence wasn't a GSV, this must be the first for the fix
	if (_prev_type == "GSV") {
	  _app->new_sat_data(_sat_data);
	  _app->signal_redraw();

	  _sat_data.clear();
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
