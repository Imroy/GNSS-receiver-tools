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
  https://store-lgdi92x.mybigcommerce.com/content/NMEA_Format_v0.1.pdf	(Skytraq/NavSpark)
  https://store-lgdi92x.mybigcommerce.com/content/SUP800F_v0.6.pdf	(Skytraq SUP800F)
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
      switch (std::stoi(fields[0])) {
      case 1:
	return std::make_shared<STI_PPS>(tid, type, fields, checksum);

      case 4:
	switch (std::stoi(fields[1])) {
	case 1:
	  return std::make_shared<STI_sensors>(tid, type, fields, checksum);

	}
      }

    throw UnknownSentenceType(tid, type);
  }


  //! Convert the "hhmmss.ss" string to a time_duration object
  ptime::time_duration hhmmss_to_duration(std::string hhmmss) {
    int hours = std::stoi(hhmmss.substr(0, 2));
    int minutes = std::stoi(hhmmss.substr(2, 2));
    double seconds = std::stod(hhmmss.substr(4));

    return ptime::time_duration(hours, minutes, seconds);
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


  GGA::GGA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _utc_time(hhmmss_to_duration(fields[0])),
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
    _utc_time(hhmmss_to_duration(fields[4])),
    _mode(read_receivermode(fields[5]))
  {}


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


  RMC::RMC(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _utc_datetime(greg::date(std::stoi(fields[8].substr(4, 2)) + 2000,	// NOTE: two-digit year!
			     std::stoi(fields[8].substr(2, 2)),
			     std::stoi(fields[8].substr(0, 2))),
		  hhmmss_to_duration(fields[0])),
    _status(fields[1] == "A"),
    _lattitude(dm_to_degrees(fields[2], 2, fields[3], "S")),
    _longitude(dm_to_degrees(fields[4], 3, fields[5], "W")),
    _speed(std::stod(fields[6])),
    _course(std::stod(fields[7])),
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
    _utc_datetime(greg::date(std::stoi(fields[3]), std::stoi(fields[2]), std::stoi(fields[1])),
		  hhmmss_to_duration(fields[0])),
    _tzhr(std::stoi(fields[4])),
    _tzmin(std::stoi(fields[5]))
  {}


  STI_PPS::STI_PPS(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _timing_mode((SkyTraq::TimingMode)std::stoi(fields[1])),
    _survey_length((fields.size() > 2 && fields[2].length() > 0) ? std::stod(fields[2]) : 0),
    _quant_error((fields.size() > 3 && fields[3].length() > 0) ? std::stod(fields[3]) : -1e+9)
  {}


  STI_sensors::STI_sensors(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum) :
    Sentence(tid, type, checksum),
    _pitch(std::stod(fields[2])),
    _roll(std::stod(fields[3])),
    _yaw(std::stod(fields[4])),
    _pres(std::stod(fields[5])),
    _temp(std::stod(fields[6]))
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
    throw invalid_argument("Unrecognised value for FixQuality");
  }

  std::string to_string(NMEA0183::OpMode mode) {
    switch (mode) {
    case NMEA0183::OpMode::Manual:
      return "manual";
    case NMEA0183::OpMode::Automatic:
      return "automatic";
    }
    throw invalid_argument("Unrecognised value for OpMode");
  }

  std::string to_string(NMEA0183::ReceiverMode mode) {
    switch (mode) {
    case NMEA0183::ReceiverMode::unknown:
      return "";
    case NMEA0183::ReceiverMode::NotValid:
      return "data not valid";
    case NMEA0183::ReceiverMode::Autonomous:
      return "autonomous mode";
    case NMEA0183::ReceiverMode::Differential:
      return "differential mode";
    case NMEA0183::ReceiverMode::Estimated:
      return "estimated mode";
    case NMEA0183::ReceiverMode::Simulated:
      return "simulated mode";
    }
    throw invalid_argument("Unrecognised value for ReceiverMode");
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
    throw invalid_argument("Unrecognised value for FixType");
  }


}; // namespace std
