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

/*
  Sources:
  https://store-lgdi92x.mybigcommerce.com/content/NMEA_Format_v0.1.pdf (Skytraq/NavSpark)
  http://www.gpsinformation.org/dale/nmea.htm
 */

namespace NMEA0183 {

  unsigned char generate_checksum(std::string tid, std::string type, std::string data) {
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


  std::vector<std::string> split_fields(std::string data) {
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
    if (line[0] != '$')
      throw InvalidSentence();

    size_t first_comma = line.find_first_of(',');
    std::string tid = line.substr(1, first_comma - 4);
    std::string type = line.substr(first_comma - 3, 3);
    std::string data = line.substr(first_comma, line.length() - first_comma - 3);

    size_t asterisk = line.find_last_of('*');
    if (asterisk == line.npos)
      throw InvalidSentence();

    unsigned char checksum = std::stoi(line.substr(asterisk + 1, line.length() - asterisk - 1), nullptr, 16);
    unsigned char computed_cs = generate_checksum(tid, type, data);
    if (computed_cs != checksum)
      throw ChecksumMismatch(computed_cs, checksum);

    auto fields = split_fields(data);

    if ((type == "GGA") &&
	((tid == "GP") || (tid == "GN")))
      return std::make_shared<GGA>(tid, type, fields, checksum);

    if ((type == "GLL") &&
	((tid == "GP") || (tid == "GN")))
      return std::make_shared<GLL>(tid, type, fields, checksum);

    if ((type == "GSA") &&
	((tid == "GP") || (tid == "GN") || (tid == "BD") || (tid == "GL")))
      return std::make_shared<GSA>(tid, type, fields, checksum);

    if ((type == "GSV") &&
	((tid == "GP") || (tid == "BD") || (tid == "GL")))
      return std::make_shared<GSV>(tid, type, fields, checksum);

    if ((type == "RMC") &&
	((tid == "GP") || (tid == "GN")))
      return std::make_shared<RMC>(tid, type, fields, checksum);

    if ((type == "VTG") &&
	((tid == "GP") || (tid == "GN")))
      return std::make_shared<VTG>(tid, type, fields, checksum);

    if ((type == "ZDA") &&
	((tid == "GP") || (tid == "GN")))
      return std::make_shared<ZDA>(tid, type, fields, checksum);

    if ((type == "STI") &&
	(tid == "P"))
      return std::make_shared<STI>(tid, type, fields, checksum);

    throw UnknownSentenceType(tid, type);
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


  std::ostream& operator<< (std::ostream& out, FixQuality quality) {
    out << std::to_string(quality);
    return out;
  }


  GGA::GGA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _utc_time(hhmmss_to_seconds(fields[0])),
    _lattitude(dm_to_degrees(fields[1], 2, fields[2], "S")),
    _longitude(dm_to_degrees(fields[3], 3, fields[4], "W")),
    _fix_quality((FixQuality)std::stoi(fields[5])),
    _num_sats_used(std::stoi(fields[6])),
    _hdop(std::stod(fields[7])),
    _altitude(std::stod(fields[8])),
    _geoid_sep(std::stod(fields[10])),
    _dgps_update_age(fields[12].length() > 0 ? std::stoi(fields[12]) : -1),
    _dgps_station_id(fields[13].length() > 0 ? std::stoi(fields[13]) : -1)
  {}


  ReceiverMode read_receivermode(std::string field) {
    ReceiverMode mode = ReceiverMode::unknown;

    if (field.length() > 0) {
      if (field == "A")
	mode = ReceiverMode::Autonomous;
      else if (field == "D")
	mode = ReceiverMode::Differential;
      else if (field == "E")
	mode = ReceiverMode::Estimated;
      else if (field == "S")
	mode = ReceiverMode::Simulated;
    }

    return mode;
  }


  GLL::GLL(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _lattitude(dm_to_degrees(fields[0], 2, fields[1], "S")),
    _longitude(dm_to_degrees(fields[2], 3, fields[3], "W")),
    _utc_time(hhmmss_to_seconds(fields[4])),
    _mode(read_receivermode(fields[5]))
  {}


  std::ostream& operator<< (std::ostream& out, OpMode mode) {
    switch (mode) {
    case OpMode::Manual:
      out << "manual";
      break;
    case OpMode::Automatic:
      out << "automatic";
      break;
    }
    return out;
  }


  std::ostream& operator<< (std::ostream& out, FixType type) {
    out << std::to_string(type);
    return out;
  }


  GSA::GSA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _mode(fields[0] == "A" ? OpMode::Automatic : OpMode::Manual),
    _fixtype((FixType)std::stoi(fields[1])),
    _pdop(std::stod(fields[fields.size() - 3])),
    _hdop(std::stod(fields[fields.size() - 2])),
    _vdop(std::stod(fields[fields.size() - 1]))
  {
    for (unsigned int i = 2; i < fields.size() - 3; i++)
      if (fields[i].length() > 0)
	_sat_ids.push_back(std::stoi(fields[i]));
  }


  GSV::GSV(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _num_messages(std::stoi(fields[0])),
    _msg_seq(std::stoi(fields[1])),
    _sats_in_view(std::stoi(fields[2]))
  {
    for (unsigned int i = 3; i < fields.size(); i += 4) {
      int snr = -1;
      if (fields[i + 3].size() > 0)
	snr = std::stoi(fields[i + 3]);
      _sat_data.push_back(std::make_shared<SatelliteData>(std::stoi(fields[i]),
							  std::stoi(fields[i + 1]),
							  std::stoi(fields[i + 2]),
							  snr));
    }
  }


  std::ostream& operator<< (std::ostream& out, ReceiverMode mode) {
    switch (mode) {
    case ReceiverMode::unknown:
      break;
    case ReceiverMode::NotValid:
      out << "data not valid";
      break;
    case ReceiverMode::Autonomous:
      out << "autonomous mode";
      break;
    case ReceiverMode::Differential:
      out << "differential mode";
      break;
    case ReceiverMode::Estimated:
      out << "estimated mode";
      break;
    case ReceiverMode::Simulated:
      out << "simulated mode";
      break;
    }
    return out;
  }


  RMC::RMC(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _utc_time(hhmmss_to_seconds(fields[0])),
    _status(fields[1] == "A"),
    _lattitude(dm_to_degrees(fields[2], 2, fields[3], "S")),
    _longitude(dm_to_degrees(fields[4], 3, fields[5], "W")),
    _speed(std::stod(fields[6])),
    _course(std::stod(fields[7])),
    _day(std::stoi(fields[8].substr(0, 2))),
    _month(std::stoi(fields[8].substr(2, 2))),
    _year(std::stoi(fields[8].substr(4, 2))),
    _mode(read_receivermode(fields[11]))
  {}


  VTG::VTG(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _course_true(std::stod(fields[0])),
    _course_magnetic(fields[2].length() > 0 ? std::stod(fields[2]) : -1.0),
    _speed_knots(std::stod(fields[4])),
    _speed(std::stod(fields[6])),
    _mode(read_receivermode(fields[8]))
  {}


  ZDA::ZDA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _utc_time(hhmmss_to_seconds(fields[0])),
    _day(std::stoi(fields[1])),
    _month(std::stoi(fields[2])),
    _year(std::stoi(fields[3])),
    _tzhr(std::stoi(fields[4])),
    _tzmin(std::stoi(fields[5]))
  {}


  std::ostream& operator<< (std::ostream& out, PPSmode mode) {
    out << std::to_string(mode);
    return out;
  }


  STI::STI(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _proprietary(std::stoi(fields[0])),
    _ppsmode((PPSmode)std::stoi(fields[1])),
    _survey_length((fields.size() > 2 && fields[2].length() > 0) ? std::stod(fields[2]) : 0),
    _quant_error((fields.size() > 3 && fields[3].length() > 0) ? std::stod(fields[3]) : -1e+9)
  {}


}; // namespace NMEA0183

namespace std {

  std::string to_string(NMEA0183::FixQuality quality) {
    switch (quality) {
    case NMEA0183::FixQuality::Unavailable:
      return "position fix unavailable";
    case NMEA0183::FixQuality::SPSmode:
      return "valid position fix, SPS mode";
    case NMEA0183::FixQuality::DGPSmode:
      return "valid position fix, DGPS mode";
    case NMEA0183::FixQuality::PPSmode:
      return "valid position fix, PPS mode";
    case NMEA0183::FixQuality::RTKmode:
      return "valid position fix, RTK mode";
    case NMEA0183::FixQuality::FloatRTKmode:
      return "valid position fix, float RTK mode";
    case NMEA0183::FixQuality::DeadReckoningMode:
      return "valid position fix, dead reckoning mode";
    case NMEA0183::FixQuality::ManualMode:
      return "valid position fix, manual input mode";
    case NMEA0183::FixQuality::SimulationMode:
      return "valid position fix, simulation mode";
    }
    return "";
  }

  std::string to_string(NMEA0183::FixType type) {
    switch (type) {
    case NMEA0183::FixType::NotAvailable:
      return "not available";
    case NMEA0183::FixType::TwoDimensional:
      return "2D";
    case NMEA0183::FixType::ThreeDimensional:
      return "3D";
    }
    return "";
  }

  std::string to_string(NMEA0183::PPSmode mode) {
    switch (mode) {
    case NMEA0183::PPSmode::PVT:
      return "PVT";
    case NMEA0183::PPSmode::Survey:
      return "survey";
    case NMEA0183::PPSmode::Static:
      return "static";
    }
    return "";
  }

}; // namespace std
