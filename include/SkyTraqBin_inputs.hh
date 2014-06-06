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
#ifndef __SKYTRAQBIN_INPUTS_HH__
#define __SKYTRAQBIN_INPUTS_HH__

namespace SkyTraqBin {

  /* All input message class names shall start with a verb
     Common words shortened:
      configure => config
      download => dl
      image => img
      message => msg
      navigation => nav
      position => pos
      query => q
      software => sw
      system => sys
      version => ver
   */

  enum class StartMode : uint8_t {
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


  enum class SwType : uint8_t {
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


  enum class BaudRate : uint8_t {
    Baud4800 = 0,
    Baud9600,
    Baud19200,
    Baud38400,
    Baud57600,
    Baud115200,
    Baud230400,
    Baud460800,
    Baud921600,
  }; // class BaudRate


  enum class UpdateType : uint8_t {
    SRAM = 0,
      SRAM_and_flash,
      Temporary,
  }; // class UpdateType


  //! CONFIGURE SERIAL PORT - Set up serial port property
  class Config_serial_port : public Input_message {
  private:
    uint8_t _com_port;
    BaudRate _baud_rate;
    UpdateType _update_type;

    inline const Payload_length body_length(void) const { return 3; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    inline Config_serial_port(uint8_t cp, BaudRate br, UpdateType ut) :
      Input_message(0x05),
      _com_port(cp), _baud_rate(br), _update_type(ut)
    {}

    inline const uint8_t com_port(void) const { return _com_port; }
    inline void set_com_port(uint8_t cp) { _com_port = cp; }

    inline const BaudRate baud_rate(void) const { return _baud_rate; }
    inline void set_baud_rate(BaudRate br) { _baud_rate = br; }

    inline const UpdateType update_type(void) const { return _update_type; }
    inline void set_update_type(UpdateType ut) { _update_type = ut; }

  }; // class Config_serial_port


  //! CONFIGURE NMEA MESSAGE - Configure NMEA message interval
  class Config_NMEA_msg : public Input_message {
  private:
    uint8_t _gga_int, _gsa_int, _gsv_int, _gll_int, _rmc_int, _vtg_int, _zda_int;
    UpdateType _update_type;

    inline const Payload_length body_length(void) const { return 8; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    inline Config_NMEA_msg(uint8_t gga, uint8_t gsa, uint8_t gsv, uint8_t gll,
			   uint8_t rmc, uint8_t vtg, uint8_t zda, UpdateType ut) :
      Input_message(0x08),
      _gga_int(gga), _gsa_int(gsa), _gsv_int(gsv), _gll_int(gll),
      _rmc_int(rmc), _vtg_int(vtg), _zda_int(zda), _update_type(ut)
    {}

    inline const uint8_t GGA_interval(void) const { return _gga_int; }
    inline void set_GGA_interval(uint8_t i) { _gga_int = i; }

    inline const uint8_t GSA_interval(void) const { return _gsa_int; }
    inline void set_GSA_interval(uint8_t i) { _gsa_int = i; }

    inline const uint8_t GSV_interval(void) const { return _gsv_int; }
    inline void set_GSV_interval(uint8_t i) { _gsv_int = i; }

    inline const uint8_t GLL_interval(void) const { return _gll_int; }
    inline void set_GLL_interval(uint8_t i) { _gll_int = i; }

    inline const uint8_t RMC_interval(void) const { return _rmc_int; }
    inline void set_RMC_interval(uint8_t i) { _rmc_int = i; }

    inline const uint8_t VTG_interval(void) const { return _vtg_int; }
    inline void set_VTG_interval(uint8_t i) { _vtg_int = i; }

    inline const uint8_t ZDA_interval(void) const { return _zda_int; }
    inline void set_ZDA_interval(uint8_t i) { _zda_int = i; }

    inline const UpdateType update_type(void) const { return _update_type; }
    inline void set_update_type(UpdateType ut) { _update_type = ut; }

  }; // class Config_NMEA_msg


  enum class MessageType : uint8_t {
    None = 0,
      NMEA0183,
      Binary,
  }; // class MessageType


  //! CONFIGURE MESSAGE TYPE - Configure and select output message type
  class Config_msg_type : public Input_message {
  private:
    MessageType _msg_type;
    UpdateType _update_type;

    inline const Payload_length body_length(void) const { return 2; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    inline Config_msg_type(MessageType mt, UpdateType ut) :
      Input_message(0x09),
      _msg_type(mt),
      _update_type(ut)
    {}

    inline const MessageType message_type(void) const { return _msg_type; }
    inline void set_message_type(MessageType mt) { _msg_type = mt; }

    inline const UpdateType update_type(void) const { return _update_type; }
    inline void set_update_type(UpdateType ut) { _update_type = ut; }

  }; // class Config_msg_type


  enum class FlashType : uint8_t {
    Auto = 0,
      QSPI_Winbond,
      QSPI_EON,
      Parallel_Numonyx,
      Parallel_EON,
  }; // class FlashType


  enum class BufferUsed : uint8_t {
    Size8K = 0,
      Size16K,
      Size24K,
      Size32K,
  }; // class BufferUsed


  //! SOFTWARE IMAGE DOWNLOAD - Download software image to system flash
  class Sw_img_download : public Input_message {
  private:
    BaudRate _baud_rate;
    FlashType _flash_type;
    uint16_t _flash_id;
    BufferUsed _buffer_used;

