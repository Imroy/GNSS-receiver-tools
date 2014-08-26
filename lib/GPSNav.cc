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
#include "GPSNav.hh"

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
