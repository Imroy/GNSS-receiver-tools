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
#include <memory>

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

  //! Base class for holding NMEA-0183 sentence data
  class Sentence {
  private:
    const char _talker_id[2], _type[3];
    const unsigned char _checksum;

    static unsigned char _generate_checksum(std::string tid, std::string type, std::string data);
    static std::vector<std::string> _split_fields(std::string data);

  public:
    //! Constructor
    Sentence(std::string tid, std::string type, unsigned char checksum);

    //! Virtual destructor to force polymorphism
    inline virtual ~Sentence() {}

    typedef std::shared_ptr<Sentence> ptr;

    inline const std::string talker_id(void) const { return std::string(_talker_id, 2); }
    inline const std::string type(void) const { return std::string(_type, 3); }
    inline const unsigned char checksum(void) const { return _checksum; }

    //! Check the type of an object
    template <typename T>
    inline bool isa(void) const { return typeid(*this) == typeid(T); }

    //! Recast this object to another type
    template <typename T>
    inline T* cast_as(void) { return dynamic_cast<T*>(this); }

    friend Sentence::ptr parse_sentence(std::string line);
  }; // class Sentence

  //! Sentence parser
  Sentence::ptr parse_sentence(std::string line);


  enum class FixQuality {
    Unavailable,
      SPSmode,
      DGPSmode,
      PPSmode,
      RTKmode,
      FloatRTKmode,
      DeadReckoningMode,
      ManualMode,
      SimulationMode,
  }; // class FixQuality

  std::ostream& operator<< (std::ostream& out, FixQuality quality);


  //! Global Positioning System fix data
  class GGA : public Sentence {
  private:
    double _utc_time;
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

    inline const double UTC_time(void) const { return _utc_time; }
    inline const double lattitude(void) const { return _lattitude; }
    inline const double longitude(void) const { return _longitude; }
    inline const FixQuality fix_quality(void) const { return _fix_quality; }
    inline const int num_sats_used(void) const { return _num_sats_used; }
    inline const double HDOP(void) const { return _hdop; }
    inline const double altitude(void) const { return _altitude; }
    inline const double GEOID_separation(void) const { return _geoid_sep; }
    inline const int DGPS_update_age(void) const { return _dgps_update_age; }
    inline const int DGPS_station_id(void) const { return _dgps_station_id; }

  }; // class GGA


  enum class ReceiverMode {
    unknown,
      NotValid,
      Autonomous,
      Differential,
      Estimated,
      Simulated,
  }; // class ReceiverMode

  std::ostream& operator<< (std::ostream& out, ReceiverMode mode);


  //! Geographic position - latitude/longitude
  class GLL : public Sentence {
  private:
    double _lattitude, _longitude; // north and east are positive, respectively
    double _utc_time;
    ReceiverMode _mode;

  public:
    GLL(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    inline const double lattitude(void) const { return _lattitude; }
    inline const double longitude(void) const { return _longitude; }
    inline const double UTC_time(void) const { return _utc_time; }
    inline const ReceiverMode receiver_mode(void) const { return _mode; }

  }; // class GLL


  enum class OpMode {
    Manual,
      Automatic,
  }; // class OpMode

  std::ostream& operator<< (std::ostream& out, OpMode mode);


  enum class FixType {
    NotAvailable,
      TwoDimensional,
      ThreeDimensional,
  }; // class FixType

  std::ostream& operator<< (std::ostream& out, FixType type);


  //! GNSS DOP and active satellites
  class GSA : public Sentence {
  private:
    OpMode _mode;
    FixType _fixtype;
    std::vector<int> _sat_ids;
    double _pdop, _hdop, _vdop;

  public:
    GSA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    inline const OpMode mode(void) const { return _mode; }
    inline const FixType fix_type(void) const { return _fixtype; }
    inline const std::vector<int> satellite_ids(void) const { return _sat_ids; }
    inline const double PDOP(void) const { return _pdop; }
    inline const double HDOP(void) const { return _hdop; }
    inline const double VDOP(void) const { return _vdop; }

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

    inline const int num_messages(void) const { return _num_messages; }
    inline const int message_seq(void) const { return _msg_seq; }
    inline const int satellites_in_view(void) const { return _sats_in_view; }
    inline const std::vector<SatelliteData::ptr> satellite_data(void) const { return _sat_data; }

  }; // class GSV


  //! Recommended minimum specific GNSS data
  class RMC : public Sentence {
  private:
    double _utc_time;
    bool _status;
    double _lattitude, _longitude; // north and east are positive, respectively
    double _speed, _course;
    int _day, _month, _year;
    ReceiverMode _mode;

  public:
    RMC(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    inline const double UTC_time(void) const { return _utc_time; }
    inline const bool status(void) const { return _status; }
    inline const double lattitude(void) const { return _lattitude; }
    inline const double longitude(void) const { return _longitude; }
    inline const double speed(void) const { return _speed; }
    inline const double course(void) const { return _course; }
    inline const int UTC_day(void) const { return _day; }
    inline const int UTC_month(void) const { return _month; }
    inline const int UTC_year(void) const { return _year; }
    inline const ReceiverMode receiver_mode(void) const { return _mode; }

  }; // class RMC


  //! Course over ground and ground speed
  class VTG : public Sentence {
  private:
    double _course_true, _course_magnetic;
    double _speed_knots, _speed;
    ReceiverMode _mode;

  public:
    VTG(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    inline const double true_course(void) const { return _course_true; }
    inline const double magnetic_course(void) const { return _course_magnetic; }
    inline const double speed_knots(void) const { return _speed_knots; }
    inline const double speed(void) const { return _speed; }
    inline const ReceiverMode receiver_mode(void) const { return _mode; }

  }; // class VTG


  //! Time and date
  class ZDA : public Sentence {
  private:
    double _utc_time;
    int _day, _month, _year;
    int _tzhr, _tzmin;

  public:
    ZDA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    inline const double UTC_time(void) const { return _utc_time; }
    inline const int UTC_day(void) const { return _day; }
    inline const int UTC_month(void) const { return _month; }
    inline const int UTC_year(void) const { return _year; }
    inline const int TZ_hours(void) const { return _tzhr; }
    inline const int TZ_minutes(void) const { return _tzmin; }

  }; // class ZDA

}; // namespace NMEA0183

namespace std {
  std::string to_string(NMEA0183::FixQuality quality);
  std::string to_string(NMEA0183::FixType type);
}; // namespace std

#endif // __NMEA_0183_HH__
