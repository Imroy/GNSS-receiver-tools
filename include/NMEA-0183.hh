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
#ifndef __NMEA_0183_HH__
#define __NMEA_0183_HH__

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "SkyTraq.hh"

namespace greg = boost::gregorian;
namespace ptime = boost::posix_time;

/*
  Sources:
  https://store-lgdi92x.mybigcommerce.com/content/NMEA_Format_v0.1.pdf	(Skytraq/NavSpark)
  https://store-lgdi92x.mybigcommerce.com/content/SUP800F_v0.6.pdf	(Skytraq SUP800F)
  http://www.gpsinformation.org/dale/nmea.htm
 */


namespace NMEA0183 {

  class InvalidSentence : public std::exception {
  private:

  public:
    InvalidSentence() {}

    const char* what() const throw() {
      return "Invalid sentence";
    }
  }; // class InvalidSentence


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


  class UnknownSentenceType : public std::exception {
  private:
    std::string _tid, _type;

  public:
    UnknownSentenceType(std::string tid, std::string type) :
      _tid(tid), _type(type)
    {}

    const char* what() const throw() {
      std::ostringstream oss;
      oss << "Unrecognised sentence $" << _tid << _type;
      return oss.str().c_str();
    }
  }; // class UnknownSentenceType


#define GETTER(type, name, field) inline const type name(void) const { return field; }

  //! Base class for holding NMEA-0183 sentence data
  class Sentence : public SkyTraq::Message {
  private:
    const char _talker_id[2], _type[3];
    const unsigned char _checksum;

  public:
    //! Constructor
    Sentence(std::string tid, std::string type, unsigned char checksum);

    typedef std::shared_ptr<Sentence> ptr;

    GETTER(std::string, talker_id, std::string(_talker_id, 2));
    GETTER(std::string, type, std::string(_type, 3));
    GETTER(unsigned, char checksum, _checksum);

    //    friend Sentence::ptr parse_sentence(std::string line);
  }; // class Sentence

  //! Sentence parser
  Sentence::ptr parse_sentence(std::string line);


  enum class FixQuality : uint8_t {
    Unavailable = 0,
      SPSmode,
      DGPSmode,
      PPSmode,
      RTKmode,
      FloatRTKmode,
      DeadReckoningMode,
      ManualMode,
      SimulationMode,
  }; // class FixQuality


  //! Global Positioning System fix data
  class GGA : public Sentence {
  private:
    ptime::time_duration _utc_time;
    double _lattitude, _longitude; // north and east are positive, respectively
    FixQuality _fix_quality;
    int _num_sats_used;
    double _hdop;
    double _altitude;		// Metres above mean sea level
    double _geoid_sep;		// Height of geoid (mean sea level) above WGS84 ellipsoid
    int _dgps_update_age;	// Seconds since last DGPS update
    int _dgps_station_id;	// use -1 when no value was given

  public:
    GGA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    GETTER(ptime::time_duration, UTC_time, _utc_time);
    GETTER(double, lattitude, _lattitude);
    GETTER(double, longitude, _longitude);
    GETTER(FixQuality, fix_quality, _fix_quality);
    GETTER(int, num_sats_used, _num_sats_used);
    GETTER(double, HDOP, _hdop);
    GETTER(double, altitude, _altitude);
    GETTER(double, GEOID_separation, _geoid_sep);
    GETTER(int, DGPS_update_age, _dgps_update_age);
    GETTER(int, DGPS_station_id, _dgps_station_id);

  }; // class GGA


  enum class ReceiverMode : uint8_t {
    unknown = 0,
      NotValid,
      Autonomous,
      Differential,
      Estimated,
      Simulated,
  }; // class ReceiverMode


  //! Geographic position - latitude/longitude
  class GLL : public Sentence {
  private:
    double _lattitude, _longitude; // north and east are positive, respectively
    ptime::time_duration _utc_time;
    ReceiverMode _mode;

  public:
    GLL(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    GETTER(double, lattitude, _lattitude);
    GETTER(double, longitude, _longitude);
    GETTER(ptime::time_duration, UTC_time, _utc_time);
    GETTER(ReceiverMode, receiver_mode, _mode);

  }; // class GLL


  enum class OpMode : uint8_t {
    Manual = 0,
      Automatic,
  }; // class OpMode


  enum class FixType : uint8_t {
    NotAvailable = 0,
      TwoDimensional,
      ThreeDimensional,
  }; // class FixType


  //! GNSS DOP and active satellites
  class GSA : public Sentence {
  private:
    OpMode _mode;
    FixType _fixtype;
    std::vector<int> _sat_ids;
    double _pdop, _hdop, _vdop;

  public:
    GSA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    GETTER(OpMode, mode, _mode);
    GETTER(FixType, fix_type, _fixtype);
    GETTER(std::vector<int>, satellite_ids, _sat_ids);
    GETTER(double, PDOP, _pdop);
    GETTER(double, HDOP, _hdop);
    GETTER(double, VDOP, _vdop);

  }; // class GSA


