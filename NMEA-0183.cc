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
#include "NMEA-0183.hh"

namespace NMEA0183 {

  unsigned char Sentence::_generate_checksum(void) {
    unsigned char cs;
    cs = _talker_id[0];
    cs ^= _talker_id[1];

    cs ^= (unsigned char)_type[0];
    cs ^= (unsigned char)_type[1];
    cs ^= (unsigned char)_type[2];

    for (auto i : _data)
      cs ^= i;

    return cs;
  }

  Sentence::Sentence(std::string line) :
    _talker_id{ line[0], line[1] }, _type{ line[2], line[3], line[4] },
    _data(line.substr(5, line.length() - 3)),
    _checksum(std::stoi(line.substr(line.length() - 2, 2), NULL, 16))
  {
    unsigned char computed_cs = this->_generate_checksum();
    if (computed_cs != _checksum)
      throw ChecksumMismatch(computed_cs, _checksum);
  }

  Sentence::Sentence(std::string tid, std::string type, std::string data) :
    _talker_id{ tid[0], tid[1] }, _type{ type[0], type[1], type[2]},
    _data(data),
    _checksum(this->_generate_checksum())
  {}

  Sentence::Sentence(std::string tid, std::string type, std::string data, unsigned char checksum) :
    _talker_id{ tid[0], tid[1] }, _type{ type[0], type[1], type[2]},
    _data(data),
    _checksum(checksum)
  {}


}; // namespace NMEA0183
