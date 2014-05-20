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
#include <string>
#include <iostream>
#include "NMEA-0183.hh"

int main(int argc, char* argv[]) {
  while (!std::cin.eof()) {
    std::string line;
    getline(std::cin, line);
    if (std::cin.eof())
      break;

    if ((line.length() > 0) && (line[line.length() - 1] == '\x0d'))
      line.resize(line.length() - 1);

    //    std::cerr << "line=\"" << line << "\"" << std::endl;
    try {
      auto s = NMEA0183::parse_sentence(line);

      std::cout << "Talker id=\"" << s->talker_id() << "\", type=\"" << s->type() << "\"" << std::endl;
      if (s->isa<NMEA0183::GGA>()) {
	NMEA0183::GGA *gga = s->cast_as<NMEA0183::GGA>();
	if (gga != NULL) {
	  std::cout << "\tTime " << gga->UTC_time()
		    << ", longitude " << gga->longitude() << "°, lattitude " << gga->lattitude() << "°"
		    << ", \"" << gga->GPS_quality() << "\""
		    << ", " << gga->num_sats_used() << " satellites used"
		    << ", HDOP " << gga->HDOP()
		    << ", altitude " << gga->altitude() << " m"
		    << std::endl;
	}
      } else if (s->isa<NMEA0183::GLL>()) {
	NMEA0183::GLL *gll = s->cast_as<NMEA0183::GLL>();
	if (gll != NULL) {
	  std::cout << "\tLongitude " << gll->longitude() << "°, lattitude " << gll->lattitude() << "°"
		    << ", time " << gll->UTC_time()
		    << ", " << (gll->status() ? "valid" : "not valid")
		    << std::endl;
	}
      } else if (s->isa<NMEA0183::GSA>()) {
	NMEA0183::GSA *gsa = s->cast_as<NMEA0183::GSA>();
	if (gsa != NULL) {
	  std::cout << "\t" << gsa->mode()
		    << ", fix: " << gsa->fix_type()
		    << ", PDOP " << gsa->PDOP() << ", HDOP " << gsa->HDOP() << ", VDOP " << gsa->VDOP()
		    << std::endl;
	  std::cout << "\tSatellites:";
	  for (auto sat : gsa->satellite_ids())
	    std::cout << " " << sat;
	  std::cout << std::endl;
	}
      } else if (s->isa<NMEA0183::GSV>()) {
	NMEA0183::GSV *gsv = s->cast_as<NMEA0183::GSV>();
	if (gsv != NULL) {
	  std::cout << "\t" << gsv->message_seq() << "/" << gsv->num_messages() << " messages"
		    << ", " << gsv->satellites_in_view() << " satellites in view"
		    << std::endl;
	}
      } else if (s->isa<NMEA0183::RMC>()) {
	NMEA0183::RMC *rmc = s->cast_as<NMEA0183::RMC>();
	if (rmc != NULL) {
	  std::cout << "\tTime " << rmc->UTC_time()
		    << ", " << (rmc->status() ? "data valid" : "warning")
		    << ", longitude " << rmc->longitude() << "°, lattitude " << rmc->lattitude() << "°"
		    << ", speed " << rmc->speed() << ", course " << rmc->course()
		    << ", date " << rmc->UTC_day() << "/" << rmc->UTC_month() << "/" << rmc->UTC_year()
		    << ", " << rmc->receiver_mode()
		    << std::endl;
	}
      } else if (s->isa<NMEA0183::VTG>()) {
	NMEA0183::VTG *vtg = s->cast_as<NMEA0183::VTG>();
	if (vtg != NULL) {
	  std::cout << "\tCourse " << vtg->true_course() << "°"
		    << ", speed " << vtg->speed() << " km/h"
		    << ", " << vtg->receiver_mode()
		    << std::endl;
	}
      } else if (s->isa<NMEA0183::ZDA>()) {
	NMEA0183::ZDA *zda = s->cast_as<NMEA0183::ZDA>();
	if (zda != NULL) {
	  std::cout << "\tTime " << zda->UTC_time()
		    << ", date " << zda->UTC_day() << "/" << zda->UTC_month() << "/" << zda->UTC_year()
		    << ", TZ " << zda->TZ_hours() << ":" << zda->TZ_minutes()
		    << std::endl;
	}
      }

      std::cout << std::endl;
    } catch (std::exception &e) {}
  }

  return 0;
}