  //! The satellite data from GSV sentences
  struct SatelliteData {
    int id;
    int elevation, azimuth;
    int snr;		// use -1 when no SNR value was given

    inline SatelliteData(int i, int e, int a, int s) :
      id(i), elevation(e), azimuth(a), snr(s)
    {}

    typedef std::shared_ptr<SatelliteData> ptr;
  }; // struct SatelliteData


  //! GNSS satellites in view
  class GSV : public Sentence {
  private:
    int _num_messages, _msg_seq, _sats_in_view;
    std::vector<SatelliteData::ptr> _sat_data;

  public:
    GSV(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    GETTER(int, num_messages, _num_messages);
    GETTER(int, message_seq, _msg_seq);
    GETTER(int, satellites_in_view, _sats_in_view);
    GETTER(std::vector<SatelliteData::ptr>, satellite_data, _sat_data);

  }; // class GSV


  //! Recommended minimum specific GNSS data
  class RMC : public Sentence {
  private:
    ptime::ptime _utc_datetime;
    bool _status;
    double _lattitude, _longitude; // north and east are positive, respectively
    double _speed, _course;
    ReceiverMode _mode;

  public:
    RMC(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    GETTER(ptime::ptime, UTC_datetime, _utc_datetime);
    GETTER(ptime::time_duration, UTC_time, _utc_datetime.time_of_day());
    GETTER(bool, status, _status);
    GETTER(double, lattitude, _lattitude);
    GETTER(double, longitude, _longitude);
    GETTER(double, speed, _speed);
    GETTER(double, course, _course);
    GETTER(greg::date, UTC_date, _utc_datetime.date());
    GETTER(ReceiverMode, receiver_mode, _mode);

  }; // class RMC


  //! Course over ground and ground speed
  class VTG : public Sentence {
  private:
    double _course_true, _course_magnetic;
    double _speed_knots, _speed;
    ReceiverMode _mode;

  public:
    VTG(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    GETTER(double, true_course, _course_true);
    GETTER(double, magnetic_course, _course_magnetic);
    GETTER(double, speed_knots, _speed_knots);
    GETTER(double, speed, _speed);
    GETTER(ReceiverMode, receiver_mode, _mode);

  }; // class VTG


  //! Time and date
  class ZDA : public Sentence {
  private:
    ptime::ptime _utc_datetime;
    int _tzhr, _tzmin;

  public:
    ZDA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    GETTER(ptime::ptime, UTC_datetime, _utc_datetime);
    GETTER(ptime::time_duration, UTC_time, _utc_datetime.time_of_day());
    GETTER(greg::date, UTC_date, _utc_datetime.date());
    GETTER(int, TZ_hours, _tzhr);
    GETTER(int, TZ_minutes, _tzmin);

  }; // class ZDA


  //! 1 PPS timing report
  class STI_PPS : public Sentence {
  private:
    SkyTraq::TimingMode _timing_mode;
    double _survey_length, _quant_error;

  public:
    STI_PPS(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    GETTER(SkyTraq::TimingMode, timing_mode, _timing_mode);
    GETTER(double, survey_length, _survey_length);
    GETTER(double, quant_error, _quant_error);

  }; // class STI_PPS


  //! Sensor data from the SUP800F
  class STI_sensors : public Sentence {
  private:
    double _pitch, _roll, _yaw;
    double _pres, _temp;

  public:
    STI_sensors(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    GETTER(double, pitch, _pitch);
    GETTER(double, roll, _roll);
    GETTER(double, yaw, _yaw);
    GETTER(double, pressure, _pres);
    GETTER(double, temperature, _temp);

  }; // class STI_sensors

#undef GETTER

}; // namespace NMEA0183

#define ENUM_OSTREAM_OPERATOR(type) inline std::ostream& operator<< (std::ostream& out, type val) { out << std::to_string(val); return out;  }

namespace std {
  std::string to_string(NMEA0183::FixQuality quality);
  ENUM_OSTREAM_OPERATOR(NMEA0183::FixQuality);

  std::string to_string(NMEA0183::ReceiverMode mode);
  ENUM_OSTREAM_OPERATOR(NMEA0183::ReceiverMode);

  std::string to_string(NMEA0183::OpMode mode);
  ENUM_OSTREAM_OPERATOR(NMEA0183::OpMode);

  std::string to_string(NMEA0183::FixType type);
  ENUM_OSTREAM_OPERATOR(NMEA0183::FixType);

}; // namespace std

// Undefine our macros here, unless Doxygen is reading this
#ifndef DOXYGEN_SKIP_FOR_USERS
#undef ENUM_OSTREAM_OPERATOR
#endif

#endif // __NMEA_0183_HH__
