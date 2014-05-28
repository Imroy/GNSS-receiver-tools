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
#ifndef __SKYTRAQBIN_HH__
#define __SKYTRAQBIN_HH__

#include <memory>
#include <sstream>
#include <vector>
#include <math.h>

namespace SkyTraqBin {

  typedef unsigned short int Payload_length;

  class InvalidMessage : public std::exception {
  private:

  public:
    InvalidMessage() {}

    const char* what() const throw() {
      return "Invalid message";
    }
  }; // class InvalidMessage


  //! Exception class for when checksums don't match
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
      oss << "Checksum mismatch in SkyTraq binary stream - computed 0x" << std::hex << (int)_computed_cs << ", found 0x" << (int)_stream_cs;
      return oss.str().c_str();
    }
  }; // class ChecksumMismatch


  //! Base class for a binary message
  class Message {
  protected:
    unsigned char _msg_id;

  public:
    //! Constructor
    Message(unsigned char id) :
      _msg_id(id)
    {}

    //! Virtual destructor to force polymorphism
    inline virtual ~Message() {}

    inline const unsigned char message_id(void) const { return _msg_id; }

    //! Check the type of an object
    template <typename T>
    inline bool isa(void) const { return typeid(*this) == typeid(T); }

    //! Recast this object to another type
    template <typename T>
    inline T* cast_as(void) { return dynamic_cast<T*>(this); }

    typedef std::shared_ptr<Message> ptr;
  }; // class Message


  //! Base class for messages that come from the GPS receiver
  class Output_message : public Message {
  protected:

  public:
    //! Constructor from a binary buffer
    inline Output_message(unsigned char* payload, Payload_length payload_len) :
      Message(payload_len > 0 ? payload[0] : 0)
    {}

    typedef std::shared_ptr<Output_message> ptr;
  }; // class Output_message


  //! Parser
  std::vector<Output_message::ptr> parse_messages(unsigned char* buffer, std::streamsize len);


  //! Base class for messages that go to the GPS receiver
  class Input_message : public Message {
  protected:
    //! The length of the body (not including message id)
    virtual const Payload_length body_length(void) const = 0;

    //! Write body fields into a pre-allocated buffer
    virtual void body_to_buf(unsigned char* buffer) const = 0;

  public:
    //! Constructor
    Input_message(unsigned char id) :
      Message(id)
    {}

    //! The total length of the message
    inline const Payload_length message_length(void) const { return 2 + 2 + 1 + body_length() + 1 + 2; }

    //! Write the message into a buffer
    /*!
      Use message_length() to know how big the buffer needs to be.
     */
    virtual void to_buf(unsigned char *buffer) const;

    typedef std::shared_ptr<Input_message> ptr;
  }; // class Input_message

  /* All input message class names shall start with a verb
     Common words shortened:
      configure => config
      download => dl
      message => msg
      navigation => nav
      position => pos
      query => q
      software => sw
      system => sys
      version => ver
   */

  enum class StartMode {
    HotStart = 1,
      WarmStart,
      ColdStart,
  }; // class StartMode

  //! SYSTEM RESTART - Force System to restart
  class Restart_sys : public Input_message {
  private:
    StartMode _start_mode;
    uint16_t _utc_year;
    uint8_t _utc_month, _utc_day, _utc_hour, _utc_minute, _utc_second;
    int16_t _lattitude, _longitude, _altitude;

    inline const Payload_length body_length(void) const { return 14; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    inline Restart_sys(StartMode mode,
		       uint16_t y, uint8_t m, uint8_t d,
		       uint8_t hr, uint8_t min, uint8_t sec,
		       int16_t lat, int16_t lon, int16_t alt) :
      Input_message(0x01),
      _start_mode(mode),
      _utc_year(y), _utc_month(m), _utc_day(d),
      _utc_hour(hr), _utc_minute(min), _utc_second(sec),
      _lattitude(lat), _longitude(lon), _altitude(alt)
    {}

    //! Constructor with floating point lat/long/alt
    inline Restart_sys(StartMode mode,
		       uint16_t y, uint8_t m, uint8_t d,
		       uint8_t hr, uint8_t min, uint8_t sec,
		       double lat, double lon, double alt) :
      Input_message(0x01),
      _start_mode(mode),
      _utc_year(y), _utc_month(m), _utc_day(d),
      _utc_hour(hr), _utc_minute(min), _utc_second(sec),
      _lattitude(floor(0.5 + lat * 100)), _longitude(floor(0.5 + lon * 100)), _altitude(floor(0.5 + alt))
    {}

    inline const StartMode start_mode(void) const { return _start_mode; }
    inline void set_start_mode(StartMode mode) { _start_mode = mode; }

    inline const uint16_t UTC_year(void) const { return _utc_year; }
    inline void set_UTC_year(uint16_t y) { _utc_year = y; }

    inline const uint8_t UTC_month(void) const { return _utc_month; }
    inline void set_UTC_month(uint8_t m) { _utc_month = m; }

    inline const uint8_t UTC_day(void) const { return _utc_day; }
    inline void set_UTC_day(uint8_t d) { _utc_day = d; }

    inline const uint8_t UTC_hour(void) const { return _utc_hour; }
    inline void set_UTC_hour(uint8_t hr) { _utc_hour = hr; }

    inline const uint8_t UTC_minute(void) const { return _utc_minute; }
    inline void set_UTC_minute(uint8_t min) { _utc_minute = min; }

    inline const uint8_t UTC_second(void) const { return _utc_second; }
    inline void set_UTC_second(uint8_t sec) { _utc_second = sec; }

    inline const int16_t lattitude_raw(void) const { return _lattitude; }
    inline void set_lattitude_raw(int16_t lat) { _lattitude = lat; }

    inline const double lattitude(void) const { return _lattitude * 0.01; }
    inline void set_lattitude(double lat) { _lattitude = floor(0.5 + lat * 100); }

    inline const int16_t longitude_raw(void) const { return _longitude; }
    inline void set_longitude_raw(int16_t lon) { _longitude = lon; }

    inline const double longitude(void) const { return _longitude * 0.01; }
    inline void set_longitude(double lon) { _longitude = floor(0.5 + lon * 100); }

    inline const int16_t altitude(void) const { return _altitude; }
    inline void set_altitude(int16_t alt) { _altitude = alt; }

  };


  enum class SwType {
    SystemCode = 1,
  }; // class SwType


  //! QUERY SOFTWARE VERSION - Query revision information of loaded software
  class Q_sw_ver : public Input_message {
  private:
    SwType _sw_type;

    inline const Payload_length body_length(void) const { return 1; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    inline Q_sw_ver(SwType type = SwType::SystemCode) :
      Input_message(0x02),
      _sw_type(type)
    {}

    inline const SwType software_type(void) const { return _sw_type; }
    inline void set_software_type(SwType type) { _sw_type = type; }

  }; // class Q_sw_ver


  //! QUERY SOFTWARE CRC - Query CRC information of loaded software
  class Q_sw_CRC : public Input_message {
  private:
    SwType _sw_type;

    inline const Payload_length body_length(void) const { return 1; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    inline Q_sw_CRC(SwType type = SwType::SystemCode) :
      Input_message(0x03),
      _sw_type(type)
    {}

    inline const SwType software_type(void) const { return _sw_type; }
    inline void set_software_type(SwType type) { _sw_type = type; }

  }; // class Q_sw_CRC


  //! SET FACTORY DEFAULTS - Set the system to factory default values
  class Set_factory_defaults : public Input_message {
  private:
    bool _reset;

    inline const Payload_length body_length(void) const { return 1; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    inline Set_factory_defaults(bool r = true) :
      Input_message(0x04),
      _reset(r)
    {}

    inline const bool reset(void) const { return _reset; }
    inline void set_reset(bool r) { _reset = r; }

  }; // class Set_factory_defaults


  struct PackedVersion {
    uint8_t X, Y, Z;

    inline PackedVersion(uint8_t x, uint8_t y, uint8_t z) :
      X(x), Y(y), Z(z)
    {}
  }; // struct PackedVersion


  struct PackedDate {
    uint8_t year, month, day;

    inline PackedDate(uint8_t y, uint8_t m, uint8_t d) :
      year(y), month(m), day(d)
    {}
  }; // struct PackedDate


  //! SOFTWARE VERSION - Software version of the GNSS receiver
  class Sw_ver : public Output_message {
  private:
    SwType _sw_type;
    PackedVersion _kernel_ver, _odm_ver;
    PackedDate _revision;

  public:
    //! Constructor from a binary buffer
    Sw_ver(unsigned char* payload, Payload_length payload_len);

    inline const SwType software_type(void) const { return _sw_type; }
    inline PackedVersion kernel_version(void) const { return _kernel_ver; }
    inline const PackedVersion ODM_version(void) const { return _odm_ver; }
    inline PackedDate revision(void) const { return _revision; }

  }; // class Sw_ver


  //! SOFTWARE CRC -Software CRC of the GNSS receiver
  class Sw_CRC : public Output_message {
  private:
    SwType _sw_type;
    uint16_t _crc;

  public:
    //! Constructor from a binary buffer
    Sw_CRC(unsigned char* payload, Payload_length payload_len);

    inline const SwType software_type(void) const { return _sw_type; }
    inline uint16_t CRC(void) const { return _crc; }

  }; // class Sw_CRC


  //! ACK - Acknowledgement to a Request Message
  class Ack : public Output_message {
  private:
    uint8_t _ack_id;
    bool _has_subid;
    uint8_t _ack_subid;

  public:
    //! Constructor from a binary buffer
    Ack(unsigned char* payload, Payload_length payload_len);

    inline const uint8_t ack_id(void) const { return _ack_id; }
    inline const bool has_subid(void) const { return _has_subid; }
    inline const uint8_t ack_subid(void) const { return _ack_subid; }

  }; // class Ack


  //! NACK - Response to an unsuccessful request message
  class Nack : public Output_message {
  private:
    uint8_t _ack_id;
    bool _has_subid;
    uint8_t _ack_subid;

  public:
    //! Constructor from a binary buffer
    Nack(unsigned char* payload, Payload_length payload_len);

    inline const uint8_t ack_id(void) const { return _ack_id; }
    inline const bool has_subid(void) const { return _has_subid; }
    inline const uint8_t ack_subid(void) const { return _ack_subid; }

  }; // class Ack


  //! POSITON UPDATE RATE - Position Update rate of the GNSS system
  class Pos_update_rate : public Output_message {
  private:
    uint8_t _update_rate;

  public:
    //! Constructor from a binary buffer
    Pos_update_rate(unsigned char* payload, Payload_length payload_len);

    inline const uint8_t update_rate(void) const { return _update_rate; }

  }; // class Pos_update_rate


  enum class TalkerID {
    GPmode,
      GNmode,
  }; // class TalkerID


  //! GNSS NMEA TALKER ID - NMEA talker ID of GNSS receiver
  class NMEA_talker_id : public Output_message {
  private:
    TalkerID _talker_id;

  public:
    //! Constructor from a binary buffer
    NMEA_talker_id(unsigned char* payload, Payload_length payload_len);

    inline const TalkerID talker_id(void) const { return _talker_id; }

  }; //class NMEA_talker_id


  //! MEAS_TIME - Measurement time information (0xDC) (Periodic)
  class Measurement_time : public Output_message {
  private:
    uint8_t _issue;
    uint16_t _weeknumber;
    uint32_t _time_in_week;
    uint16_t _period;

  public:
    //! Constructor from a binary buffer
    Measurement_time(unsigned char* payload, Payload_length payload_len);

    inline const uint8_t issue_of_data(void) const { return _issue; }
    inline const uint16_t week_number(void) const { return _weeknumber; }
    inline const uint32_t time_in_week(void) const { return _time_in_week; }
    inline const uint16_t period(void) const { return _period; }

  }; // class Measurement_time


  struct RawMeasurement {
    uint8_t PRN, CN0;
    double pseudorange;
    double carrier_phase;
    float doppler_freq;
    uint8_t channel_indicator;

    RawMeasurement(uint8_t p, uint8_t c, double pr, double cph, float df, uint8_t ci) :
      PRN(p), CN0(c), pseudorange(pr), carrier_phase(cph), doppler_freq(df), channel_indicator(ci)
    {}
  }; // struct RawMeasurement


  //! RAW_MEAS - Raw measurements from each channel (0xDD) (Periodic)
  class Raw_measurements : public Output_message {
  private:
    uint8_t _issue;
    uint8_t _num_meas;
    std::vector<RawMeasurement> _measurements;

  public:
    //! Constructor from a binary buffer
    Raw_measurements(unsigned char* payload, Payload_length payload_len);

    inline const uint8_t issue_of_data(void) const { return _issue; }
    inline const uint8_t num_measurements(void) const { return _num_meas; }
    inline const std::vector<RawMeasurement> measurements(void) const { return _measurements; }

  }; // class Raw_measurements


  struct SvStatus {
    uint8_t channel_id, PRN, SV_status, URA;
    int8_t CN0;
    int16_t elevation, azimuth;
    uint8_t channel_status;

    SvStatus(uint8_t cid, uint8_t p, uint8_t svs, uint8_t u, int8_t c, int16_t el, int16_t az, uint8_t cs) :
      channel_id(cid), PRN(p), SV_status(svs), URA(u), CN0(c), elevation(el), azimuth(az), channel_status(cs)
    {}
  }; // struct SvStatus


  //! SV_CH_STATUS - SV and channel status (0xDE) (Periodic)
  class SV_channel_status : public Output_message {
  private:
    uint8_t _issue;
    uint8_t _num_sv;
    std::vector<SvStatus> _statuses;

  public:
    //! Constructor from a binary buffer
    SV_channel_status(unsigned char* payload, Payload_length payload_len);

    inline const uint8_t issue_of_data(void) const { return _issue; }
    inline const uint8_t num_svs(void) const { return _num_sv; }
    inline const std::vector<SvStatus> statuses(void) const { return _statuses; }

  };


}; // SkyTraqBin

#endif // __SKYTRAQBIN_HH__