    inline const Payload_length body_length(void) const { return 5; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor using 'automatic' flash type and flash ID of 0
    inline Sw_img_download(BaudRate br, BufferUsed bu) :
      Input_message(0x0b),
      _baud_rate(br),
      _flash_type(FlashType::Auto), _flash_id(0),
      _buffer_used(bu)
    {}

    inline Sw_img_download(BaudRate br, FlashType ft, uint16_t fid, BufferUsed bu) :
      Input_message(0x0b),
      _baud_rate(br),
      _flash_type(ft), _flash_id(fid),
      _buffer_used(bu)
    {}

    inline const BaudRate baud_rate(void) const { return _baud_rate; }
    inline void set_baud_rate(BaudRate br) { _baud_rate = br; }

    inline const FlashType flash_type(void) const { return _flash_type; }
    inline void set_flash_type(FlashType ft) { _flash_type = ft; }

    inline const BufferUsed buffer_used(void) const { return _buffer_used; }
    inline void set_buffer_used(BufferUsed bu) { _buffer_used = bu; }

  }; // class Sw_img_download


  enum class PowerMode : uint8_t {
    Normal = 0,
      PowerSave,
  }; // class PowerMode


  //! CONFIGURE SYSTEM POWER MODE - Set the power mode of GNSS system
  class Config_sys_power_mode : public Input_message {
  private:
    PowerMode _power_mode;
    UpdateType _update_type;

    inline const Payload_length body_length(void) const { return 2; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    Config_sys_power_mode(PowerMode pm, UpdateType ut) :
      Input_message(0x0c),
      _power_mode(pm),
      _update_type(ut)
    {}

    inline const PowerMode power_mode(void) const { return _power_mode; }
    inline void set_power_mode(PowerMode pm) { _power_mode = pm; }

    inline const UpdateType update_type(void) const { return _update_type; }
    inline void set_update_type(UpdateType ut) { _update_type = ut; }

  }; // class Config_sys_power_mode


  //! CONFIGURE SYSTEM POSITION RATE - Configure the position update rate of GNSS system
  class Config_sys_pos_rate : public Input_message {
  private:
    uint8_t _rate;
    UpdateType _update_type;

    inline const Payload_length body_length(void) const { return 2; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    Config_sys_pos_rate(uint8_t r, UpdateType ut) :
      Input_message(0x0e),
      _rate(r),
      _update_type(ut)
    {}

    inline const uint8_t rate(void) const { return _rate; }
    inline void set_rate(uint8_t r) { _rate = r; }

    inline const UpdateType update_type(void) const { return _update_type; }
    inline void set_update_type(UpdateType ut) { _update_type = ut; }

  }; // class Config_sys_pos_rate


  //! QUERY POSITION UPDATE RATE - Query the position update rate of GNSS system
  class Q_pos_update_rate : public Input_message {
  private:
    inline const Payload_length body_length(void) const { return 0; }
    virtual inline void body_to_buf(unsigned char* buffer) const { }

  public:
    Q_pos_update_rate(void) :
      Input_message(0x10)
    {}

  }; // class Q_pos_update_rate


  //! CONFIGURE NAVIGATION DATA MESSAGE INTERVAL - Configure binary navigation data message interval
  class Config_nav_data_msg_interval : public Input_message {
  private:
    uint8_t _interval;	// 0: disable
    UpdateType _update_type;

    inline const Payload_length body_length(void) const { return 2; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    Config_nav_data_msg_interval(uint8_t i, UpdateType ut) :
      Input_message(0x11),
      _interval(i),
      _update_type(ut)
    {}

    inline const uint8_t interval(void) const { return _interval; }
    inline void set_interval(uint8_t i) { _interval = i; }

    inline const UpdateType update_type(void) const { return _update_type; }
    inline void set_update_type(UpdateType ut) { _update_type = ut; }

  }; // class Config_nav_data_msg_interval


  //! CONFIGURE NMEA TALKER ID - Configure NMEA talker ID of GNSS receive
  class Config_NMEA_talker_ID : public Input_message {
  private:
    TalkerID _talker_id;
    UpdateType _update_type;

    inline const Payload_length body_length(void) const { return 2; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    Config_NMEA_talker_ID(TalkerID id, UpdateType ut) :
      Input_message(0x4b),
      _talker_id(id),
      _update_type(ut)
    {}

    inline const TalkerID talker_id(void) const { return _talker_id; }
    inline void set_talker_id(TalkerID id) { _talker_id = id; }

    inline const UpdateType update_type(void) const { return _update_type; }
    inline void set_update_type(UpdateType ut) { _update_type = ut; }

  }; // class Config_NMEA_talker_ID


  //! QUERY NMEA TALKER ID - Query NMEA talker ID of GNSS receiver
  class Q_NMEA_talker_ID : public Input_message {
  private:
    inline const Payload_length body_length(void) const { return 0; }
    virtual inline void body_to_buf(unsigned char* buffer) const { }

  public:
    Q_NMEA_talker_ID(void) :
      Input_message(0x4f)
    {}

  }; // class Q_NMEA_talker_ID


  //! QUERY GNSS BOOT STATUS - Query boot status of GNSS receiver
  class Q_GNSS_boot_status : public Input_message_with_subid {
  private:
    inline const Payload_length body_length(void) const { return 0; }
    virtual inline void body_to_buf(unsigned char* buffer) const { }

  public:
    Q_GNSS_boot_status(void) :
      Input_message_with_subid(0x64, 0x01)
    {}

  }; // class Q_GNSS_boot_status


}; // namespace SkyTraqBin

#endif // __SKYTRAQBIN_INPUTS_HH__
