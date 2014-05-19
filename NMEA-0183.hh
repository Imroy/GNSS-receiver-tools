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

  class Sentence {
  private:
    const char _talker_id[2], _type[3];
    const unsigned char _checksum;

    static unsigned char _generate_checksum(std::string tid, std::string type, std::string data);
    static std::vector<std::string> _split_fields(std::string data);

  public:
    // Constructor
    Sentence(std::string tid, std::string type, unsigned char checksum);

    inline virtual ~Sentence() {}

    typedef std::shared_ptr<Sentence> ptr;

    inline const std::string talker_id(void) const { return std::string(_talker_id, 2); }
    inline const std::string type(void) const { return std::string(_type, 3); }
    inline const unsigned char checksum(void) const { return _checksum; }

    template <typename T>
    inline bool isa(void) const { return typeid(*this) == typeid(T); }

    template <typename T>
    inline T* cast_as(void) { return dynamic_cast<T*>(this); }

    friend Sentence::ptr parse_sentence(std::string line);
  }; // class Sentence

  Sentence::ptr parse_sentence(std::string line);


  enum class GPSquality {
    Unavailable,
      SPSmode,
      DGPSmode,
  }; // class GPSquality

  std::ostream& operator<< (std::ostream& out, GPSquality quality);

  class GGA : public Sentence {
  private:
    double _utc_time;
    double _lattitude, _longitude; // north and east are positive, respectively
    GPSquality _gps_quality;
    int _num_sats_used;
    double _hdop;
    double _altitude; // Above mean sea level
    double _geoid_sep;
    int _dgps_station_id;

  public:
    GGA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    inline double UTC_time(void) const { return _utc_time; }
    inline double lattitude(void) const { return _lattitude; }
    inline double longitude(void) const { return _longitude; }
    inline GPSquality GPS_quality(void) const { return _gps_quality; }
    inline int num_sats_used(void) const { return _num_sats_used; }
    inline double HDOP(void) const { return _hdop; }
    inline double altitude(void) const { return _altitude; }
    inline double GEOID_separation(void) const { return _geoid_sep; }
    inline int DGPS_station_id(void) const { return _dgps_station_id; }

  }; // class GGA


  //! Geographic position - latitude/longitude
  class GLL : public Sentence {
  private:
    double _lattitude, _longitude; // north and east are positive, respectively
    double _utc_time;
    bool _status;

  public:
    GLL(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

    inline double lattitude(void) const { return _lattitude; }
    inline double longitude(void) const { return _longitude; }
    inline double UTC_time(void) const { return _utc_time; }
    inline bool status(void) const { return _status; }

  }; // class GLL


  class GSA : public Sentence {
  private:

  public:
    GSA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

  }; // class GSA


  class GSV : public Sentence {
  private:

  public:
    GSV(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

  }; // class GSV


  class RMC : public Sentence {
  private:

  public:
    RMC(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

  }; // class RMC


  class VTG : public Sentence {
  private:

  public:
    VTG(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

  }; // class VTG


  class ZDA : public Sentence {
  private:

  public:
    ZDA(std::string tid, std::string type, std::vector<std::string> fields, unsigned char checksum);

  }; // class ZDA

}; // namespace NMEA0183

#endif // __NMEA_0183_HH__
