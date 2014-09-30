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
#include <stdexcept>
#include "GPSNav.hh"

namespace GPS {

  Subframe::ptr parse_subframe(uint8_t prn, const uint8_t *bytes, uint8_t len) {
    if (len < 25)
      throw std::domain_error("Not enough bytes to determine the subframe number");

    uint8_t subframe_num = Subframe::extract_subframe_number(bytes, len);

    if (subframe_num == 1)
      return std::make_shared<Sat_clock_and_health>(prn, bytes, len);

    if (subframe_num == 2)
      return std::make_shared<Ephemeris1>(prn, bytes, len);

    if (subframe_num == 3)
      return std::make_shared<Ephemeris2>(prn, bytes, len);

    uint8_t page_num = Subframe_4_or_5::extract_page_number(bytes, len);

    if (((subframe_num == 4) && (((page_num >= 2) && (page_num <= 5))
				 || ((page_num >= 7) && (page_num <= 10))))
	|| ((subframe_num == 5) && (page_num >= 1) && (page_num <= 24)))
      return std::make_shared<Almanac>(prn, bytes, len);

    if (subframe_num == 4) {
      if (page_num == 18)
	return std::make_shared<Ionosphere_UTC>(prn, bytes, len);
    }

    return std::make_shared<Reserved_and_spare>(prn, bytes, len);
  }


}; // namespace GPS

namespace std {

  std::string to_string(GPS::SignalComponentHealth health) {
    switch (health) {
    case GPS::SignalComponentHealth::All_ok:
      return "all signals ok";
    case GPS::SignalComponentHealth::Is_temporarily_out:
      return "satellite is temporarily out";
    case GPS::SignalComponentHealth::Will_be_temporarily_out:
      return "satellite will be temporarily out";
    case GPS::SignalComponentHealth::Spare:
      return "spare";
    case GPS::SignalComponentHealth::Bad:
      return "more than one combination would be required to describe anomolies";
    case GPS::SignalComponentHealth::Problems:
      return "satellite is experiencing code modulation and/or signal power level transmission problems";
    }
    throw invalid_argument("Unrecognised value for StartMode");
  }

}; // namespace std
