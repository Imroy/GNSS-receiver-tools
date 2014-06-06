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

#include <string.h>

namespace SkyTraqBin {

  enum class StartMode : uint8_t {
    HotStart = 1,
      WarmStart,
      ColdStart,
  }; // class StartMode


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


  enum class MessageType : uint8_t {
    None = 0,
      NMEA0183,
      Binary,
  }; // class MessageType


  enum class OutputRate : uint8_t {
    Rate1Hz = 0,
      Rate2Hz,
      Rate4Hz,
      Rate5Hz,
      Rate10Hz,
      Rate20Hz,
  }; // class OutputRate


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


  //! Configure Binary Measurement Output Rates
  class Config_bin_measurement_output_rates : public Input_message {
  private:
    OutputRate _output_rate;
    bool _meas_time, _raw_meas, _sv_ch_status, _rcv_state, _subframe;
    UpdateType _update_type;

    inline const Payload_length body_length(void) const { return 7; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    Config_bin_measurement_output_rates(OutputRate o, bool mt, bool rm, bool svch, bool rcv, bool sub, UpdateType ut) :
      Input_message(0x12),
      _output_rate(o),
      _meas_time(mt), _raw_meas(rm), _sv_ch_status(svch), _rcv_state(rcv), _subframe(sub),
      _update_type(ut)
    {}

    inline const OutputRate output_rate(void) const { return _output_rate; }
    inline void set_output_rate(OutputRate o) { _output_rate = o; }

    inline const bool meas_time(void) const { return _meas_time; }
    inline void set_meas_time(bool mt=true) { _meas_time = mt; }
    inline void unset_meas_time(void) { _meas_time = false; }

    inline const bool raw_meas(void) const { return _raw_meas; }
    inline void set_raw_meas(bool rm=true) { _raw_meas = rm; }
    inline void unset_raw_meas(void) { _raw_meas = false; }

    inline const bool SV_CH_status(void) const { return _sv_ch_status; }
    inline void set_SV_CH_status(bool svch=true) { _sv_ch_status = svch; }
    inline void unset_SV_CH_status(void) { _sv_ch_status = false; }

    inline const bool RCV_state(void) const { return _rcv_state; }
    inline void set_RCV_state(bool rcv=true) { _rcv_state = rcv; }
    inline void unset_RCV_state(void) { _rcv_state = false; }

    inline const bool subframe(void) const { return _subframe; }
    inline void set_subframe(bool sub=true) { _subframe = sub; }
    inline void unset_subframe(void) { _subframe = false; }

    inline const UpdateType update_type(void) const { return _update_type; }
    inline void set_update_type(UpdateType ut) { _update_type = ut; }

  }; // class Config_bin_measurement_output_rates


  //! QUERY POWER MODE - Query status of power mode of GNSS receiver
  class Q_power_mode : public Input_message {
  public:
    Q_power_mode(void) :
      Input_message(0x15)
    {}

  }; // class Q_power_mode


  //! CONFIGURE DATUM - Configure datum used for GNSS position transformation
  class Config_datum : public Input_message {
  private:
    uint16_t _datum_index;
    uint8_t _ellip_index;
    int16_t _delta_x, _delta_y, _delta_z; // metres
    uint32_t _semi_major_axis, _inv_flattening;
    UpdateType _update_type;

    inline const Payload_length body_length(void) const { return 18; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    Config_datum(uint16_t di, uint8_t ei,
		 int16_t dx, int16_t dy, int16_t dz,
		 uint32_t sma, uint32_t inf, UpdateType ut) :
      Input_message(0x29),
      _datum_index(di), _ellip_index(ei),
      _delta_x(dx), _delta_y(dy), _delta_z(dz),
      _semi_major_axis(sma), _inv_flattening(inf),
      _update_type(ut)
    {}

    inline const uint16_t datum_index(void) const { return _datum_index; }
    inline void set_datum_index(uint16_t di) { _datum_index = di; }

    inline const uint8_t ellipsoid_index(void) const { return _ellip_index; }
    inline void set_ellipsoid_index(uint8_t ei) { _ellip_index = ei; }

    inline const int16_t delta_X(void) const { return _delta_x; }
    inline void set_delta_X(int16_t dx) { _delta_x = dx; }

    inline const int16_t delta_Y(void) const { return _delta_y; }
    inline void set_delta_Y(int16_t dy) { _delta_y = dy; }

    inline const int16_t delta_Z(void) const { return _delta_z; }
    inline void set_delta_Z(int16_t dz) { _delta_z = dz; }

    inline const uint32_t semi_major_axis(void) const { return _semi_major_axis; }
    inline void set_semi_major_axis(uint32_t sma) { _semi_major_axis = sma; }

    inline const uint32_t inv_flattening(void) const { return _inv_flattening; }
    inline void set_inv_flattening(uint32_t inf) { _inv_flattening = inf; }

    inline const UpdateType update_type(void) const { return _update_type; }
    inline void set_update_type(UpdateType ut) { _update_type = ut; }

  }; // class Config_datum


  //! CONFIGURE DOP MASK - Configure values of DOP mask
  class Config_DOP_mask : public Input_message {
  private:
    DOPmode _dop_mode;
    uint16_t _pdop, _hdop, _gdop; // * 0.1
    UpdateType _update_type;

