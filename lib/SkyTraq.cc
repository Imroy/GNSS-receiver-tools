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
#include "SkyTraq.hh"

/*
  Sources:
  https://store-lgdi92x.mybigcommerce.com/content/AN0028_1.4.31.pdf	(Binary messages of Skytraq Venus 8)
  https://store-lgdi92x.mybigcommerce.com/content/Venus838LPx-T_DS_v2.pdf (Venus838LPx-T datasheet)
 */

namespace SkyTraq {

  std::ostream& operator<< (std::ostream& out, TimingMode tm) {
    out << std::to_string(tm);
    return out;
  }


}; // namespace SkyTraq

namespace std {

  std::string to_string(SkyTraq::TimingMode tm) {
    switch (tm) {
    case SkyTraq::TimingMode::PVT:
      return "PVT";
    case SkyTraq::TimingMode::Survey:
      return "survey";
    case SkyTraq::TimingMode::Static:
      return "static";
    }
    throw invalid_argument("Unrecognised value for TimingMode");
  }

}; // namespace std
