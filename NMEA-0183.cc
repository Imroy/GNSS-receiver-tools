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
#include <string.h>

namespace NMEA0183 {

  unsigned char Sentence::_generate_checksum(std::string tid, std::string type, std::string data) {
    unsigned char cs;
    cs = tid[0];
    cs ^= tid[1];

    cs ^= type[0];
    cs ^= type[1];
    cs ^= type[2];

    for (auto i : data)
      cs ^= i;

    return cs;
  }

  std::vector<std::string> Sentence::_split_fields(std::string data) {
    std::vector<std::string> fields;
    size_t field_start = data.find_first_of(',') + 1;
    size_t next_comma;

    while (field_start != std::string::npos) {
      next_comma = data.find_first_of(',', field_start);
      std::string field = data.substr(field_start, next_comma - field_start);
      fields.push_back(field);

      if (next_comma == std::string::npos)
	break;
      field_start = next_comma + 1;
    }

    return fields;
  }

  Sentence::Sentence(std::string tid, std::string type, unsigned char checksum) :
    _talker_id{ tid[0], tid[1] }, _type{ type[0], type[1], type[2]},
    _checksum(checksum)
  {}

  Sentence::ptr parse_sentence(std::string line) {
    std::string tid = line.substr(1, 2);
    std::string type = line.substr(3, 3);
    std::string data = line.substr(6, line.length() - 9);
    unsigned char checksum = std::stoi(line.substr(line.length() - 2, 2), NULL, 16);

    unsigned char computed_cs = Sentence::_generate_checksum(tid, type, data);
    if (computed_cs != checksum)
      throw ChecksumMismatch(computed_cs, checksum);

    auto fields = Sentence::_split_fields(data);

    if ((type == "GGA") &&
	((tid == "GP") || (tid == "GN")))
      return std::make_shared<GGA>(tid, type, fields, checksum);

    if ((type == "GLL") &&
	((tid == "GP") || (tid == "GN")))
      return std::make_shared<GLL>(tid, type, fields, checksum);

    return std::make_shared<Sentence>(tid, type, checksum);
  }

  //! Convert the "hhmmss.ss" string to a number of seconds
  double hhmmss_to_seconds(std::string hhmmss) {
    int hours = std::stoi(hhmmss.substr(0, 2));
    int minutes = std::stoi(hhmmss.substr(2, 2));
    double seconds = std::stod(hhmmss.substr(4));

    return seconds + (minutes * 60.0) + (hours * 3600.0);
  }

  //! Convert the "ddmm.mm" or "dddmm.mm" string to a number of degrees
  /*!
    \param dm The string to parse
    \param dlen The length of the degrees part, either 2 (lattitude) or 3 (longitude)
    \param indicator The north/south or east/west indicator string
    \param neg The indicator string that makes it negative e.g "S" or "W"
   */
  double dm_to_degrees(std::string dm, int dlen, std::string indicator, std::string neg) {
    int degrees = std::stoi(dm.substr(0, dlen));
    double minutes = std::stod(dm.substr(dlen));

    return (degrees + (minutes / 60.0)) * (indicator == neg ? -1 : 1);
  }

  std::ostream& operator<< (std::ostream& out, GPSquality quality) {
    switch (quality) {
    case GPSquality::Unavailable:
      out << "position fix unavailable";
      break;
    case GPSquality::SPSmode:
      out << "valid position fix, SPS mode";
      break;
    case GPSquality::DGPSmode:
      out << "valid position fix, DGPS mode";
      break;
    }

    return out;
  }


  GGA::GGA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _utc_time(hhmmss_to_seconds(fields[0])),
    _lattitude(dm_to_degrees(fields[1], 2, fields[2], "S")),
    _longitude(dm_to_degrees(fields[3], 3, fields[4], "W")),
    _gps_quality((GPSquality)std::stoi(fields[5])),
    _num_sats_used(std::stoi(fields[6])),
    _hdop(std::stod(fields[7])),
    _altitude(std::stod(fields[8])),
    _geoid_sep(std::stod(fields[10])),
    _dgps_station_id(std::stoi(fields[13]))
  {}


  GLL::GLL(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _lattitude(dm_to_degrees(fields[0], 2, fields[1], "S")),
    _longitude(dm_to_degrees(fields[2], 3, fields[3], "W")),
    _utc_time(hhmmss_to_seconds(fields[4])),
    _status(fields[5] == "A")
  {}



}; // namespace NMEA0183
