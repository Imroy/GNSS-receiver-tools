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

#include "boost/date_time/posix_time/posix_time_types.hpp"
#include <string.h>
#include "SkyTraqBin.hh"

namespace greg = boost::gregorian;
namespace ptime = boost::posix_time;

/*
  Sources:
  https://store-lgdi92x.mybigcommerce.com/content/AN0028_1.4.31.pdf	(Binary messages of Skytraq Venus 8)
  https://store-lgdi92x.mybigcommerce.com/content/AN0024_v07.pdf	(Raw measurement binary messages of Skytraq 6 & 8)
  https://store-lgdi92x.mybigcommerce.com/content/SUP800F_v0.6.pdf	(Skytraq SUP800F datasheet)
*/

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

  //! SYSTEM RESTART - Force System to restart
  class Restart_sys : public Input_message {
  private:
    StartMode _start_mode;
    ptime::ptime _utc_time;
    int16_t _lattitude, _longitude, _altitude;

    GETTER(Payload_length, body_length, 14);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param mode	Ways to restart the receiver
      \param y,m,d	Current date
      \param hr,min,sec	Current time
      \param lat,lon	Current lattitude and longitude (in 1/100 degree)
      \param alt	Current altitude (metres)
     */
    inline Restart_sys(StartMode mode,
		       uint16_t y, uint8_t m, uint8_t d,
		       uint8_t hr, uint8_t min, uint8_t sec,
		       int16_t lat, int16_t lon, int16_t alt) :
      Input_message(0x01),
      _start_mode(mode),
      _utc_time(greg::date(y, m, d), ptime::time_duration(hr, min, sec)),
      _lattitude(lat), _longitude(lon), _altitude(alt)
    {}

    //! Constructor with floating point lat/long/alt
    /*!
      \param mode	Ways to restart the receiver
      \param y,m,d	Current date
      \param hr,min,sec	Current time
      \param lat,lon	Current lattitude and longitude (degrees)
      \param alt	Current altitude (metres)
     */
    inline Restart_sys(StartMode mode,
		       uint16_t y, uint8_t m, uint8_t d,
		       uint8_t hr, uint8_t min, uint8_t sec,
		       double lat, double lon, double alt) :
      Input_message(0x01),
      _start_mode(mode),
      _utc_time(greg::date(y, m, d), ptime::hours(hr) + ptime::minutes(min) + ptime::seconds(sec)),
      _lattitude(floor(0.5 + lat * 100)), _longitude(floor(0.5 + lon * 100)), _altitude(floor(0.5 + alt))
    {}

    GETTER_SETTER(StartMode, start_mode, _start_mode);
    GETTER(ptime::ptime, UTC_time, _utc_time);

    GETTER_SETTER_RAW(int16_t, lattitude, _lattitude);
    GETTER_SETTER_MOD(double, lattitude, _lattitude, _lattitude * 0.01, val * 100);

    GETTER_SETTER_RAW(int16_t, longitude, _longitude);
    GETTER_SETTER_MOD(double, longitude, _longitude, _longitude * 0.01, val * 100);

    GETTER_SETTER(int16_t, altitude, _altitude);

  };


  //! QUERY SOFTWARE VERSION - Query revision information of loaded software
  //! - Responds with Sw_ver message
  class Q_sw_ver : public Input_message, public with_response {
  private:
    SwType _sw_type;

    GETTER(Payload_length, body_length, 1);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param type Software type. Defaults to 'system code', the only
      non-reserved code documented.
    */
    inline Q_sw_ver(SwType type = SwType::SystemCode) :
      Input_message(0x02),
      _sw_type(type)
    {}

    RESPONSE1(0x80);

    GETTER_SETTER(SwType, software_type, _sw_type);

  }; // class Q_sw_ver


  //! QUERY SOFTWARE CRC - Query CRC information of loaded software
  //! - Responds with Sw_CRC message
  class Q_sw_CRC : public Input_message, public with_response {
  private:
    SwType _sw_type;

    GETTER(Payload_length, body_length, 1);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param type Software type. Defaults to 'system code', the only
      non-reserved code documented.
    */
    inline Q_sw_CRC(SwType type = SwType::SystemCode) :
      Input_message(0x03),
      _sw_type(type)
    {}

    RESPONSE1(0x81);

    GETTER_SETTER(SwType, software_type, _sw_type);

  }; // class Q_sw_CRC


  //! SET FACTORY DEFAULTS - Set the system to factory default values
  class Set_factory_defaults : public Input_message {
  private:
    bool _reset;

    GETTER(Payload_length, body_length, 1);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param r Reboot after setting factory defaults
    */
    inline Set_factory_defaults(bool r = true) :
      Input_message(0x04),
      _reset(r)
    {}

    GETTER(bool, reset, _reset);
    SETTER_BOOL(reset, _reset);

  }; // class Set_factory_defaults


  //! CONFIGURE SERIAL PORT - Set up serial port property
  class Config_serial_port : public Input_message {
  private:
    uint8_t _com_port;
    BaudRate _baud_rate;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 3);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param cp "COM" port number
      \param br Baud rate
      \param ut Update type
     */
    inline Config_serial_port(uint8_t cp, BaudRate br, UpdateType ut) :
      Input_message(0x05),
      _com_port(cp), _baud_rate(br), _update_type(ut)
    {}

    GETTER_SETTER(uint8_t, com_port, _com_port);
    GETTER_SETTER(BaudRate, baud_rate, _baud_rate);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_serial_port


  //! CONFIGURE NMEA MESSAGE - Configure NMEA message interval
  class Config_NMEA_msg : public Input_message {
  private:
    uint8_t _gga_int, _gsa_int, _gsv_int, _gll_int, _rmc_int, _vtg_int, _zda_int;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 8);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param gga GGA sentence interval (seconds)
      \param gsa GSA sentence interval (seconds)
      \param gsv GSV sentence interval (seconds)
      \param gll GLL sentence interval (seconds)
      \param rmc RMC sentence interval (seconds)
      \param vtg VTG sentence interval (seconds)
      \param zda ZDA sentence interval (seconds)
      \param ut Update type
     */
    inline Config_NMEA_msg(uint8_t gga, uint8_t gsa, uint8_t gsv, uint8_t gll,
			   uint8_t rmc, uint8_t vtg, uint8_t zda, UpdateType ut) :
      Input_message(0x08),
      _gga_int(gga), _gsa_int(gsa), _gsv_int(gsv), _gll_int(gll),
      _rmc_int(rmc), _vtg_int(vtg), _zda_int(zda), _update_type(ut)
    {}

    GETTER_SETTER(uint8_t, GGA_interval, _gga_int);
    GETTER_SETTER(uint8_t, GSA_interval, _gsa_int);
    GETTER_SETTER(uint8_t, GSV_interval, _gsv_int);
    GETTER_SETTER(uint8_t, GLL_interval, _gll_int);
    GETTER_SETTER(uint8_t, RMC_interval, _rmc_int);
    GETTER_SETTER(uint8_t, VTG_interval, _vtg_int);
    GETTER_SETTER(uint8_t, ZDA_interval, _zda_int);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_NMEA_msg


  //! CONFIGURE MESSAGE TYPE - Configure and select output message type
  class Config_msg_type : public Input_message {
  private:
    MessageType _msg_type;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 2);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param mt Message type
      \param ut Update type
     */
    inline Config_msg_type(MessageType mt, UpdateType ut) :
      Input_message(0x09),
      _msg_type(mt),
      _update_type(ut)
    {}

    GETTER_SETTER(MessageType, message_type, _msg_type);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_msg_type


  //! SOFTWARE IMAGE DOWNLOAD - Download software image to system flash
  class Sw_img_download : public Input_message {
  private:
    BaudRate _baud_rate;
    FlashType _flash_type;
    uint16_t _flash_id;
    BufferUsed _buffer_used;

    GETTER(Payload_length, body_length, 5);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor using 'automatic' flash type and flash ID of 0
    /*!
      \param br Baud rate
      \param bu Size of buffer used
     */
    inline Sw_img_download(BaudRate br, BufferUsed bu) :
      Input_message(0x0b),
      _baud_rate(br),
      _flash_type(FlashType::Auto), _flash_id(0),
      _buffer_used(bu)
    {}

    //! Constructor
    /*!
      \param br Baud rate
      \param ft Flash type
      \param fid Flash ID (if flash type is not 'auto')
      \param bu Size of buffer used
     */
    inline Sw_img_download(BaudRate br, FlashType ft, uint16_t fid, BufferUsed bu) :
      Input_message(0x0b),
      _baud_rate(br),
      _flash_type(ft), _flash_id(fid),
      _buffer_used(bu)
    {}

    GETTER_SETTER(BaudRate, baud_rate, _baud_rate);
    GETTER_SETTER(FlashType, flash_type, _flash_type);
    GETTER_SETTER(BufferUsed, buffer_used, _buffer_used);

  }; // class Sw_img_download


  //! CONFIGURE SYSTEM POWER MODE - Set the power mode of GNSS system
  class Config_sys_power_mode : public Input_message {
  private:
    PowerMode _power_mode;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 2);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param pm Power mode
      \param ut Update type
     */
    Config_sys_power_mode(PowerMode pm, UpdateType ut) :
      Input_message(0x0c),
      _power_mode(pm),
      _update_type(ut)
    {}

    GETTER_SETTER(PowerMode, power_mode, _power_mode);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_sys_power_mode


  //! CONFIGURE SYSTEM POSITION RATE - Configure the position update rate of GNSS system
  class Config_sys_pos_rate : public Input_message {
  private:
    uint8_t _rate;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 2);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param r Rate - 1, 2, 4, 5, 8, 10, 20, 25, 40, or 50 (Hz)
      \param ut Update type
     */
    Config_sys_pos_rate(uint8_t r, UpdateType ut) :
      Input_message(0x0e),
      _rate(r),
      _update_type(ut)
    {}

    GETTER_SETTER(uint8_t, rate, _rate);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_sys_pos_rate


  //! QUERY POSITION UPDATE RATE - Query the position update rate of GNSS system
  //! - Responds with Pos_update_rate message
  class Q_pos_update_rate : public Input_message, public with_response {
  public:
    Q_pos_update_rate(void) :
      Input_message(0x10)
    {}

    RESPONSE1(0x86);

  }; // class Q_pos_update_rate


  //! CONFIGURE NAVIGATION DATA MESSAGE INTERVAL - Configure binary navigation data message interval
  //! Only valid on non-raw NavSparks i.e -GPS, -GL, -BD models
  class Config_nav_data_msg_interval : public Input_message {
  private:
    uint8_t _interval;	// 0: disable
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 2);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param i Navigation data message interval (seconds)
      \param ut Update type
     */
    Config_nav_data_msg_interval(uint8_t i, UpdateType ut) :
      Input_message(0x11),
      _interval(i),
      _update_type(ut)
    {}

    GETTER_SETTER(uint8_t, interval, _interval);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_nav_data_msg_interval


  //! Get Almanac - Get almanac used of firmware
  //! Only valid on NavSpark-Raws
  class Get_almanac : public Input_message, public with_response {
  private:
    uint8_t _sv_num;

    GETTER(Payload_length, body_length, 1);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param sv SV number of satellite, or 0 for all
    */
    Get_almanac(uint8_t sv) :
      Input_message(0x11),
      _sv_num(sv)
    {}

    GETTER_SETTER(uint8_t, SV_num, _sv_num);

    RESPONSE1(0x87);

  }; // class Get_almanac


  //! Configure Binary Measurement Output Rates
  //! Only valid on NavSpark-Raws
  class Config_bin_measurement_output_rates : public Input_message {
  private:
    OutputRate _output_rate;
    bool _meas_time, _raw_meas, _sv_ch_status, _rcv_state, _subframe;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 7);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param o Output rate
      \param mt Enable Meas_time reporting
      \param rm Enable Raw_meas reporting
      \param svch Enable SV_ch_status reporting
      \param rcv Enable RCV_state reporting
      \param sub Enable subframe reporting
      \param ut Update type
     */
    Config_bin_measurement_output_rates(OutputRate o, bool mt, bool rm, bool svch, bool rcv, bool sub, UpdateType ut) :
      Input_message(0x12),
      _output_rate(o),
      _meas_time(mt), _raw_meas(rm), _sv_ch_status(svch), _rcv_state(rcv), _subframe(sub),
      _update_type(ut)
    {}

    GETTER_SETTER(OutputRate, output_rate, _output_rate);
    GETTER(bool, meas_time, _meas_time);
    SETTER_BOOL(meas_time, _meas_time);

    GETTER(bool, raw_meas, _raw_meas);
    SETTER_BOOL(raw_meas, _raw_meas);

    GETTER(bool, SV_CH_status, _sv_ch_status);
    SETTER_BOOL(SV_CH_status, _sv_ch_status);

    GETTER(bool, RCV_state, _rcv_state);
    SETTER_BOOL(RCV_state, _rcv_state);

    GETTER(bool, subframe, _subframe);
    SETTER_BOOL(subframe, _subframe);

    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_bin_measurement_output_rates


  //! QUERY POWER MODE - Query status of power mode of GNSS receiver
  //! - Responds with GNSS_power_mode_status message
  class Q_power_mode : public Input_message, public with_response {
  public:
    Q_power_mode(void) :
      Input_message(0x15)
    {}

    RESPONSE1(0xB9);

  }; // class Q_power_mode


  //! CONFIGURE DATUM - Configure datum used for GNSS position transformation
  class Config_datum : public Input_message {
  private:
    uint16_t _datum_index;
    uint8_t _ellip_index;
    int16_t _delta_x, _delta_y, _delta_z; // metres
    uint32_t _semi_major_axis, _inv_flattening;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 18);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param di Datum index (see appendix B of AN0028)
      \param ei Ellipsoid index (see appendix A)
      \param dx,dy,dz Delta X, Y, and Z (metres)
      \param sma Semi-major axis
      \param inf Inverse flattening
      \param ut Update type
     */
    Config_datum(uint16_t di, uint8_t ei,
		 int16_t dx, int16_t dy, int16_t dz,
		 uint32_t sma, uint32_t inf, UpdateType ut) :
      Input_message(0x29),
      _datum_index(di), _ellip_index(ei),
      _delta_x(dx), _delta_y(dy), _delta_z(dz),
      _semi_major_axis(sma), _inv_flattening(inf),
      _update_type(ut)
    {}

    GETTER_SETTER(uint16_t, datum_index, _datum_index);
    GETTER_SETTER(uint8_t, ellipsoid_index, _ellip_index);
    GETTER_SETTER(int16_t, delta_X, _delta_x);
    GETTER_SETTER(int16_t, delta_Y, _delta_y);
    GETTER_SETTER(int16_t, delta_Z, _delta_z);
    GETTER_SETTER(uint32_t, semi_major_axis, _semi_major_axis);
    GETTER_SETTER(uint32_t, inv_flattening, _inv_flattening);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_datum


  //! CONFIGURE DOP MASK - Configure values of DOP mask
  class Config_DOP_mask : public Input_message {
  private:
    DOPmode _dop_mode;
    uint16_t _pdop, _hdop, _gdop; // * 0.1
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 8);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor with floating-point parameters
    /*!
      \param m DOP mode
      \param p,h,g PDOP, HDOP, and GDOP mask value
      \param ut Update type
     */
    Config_DOP_mask(DOPmode m, double p, double h, double g, UpdateType ut) :
      Input_message(0x2A),
      _dop_mode(m),
      _pdop(p * 10), _hdop(h * 10), _gdop(g * 10),
      _update_type(ut)
    {}

    //! Constructor with raw integer parameters
    /*!
      \param m DOP mode
      \param p,h,g PDOP, HDOP, and GDOP mask value (scaled by 10)
      \param ut Update type
     */
    Config_DOP_mask(DOPmode m, uint16_t p, uint16_t h, uint16_t g, UpdateType ut) :
      Input_message(0x2A),
      _dop_mode(m),
      _pdop(p), _hdop(h), _gdop(g),
      _update_type(ut)
    {}

    GETTER_SETTER(DOPmode, DOP_mode, _dop_mode);

    GETTER_SETTER_MOD(double, PDOP, _pdop, _pdop * 0.1, val * 10);
    GETTER_SETTER_RAW(uint16_t, PDOP, _pdop);

    GETTER_SETTER_MOD(double, HDOP, _hdop, _hdop * 0.1, val * 10);
    GETTER_SETTER_RAW(uint16_t, HDOP, _hdop);

    GETTER_SETTER_MOD(double, GDOP, _gdop, _gdop * 0.1, val * 10);
    GETTER_SETTER_RAW(uint16_t, GDOP, _gdop);

    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_DOP_mask


  //! CONFIGURE ELEVATION AND CNR MASK - Configure values of elevation and CNR mask
  class Config_elevation_CNR_mask : public Input_message {
  private:
    ElevationCNRmode _mode_select;
    uint8_t _el_mask, _cnr_mask;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 8);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param ms Elevation and CNR mode
      \param em Elevation mask (degrees)
      \param cm CNR mask (dB)
      \param ut Update type
     */
    Config_elevation_CNR_mask(ElevationCNRmode ms, uint8_t em, uint8_t cm, UpdateType ut) :
      Input_message(0x2B),
      _mode_select(ms),
      _el_mask(em), _cnr_mask(cm),
      _update_type(ut)
    {}

    GETTER_SETTER(ElevationCNRmode, mode_select, _mode_select);
    GETTER_SETTER(uint8_t, elevation_mask, _el_mask);
    GETTER_SETTER(uint8_t, CNR_mask, _cnr_mask);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_elevation_CNR_mask


  //! QUERY DATUM - Query datum used by the GNSS receiver
  //! - Responds with GNSS_datum message
  class Q_datum : public Input_message, public with_response {
  public:
    Q_datum(void) :
      Input_message(0x2D)
    {}

    RESPONSE1(0xAE);

  }; // class Q_datum


  //! QUERY DOP MASK - Query information of DOP mask used by the GNSS receiver
  //! - Responds with GNSS_DOP_mask message
  class Q_DOP_mask : public Input_message, public with_response {
  public:
    Q_DOP_mask(void) :
      Input_message(0x2E)
    {}

    RESPONSE1(0xAF);

  }; // class Q_DOP_mask


  //! QUERY ELEVATION AND CNR MASK - Query elevation and CNR mask used by the GNSS receiver
  //! - Responds with GNSS_elevation_CNR_mask message
  class Q_elevation_CNR_mask : public Input_message, public with_response {
  public:
    Q_elevation_CNR_mask(void) :
      Input_message(0x2F)
    {}

    RESPONSE1(0xB0);

  }; // class Q_elevation_CNR_mask


  //! GET GPS EPHEMERIS - Get GPS ephemeris used of GNSS receiver
  //! - Responds with GPS_ephemeris_data message
  class Get_GPS_ephemeris : public Input_message, public with_response {
  private:
    uint8_t _sv_num;

    GETTER(Payload_length, body_length, 1);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param sv SV number of satellite, 0 for all
     */
    Get_GPS_ephemeris(uint8_t sv) :
      Input_message(0x30),
      _sv_num(sv)
    {}

    RESPONSE1(0xB1);

    GETTER_SETTER(uint8_t, SV_number, _sv_num);

  }; // class Get_GPS_ephemeris


  //! CONFIGURE POSITION PINNING - Enable or disable position pinning of GNSS receiver
  class Config_pos_pinning : public Input_message {
  private:
    DefaultOrEnable _pinning;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 2);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param p Enable position pinning
      \param ut Update type
     */
    Config_pos_pinning(DefaultOrEnable p, UpdateType ut) :
      Input_message(0x39),
      _pinning(p), _update_type(ut)
    {}

    GETTER_SETTER(DefaultOrEnable, pinning, _pinning);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_pos_pinning


  //! QUERY POSITION PINNING - Query position pinning status of GNSS receiver
  //! - Responds with GNSS_pos_pinning_status
  class Q_pos_pinning : public Input_message, public with_response {
  public:
    Q_pos_pinning(void) :
      Input_message(0x3A)
    {}

    RESPONSE1(0xB4);

  }; // class Q_pos_pinning


  //! CONFIGURE POSITION PINNING PARAMETERS - Set position pinning parameters of GNSS receiver
  class Config_pos_pinning_params : public Input_message {
  private:
    uint16_t _pin_speed, _pin_count;
    uint16_t _unpin_speed, _unpin_count, _unpin_dist;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 1);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param ps Pinning speed (km/h)
      \param pc Pinning count
      \param us Unpinning speed (km/h)
      \param uc Unpinning count
      \param ud Unpinning distance (metres)
      \param ut Update type
    */
    Config_pos_pinning_params(uint16_t ps, uint16_t pc,
			      uint16_t us, uint16_t uc, uint16_t ud, UpdateType ut) :
      Input_message(0x3B),
      _pin_speed(ps), _pin_count(pc),
      _unpin_speed(us), _unpin_count(uc), _unpin_dist(ud),
      _update_type(ut)
    {}

    GETTER_SETTER(uint16_t, pinning_speed, _pin_speed);
    GETTER_SETTER(uint16_t, pinning_count, _pin_count);
    GETTER_SETTER(uint16_t, unpinning_speed, _unpin_speed);
    GETTER_SETTER(uint16_t, unpinning_count, _unpin_count);
    GETTER_SETTER(uint16_t, unpinning_distance, _unpin_dist);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_pos_pinning_params


  //! SET GPS EPHEMERIS - Set GPS ephemeris to GNSS receiver
  class Set_GPS_ephemeris : public Input_message {
  private:
    uint16_t _sv_num;
    uint8_t _subframe1[28], _subframe2[28], _subframe3[28];

    GETTER(Payload_length, body_length, 86);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param sv SV number of satellite
      \param sf1,sf2,sf3 Subframe data
     */
    Set_GPS_ephemeris(uint16_t sv, uint8_t sf1[28], uint8_t sf2[28], uint8_t sf3[28]) :
      Input_message(0x41),
      _sv_num(sv)
    {
      memcpy(&_subframe1, sf1, 28);
      memcpy(&_subframe2, sf2, 28);
      memcpy(&_subframe3, sf3, 28);
    }

    GETTER_SETTER(uint16_t, SV_number, _sv_num);

    GETTER(uint8_t*, subframe1, _subframe1);
    inline void set_subframe1(uint8_t sf1[28]) { memcpy(&_subframe1, sf1, 28); }

    GETTER(uint8_t*, subframe2, _subframe2);
    inline void set_subframe2(uint8_t sf2[28]) { memcpy(&_subframe2, sf2, 28); }

    GETTER(uint8_t*, subframe3, _subframe3);
    inline void set_subframe3(uint8_t sf3[28]) { memcpy(&_subframe3, sf3, 28); }

  }; // class Set_GPS_ephemeris


  //! QUERY 1PPS TIMING - Query 1PPS timing of the GNSS receiver
  //! Supported only in Venus838LPx-T, S1216F8-T timing mode receivers.
  class Q_1PPS_timing : public Input_message, public with_response {
  public:
    Q_1PPS_timing(void) :
      Input_message(0x44)
    {}

    RESPONSE1(0xc2);

  }; // class Q_1PPS_timing


  //! CONFIGURE 1PPS CABLE DELAY - Configure cable delay of 1PPS timing
  class Config_1PPS_cable_delay : public Input_message {
  private:
    int32_t _delay;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 5);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor from raw integer values
    /*!
      \param d Delay (1/100 ns)
      \param ut Update type
     */
    Config_1PPS_cable_delay(int32_t d, UpdateType ut) :
      Input_message(0x45),
      _delay(d), _update_type(ut)
    {}

    //! Constructor from floating-point values
    /*!
      \param d Delay (seconds)
      \param ut Update type
     */
    Config_1PPS_cable_delay(double d, UpdateType ut) :
      Input_message(0x45),
      _delay(d * 1e+11), _update_type(ut)
    {}

    GETTER_SETTER_MOD(double, delay, _delay, _delay * 1.0e-11, val * 1e+11);
    GETTER_SETTER_RAW(int32_t, delay, _delay);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_1PPS_cable_delay


  //! QUERY 1PPS CABLE DELAY - Query 1PPS cable delay of the GNSS receiver
  //! - Responds with GNSS_1PPS_cable_delay
  class Q_1PPS_cable_delay : public Input_message, public with_response {
  public:
    Q_1PPS_cable_delay(void) :
      Input_message(0x46)
    {}

    RESPONSE1(0xBB);

  }; // class Q_1PPS_cable_delay


  //! CONFIGURE NMEA TALKER ID - Configure NMEA talker ID of GNSS receive
  class Config_NMEA_talker_ID : public Input_message {
  private:
    TalkerID _talker_id;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 2);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param id Talker ID
      \param ut Update type
     */
    Config_NMEA_talker_ID(TalkerID id, UpdateType ut) :
      Input_message(0x4b),
      _talker_id(id),
      _update_type(ut)
    {}

    GETTER_SETTER(TalkerID, talker_id, _talker_id);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_NMEA_talker_ID


  //! QUERY NMEA TALKER ID - Query NMEA talker ID of GNSS receiver
  //! - Responds with NMEA_talker_ID message
  class Q_NMEA_talker_ID : public Input_message, public with_response {
  public:
    Q_NMEA_talker_ID(void) :
      Input_message(0x4f)
    {}

    RESPONSE1(0x93);

  }; // class Q_NMEA_talker_ID


  //! CONFIGURE 1PPS TIMING - Configure 1PPS timing of the GNSS receiver
  //! Supported only in Venus838LPx-T, S1216F8-T timing mode receivers.
  class Config_1PPS_timing : public Input_message {
  private:
    SkyTraq::TimingMode _timing_mode;
    uint32_t _survey_len, _std_dev;
    double _lat, _lon;
    float _alt;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 30);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param tm Timing mode
      \param sl Survey length when in survey mode
      \param sd Standard deviation when in survey mode
      \param lat Lattitude when in static mode
      \param lon Longitude when in static mode
      \param alt Altitude when in static mode
      \param ut Update type
     */
    Config_1PPS_timing(SkyTraq::TimingMode tm, uint32_t sl, uint32_t sd, double lat, double lon, float alt, UpdateType ut) :
      Input_message(0x54),
      _timing_mode(tm),
      _survey_len(sl),
      _std_dev(sd),
      _lat(lat), _lon(lon), _alt(alt),
      _update_type(ut)
    {}

    GETTER_SETTER(SkyTraq::TimingMode, timing_mode, _timing_mode);
    GETTER_SETTER(uint32_t, survey_length, _survey_len);
    GETTER_SETTER(uint32_t, standard_deviation, _std_dev);
    GETTER_SETTER(double, lattitude, _lat);
    GETTER_SETTER(double, longitude, _lon);
    GETTER_SETTER(float, altitude, _alt);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_1PPS_timing


}; // namespace SkyTraqBin

#endif // __SKYTRAQBIN_INPUTS_HH__
