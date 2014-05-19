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
	if (s != NULL) {
	  std::cout << "\tTime " << gga->UTC_time()
		    << ", longitude " << gga->longitude() << ", lattitude " << gga->lattitude()
		    << ", \"" << gga->GPS_quality() << "\""
		    << ", " << gga->num_sats_used() << " satellites used"
		    << ", HDOP " << gga->HDOP()
		    << ", altitude " << gga->altitude() << " m"
		    << std::endl;
	}
      } else if (s->isa<NMEA0183::GLL>()) {
	NMEA0183::GLL *gll = s->cast_as<NMEA0183::GLL>();
	if (s != NULL) {
	  std::cout << "\tLongitude " << gll->longitude() << ", lattitude " << gll->lattitude()
		    << ", time " << gll->UTC_time()
		    << ", " << (gll->status() ? "valid" : "not valid")
		    << std::endl;
	}
      } else if (s->isa<NMEA0183::GSA>()) {
	NMEA0183::GSA *gsa = s->cast_as<NMEA0183::GSA>();
	if (s != NULL) {
	  std::cout << "\t" << gsa->mode()
		    << ", fix: " << gsa->fix_type()
		    << ", PDOP " << gsa->PDOP() << ", HDOP " << gsa->HDOP() << ", VDOP " << gsa->VDOP()
		    << std::endl;
	  std::cout << "\tSatellites:";
	  for (auto sat : gsa->satellite_ids())
	    std::cout << " " << sat;
	  std::cout << std::endl;
	}
      }

      std::cout << std::endl;
    } catch (std::exception &e) {}
  }

  return 0;
}
