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
#ifndef __NMEA_0183_HH__
#define __NMEA_0183_HH__

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

namespace NMEA0183 {

  class ChecksumMismatch : public std::exception {
  private:
    unsigned char _computed_cs, _stream_cs;

  public:
    ChecksumMismatch(unsigned char ccs, unsigned char scs) :
      _computed_cs(ccs), _stream_cs(scs)
    {}

    const char* what() const throw() {
      std::ostringstream oss;
      oss.width(2);
      oss << "Checksum mismatch in NMEA-0183 stream - computed 0x" << std::hex << (int)_computed_cs << ", found 0x" << (int)_stream_cs;
      return oss.str().c_str();
    }    
  }; // class ChecksumMismatch

  class Sentence {
  private:
    const char _talker_id[2], _type[3];
    const std::string _data;
    const std::vector<std::string> _fields;
    const unsigned char _checksum;

    unsigned char _generate_checksum(void);
    std::vector<std::string> _split_fields(void);

  public:
    // Constructor
    Sentence(std::string line);

    // Constructor
    Sentence(std::string tid, std::string type, std::string data);
    Sentence(std::string tid, std::string type, std::string data, unsigned char checksum);

    inline const std::string talker_id(void) const { return std::string(_talker_id, 2); }
    inline const std::string type(void) const { return std::string(_type, 3); }
    inline const std::string data(void) const { return _data; }
    inline const size_t num_fields(void) const { return _fields.size(); }
    inline const std::string field(size_t i) const { return _fields[i]; }
    inline const std::vector<std::string> fields(void) const { return _fields; }
    inline const unsigned char checksum(void) const { return _checksum; }

  }; // class Sentence

}; // namespace NMEA0183

#endif // __NMEA_0183_HH__