    inline const Payload_length body_length(void) const { return 8; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    Config_DOP_mask(DOPmode m, double p, double h, double g, UpdateType ut) :
      Input_message(0x2A),
      _dop_mode(m),
      _pdop(p * 10), _hdop(h * 10), _gdop(g * 10),
      _update_type(ut)
    {}

    Config_DOP_mask(DOPmode m, uint16_t p, uint16_t h, uint16_t g, UpdateType ut) :
      Input_message(0x2A),
      _dop_mode(m),
      _pdop(p), _hdop(h), _gdop(g),
      _update_type(ut)
    {}

    inline const DOPmode DOP_mode(void) const { return _dop_mode; }
    inline void set_DOP_mode(DOPmode m) { _dop_mode = m; }

    inline const double PDOP(void) const { return _pdop * 0.1; }
    inline const uint16_t PDOP_raw(void) const { return _pdop; }
    inline void set_PDOP(double p) { _pdop = p * 10; }
    inline void set_PDOP(uint16_t p) { _pdop = p; }

    inline const double HDOP(void) const { return _hdop * 0.1; }
    inline const uint16_t HDOP_raw(void) const { return _hdop; }
    inline void set_HDOP(double h) { _hdop = h * 10; }
    inline void set_HDOP(uint16_t h) { _hdop = h; }

    inline const double GDOP(void) const { return _gdop * 0.1; }
    inline const uint16_t GDOP_raw(void) const { return _gdop; }
    inline void set_GDOP(double g) { _gdop = g * 10; }
    inline void set_GDOP(uint16_t g) { _gdop = g; }

    inline const UpdateType update_type(void) const { return _update_type; }
    inline void set_update_type(UpdateType ut) { _update_type = ut; }

  }; // class Config_DOP_mask


  //! CONFIGURE ELEVATION AND CNR MASK - Configure values of elevation and CNR mask
  class Config_elevation_CNR_mask : public Input_message {
  private:
    ElevationCNRmode _mode_select;
    uint8_t _el_mask, _cnr_mask;
    UpdateType _update_type;

    inline const Payload_length body_length(void) const { return 8; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    Config_elevation_CNR_mask(ElevationCNRmode ms, uint8_t em, uint8_t cm, UpdateType ut) :
      Input_message(0x2B),
      _mode_select(ms),
      _el_mask(em), _cnr_mask(cm),
      _update_type(ut)
    {}

    inline const ElevationCNRmode mode_select(void) const { return _mode_select; }
    inline void set_mode_select(ElevationCNRmode ms) { _mode_select = ms; }

    inline const uint8_t elevation_mask(void) const { return _el_mask; }
    inline void set_elevation_mask(uint8_t em) { _el_mask = em; }

    inline const uint8_t CNR_mask(void) const { return _cnr_mask; }
    inline void set_CNR_mask(uint8_t cm) { _cnr_mask = cm; }

    inline const UpdateType update_type(void) const { return _update_type; }
    inline void set_update_type(UpdateType ut) { _update_type = ut; }

  }; // class Config_elevation_CNR_mask


  //! QUERY DATUM - Query datum used by the GNSS receiver
  class Q_datum : public Input_message {
  public:
    Q_datum(void) :
      Input_message(0x2D)
    {}

  }; // class Q_datum


  //! QUERY DOP MASK - Query information of DOP mask used by the GNSS receiver
  class Q_DOP_mask : public Input_message {
  public:
    Q_DOP_mask(void) :
      Input_message(0x2E)
    {}

  }; // class Q_DOP_mask


  //! QUERY ELEVATION AND CNR MASK - Query elevation and CNR mask used by the GNSS receiver
  class Q_elevation_CNR_mask : public Input_message {
  public:
    Q_elevation_CNR_mask(void) :
      Input_message(0x2F)
    {}

  }; // class Q_elevation_CNR_mask


  //! GET GPS EPHEMERIS - Get GPS ephemeris used of GNSS receiver
  class Get_GPS_ephemeris : public Input_message {
  private:
    uint8_t _sv_num;

    inline const Payload_length body_length(void) const { return 1; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    Get_GPS_ephemeris(uint8_t sv) :
      Input_message(0x30),
      _sv_num(sv)
    {}

    inline const uint8_t SV_number(void) const { return _sv_num; }
    inline void set_SV_number(uint8_t sv) { _sv_num = sv; }

  }; // class Get_GPS_ephemeris


  //! SET GPS EPHEMERIS - Set GPS ephemeris to GNSS receiver
  class Set_GPS_ephemeris : public Input_message {
  private:
    uint16_t _sv_num;
    uint8_t _subframe1[28], _subframe2[28], _subframe3[28];

    inline const Payload_length body_length(void) const { return 86; }
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    Set_GPS_ephemeris(uint16_t sv, uint8_t sf1[28], uint8_t sf2[28], uint8_t sf3[28]) :
      Input_message(0x41),
      _sv_num(sv)
    {
      memcpy(&_subframe1, sf1, 28);
      memcpy(&_subframe2, sf2, 28);
      memcpy(&_subframe3, sf3, 28);
    }

    inline const uint16_t SV_number(void) const { return _sv_num; }
    inline void set_SV_number(uint16_t sv) { _sv_num = sv; }

    inline const uint8_t* subframe1(void) const { return _subframe1; }
    inline void set_subframe1(uint8_t sf1[28]) { memcpy(&_subframe1, sf1, 28); }

    inline const uint8_t* subframe2(void) const { return _subframe2; }
    inline void set_subframe2(uint8_t sf2[28]) { memcpy(&_subframe2, sf2, 28); }

    inline const uint8_t* subframe3(void) const { return _subframe3; }
    inline void set_subframe3(uint8_t sf3[28]) { memcpy(&_subframe3, sf3, 28); }

  }; // class Set_GPS_ephemeris


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
