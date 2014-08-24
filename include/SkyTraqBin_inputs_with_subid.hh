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
#ifndef __SKYTRAQBIN_INPUTS_WITH_SUBID_HH__
#define __SKYTRAQBIN_INPUTS_WITH_SUBID_HH__

#include "SkyTraqBin.hh"

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

  //! Base class for messages that go to the GPS receiver with a sub-ID
  class Input_message_with_subid : public Input_message, public with_subid {
  protected:
    //! The length of the body (not including message id or sub-id)
    virtual const Payload_length body_length(void) const { return 0; }

    //! Write body fields into a pre-allocated buffer
    virtual void body_to_buf(unsigned char* buffer) const {}

  public:
    //! Constructor
    Input_message_with_subid(uint8_t id, uint8_t subid) :
      Input_message(id),
      with_subid(subid)
    {}

    //! The total length of the message
    inline const Payload_length message_length(void) const { return StartSeq_len + PayloadLength_len + MsgID_len + MsgSubID_len + body_length() + Checksum_len + EndSeq_len; }

    //! Write the message into a buffer
    /*!
      Use message_length() to know how big the buffer needs to be.
     */
    virtual void to_buf(unsigned char *buffer) const;

    typedef std::shared_ptr<Input_message_with_subid> ptr;
  }; // class Input_message_with_subid


  //! CONFIGURE SBAS - Configure SBAS parameters of GNSS receiver
  class Config_SBAS : public Input_message_with_subid {
  private:
    bool _enable;
    EnableOrAuto _ranging;
    uint8_t _ranging_ura_mask;
    bool _correction;
    uint8_t _num_channels;
    bool _waas, _egnos, _msas, _all_sbas;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 8);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param en Enable or disable use of SBAS
      \param r Use SBAS satellite for ranging?
      \param rm Ranging URA mask (0~15, default 8)
      \param c Enable correction
      \param nc Number of tracking channels (0~3)
      \param w,e,m,a Enable use of WAAS, EGNOS, MSAS, or all SBAS satellites
      \param ut Update type
     */
    Config_SBAS(bool en, EnableOrAuto r, uint8_t rm, bool c, uint8_t nc, bool w, bool e, bool m, bool a, UpdateType ut) :
      Input_message_with_subid(0x62, 0x01),
      _enable(en),
      _ranging(r), _ranging_ura_mask(rm),
      _correction(c), _num_channels(nc),
      _waas(w), _egnos(e), _msas(m), _all_sbas(a),
      _update_type(ut)
    {}

    //! Constructor
    /*!
      \param en Enable or disable use of SBAS
      \param r Use SBAS satellite for ranging?
      \param rm Ranging URA mask (0~15, default 8)
      \param c Enable correction
      \param nc Number of tracking channels (0~3)
      \param w,e,m Enable use of WAAS, EGNOS, or MSAS satellites
      \param ut Update type
     */
    Config_SBAS(bool en, EnableOrAuto r, uint8_t rm, bool c, uint8_t nc, bool w, bool e, bool m, UpdateType ut) :
      Input_message_with_subid(0x62, 0x01),
      _enable(en),
      _ranging(r), _ranging_ura_mask(rm),
      _correction(c), _num_channels(nc),
      _waas(w), _egnos(e), _msas(m), _all_sbas(false),
      _update_type(ut)
    {}

    GETTER(bool, enabled, _enable);
    inline void enable(bool en=true) { _enable = en; }
    inline void disable(void) { _enable = false; }

    GETTER_SETTER(EnableOrAuto, ranging, _ranging);
    GETTER_SETTER(uint8_t, ranging_URA_mask, _ranging_ura_mask);

    GETTER(bool, correction, _correction);
    SETTER_BOOL(correction, _correction);

    GETTER_SETTER(uint8_t, num_channels, _num_channels);

    GETTER(bool, WAAS_enabled, _waas);
    inline void enable_WAAS(bool w=true) { _waas = w; }
    inline void disable_WAAS(void) { _waas = false; }

    GETTER(bool, EGNOS_enabled, _egnos);
    inline void enable_EGNOS(bool e=true) { _egnos = e; }
    inline void disable_EGNOS(void) { _egnos = false; }

    GETTER(bool, MSAS_enabled, _msas);
    inline void enable_MSAS(bool m=true) { _msas = m; }
    inline void disable_MSAS(void) { _msas = false; }

    GETTER(bool, All_SBAS_enabled, _all_sbas);
    inline void enable_All_SBAS(bool a=true) { _all_sbas = a; }
    inline void disable_All_SBAS(void) { _all_sbas = false; }

    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_SBAS


  //! QUERY SBAS STATUS - Query SBAS status of GNSS receiver
  //! - Responds with GNSS_SBAS_status
  class Q_SBAS_status : public Input_message_with_subid, public with_response {
  public:
    Q_SBAS_status(void) :
      Input_message_with_subid(0x62, 0x02)
    {}

    RESPONSE2(0x62, 0x80);

  }; // class Q_SBAS_status


  //! CONFIGURE QZSS - Configure QZSS of GNSS receiver
  class Config_QZSS : public Input_message_with_subid {
  private:
    bool _enable;
    uint8_t _num_channels;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 3);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param e Enable use of QZSS
      \param nc Number of tracking channels (1~3, default 1)
      \param ut Update type
     */
    Config_QZSS(bool e, uint8_t nc, UpdateType ut) :
      Input_message_with_subid(0x62, 0x03),
      _enable(e), _num_channels(nc), _update_type(ut)
    {}

    GETTER(bool, enabled, _enable);
    inline void enable(bool en=true) { _enable = en; }
    inline void disable(void) { _enable = false; }

    GETTER_SETTER(uint8_t, num_channels, _num_channels);

    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_QZSS


  //! QUERY QZSS STATUS - Query QZSS status of GNSS receiver
  class Q_QZSS_status : public Input_message_with_subid, public with_response {
  public:
    Q_QZSS_status(void) :
      Input_message_with_subid(0x62, 0x04)
    {}

    RESPONSE2(0x62, 0x81);

  }; // class Q_QZSS_status


  //! CONFIGURE SAEE - configure SAEE of GNSS receiver
  class Config_SAEE : public Input_message_with_subid {
  private:
    DefaultOrEnable _enable;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 2);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param e Enable SAEE
      \param ut Update type
     */
    Config_SAEE(DefaultOrEnable e, UpdateType ut) :
      Input_message_with_subid(0x63, 0x01),
      _enable(e), _update_type(ut)
    {}

    GETTER_SETTER(DefaultOrEnable, enable, _enable);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_SAEE


  //! QUERY SAEE STATUS
  class Q_SAEE_status : public Input_message_with_subid, public with_response {
  public:
    Q_SAEE_status(void) :
      Input_message_with_subid(0x63, 0x02)
    {}

    RESPONSE2(0x63, 0x80);

  }; // class Q_SAEE_status


  //! QUERY GNSS BOOT STATUS - Query boot status of GNSS receiver
  //! - Responds with GNSS_boot_status message
  class Q_GNSS_boot_status : public Input_message_with_subid, public with_response {
  public:
    Q_GNSS_boot_status(void) :
      Input_message_with_subid(0x64, 0x01)
    {}

    RESPONSE2(0x64, 0x80);

  }; // class Q_GNSS_boot_status


  //! CONFIGURE EXTENDED NMEA MESSAGE INTERVAL - Configure extended NMEA message Interval of GNSS receiver
  class Config_extended_NMEA_msg_interval : public Input_message_with_subid {
  private:
    uint8_t _gga, _gsa, _gsv, _gll, _rmc, _vtg, _zda, _gns, _gbs, _grs, _dtm, _gst;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 13);
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
      \param gns GNS sentence interval (seconds)
      \param gbs GBS sentence interval (seconds)
      \param grs GRS sentence interval (seconds)
      \param dtm DTM sentence interval (seconds)
      \param gst GST sentence interval (seconds)
      \param ut Update type
     */
    Config_extended_NMEA_msg_interval(uint8_t gga, uint8_t gsa, uint8_t gsv, uint8_t gll,
				      uint8_t rmc, uint8_t vtg, uint8_t zda, uint8_t gns,
				      uint8_t gbs, uint8_t grs, uint8_t dtm, uint8_t gst,
				      UpdateType ut) :
      Input_message_with_subid(0x64, 0x02),
      _gga(gga), _gsa(gsa), _gsv(gsv), _gll(gll),
      _rmc(rmc), _vtg(vtg), _zda(zda), _gns(gns),
      _gbs(gbs), _grs(grs), _dtm(dtm), _gst(gst),
      _update_type(ut)
    {}

    GETTER_SETTER(uint8_t, GGA_interval, _gga);
    GETTER_SETTER(uint8_t, GSA_interval, _gsa);
    GETTER_SETTER(uint8_t, GSV_interval, _gsv);
    GETTER_SETTER(uint8_t, GLL_interval, _gll);
    GETTER_SETTER(uint8_t, RMC_interval, _rmc);
    GETTER_SETTER(uint8_t, VTG_interval, _vtg);
    GETTER_SETTER(uint8_t, ZDA_interval, _zda);
    GETTER_SETTER(uint8_t, GNS_interval, _gns);
    GETTER_SETTER(uint8_t, GBS_interval, _gbs);
    GETTER_SETTER(uint8_t, GRS_interval, _grs);
    GETTER_SETTER(uint8_t, DTM_interval, _dtm);
    GETTER_SETTER(uint8_t, GST_interval, _gst);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_extended_NMEA_msg_interval


  //! QUERY EXTENDED NMEA MESSAGE INTERVAL
  class Q_extended_NMEA_msg_interval : public Input_message_with_subid, public with_response {
  public:
    Q_extended_NMEA_msg_interval(void) :
      Input_message_with_subid(0x64, 0x03)
    {}

    RESPONSE2(0x64, 0x81);

  }; // class Q_extended_NMEA_msg_interval


  //! CONFIGURE INTERFERENCE DETECTION - Configure the interference detection of GNSS receiver
  class Config_interference_detection : public Input_message_with_subid {
  private:
    bool _enable;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 2);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param e Enable interference detection
      \param ut Update type
     */
    Config_interference_detection(bool e, UpdateType ut) :
      Input_message_with_subid(0x64, 0x06),
      _enable(e), _update_type(ut)
    {}

    GETTER(bool, enabled, _enable);
    inline void enable(bool en=true) { _enable = en; }
    inline void disable(void) { _enable = false; }

    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_interference_detection


  //! QUERY INTERFERENCE DETECTION STATUS - Query the status of interference detection of the GNSS receiver
  class Q_interference_detection_status : public Input_message_with_subid, public with_response {
  public:
    Q_interference_detection_status(void) :
      Input_message_with_subid(0x64, 0x07)
    {}

    RESPONSE2(0x64, 0x83);

  }; // class Q_interference_detection_status


  //! CONFIGURE GPS PARAMETER SEARCH ENGINE NUMBER - Configure the parameter search engine number of GPS receiver
  class Config_GPS_param_search_engine_num : public Input_message_with_subid {
  private:
    ParameterSearchEngineMode _pse_mode;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 2);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    Config_GPS_param_search_engine_num(ParameterSearchEngineMode pm, UpdateType ut) :
      Input_message_with_subid(0x64, 0x0a),
      _pse_mode(pm), _update_type(ut)
    {}

    GETTER_SETTER(ParameterSearchEngineMode, pse_mode, _pse_mode);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_GPS_param_search_engine_num


  //! QUERY GPS PARAMETER SEARCH ENGINE NUMBER - Query the parameter search engine number of the GPS receiver
  class Q_GPS_param_search_engine_num : public Input_message_with_subid, public with_response {
  public:
    Q_GPS_param_search_engine_num(void) :
      Input_message_with_subid(0x64, 0x0b)
    {}

    RESPONSE2(0x64, 0x85);

  }; // class Q_GPS_param_search_engine_num


  //! CONFIGURE GNSS NAVIGATION MODE - Configure the navigation mode of GNSS receiver
  class Config_GNSS_nav_mode : public Input_message_with_subid {
  private:
    NavigationMode _mode;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 2);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param m Navigation mode
      \param ut Update type
     */
    Config_GNSS_nav_mode(NavigationMode m, UpdateType ut) :
      Input_message_with_subid(0x64, 0x17),
      _mode(m), _update_type(ut)
    {}

    GETTER_SETTER(NavigationMode, navigation_mode, _mode);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_GNSS_nav_mode


  //! QUERY GNSS NAVIGATION MODE - Query the navigation mode of GNSS receiver
  class Q_GNSS_nav_mode : public Input_message_with_subid, public with_response {
  public:
    Q_GNSS_nav_mode(void) :
      Input_message_with_subid(0x64, 0x18)
    {}

    RESPONSE2(0x64, 0x8B);

  }; // class Q_GNSS_nav_mode


  //! CONFIGURE GNSS CONSTELLATION TYPE FOR NAVIGATION SOLUTION - Set the GNSS constellation type for navigation solution
  class Config_constellation_type : public Input_message_with_subid {
  private:
    bool _gps, _glonass, _galileo, _beidou;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 3);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param gp Enable GPS
      \param gl Enable Glonass
      \param ga Enable Galileo
      \param bd Enable Beidou
      \param ut Update type
     */
    Config_constellation_type(bool gp, bool gl, bool ga, bool bd, UpdateType ut) :
      Input_message_with_subid(0x64, 0x19),
      _gps(gp), _glonass(gl), _galileo(ga), _beidou(bd), _update_type(ut)
    {}

    GETTER(bool, GPS, _gps);
    SETTER_BOOL(GPS, _gps);

    GETTER(bool, GLONASS, _glonass);
    SETTER_BOOL(GLONASS, _glonass);

    GETTER(bool, Galileo, _galileo);
    SETTER_BOOL(Galileo, _galileo);

    GETTER(bool, Beidou, _beidou);
    SETTER_BOOL(Beidou, _beidou);

    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_constellation_type


  //! QUERY GNSS CONSTELLATION TYPE FOR NAVIGATION SOLUTION - Query the GNSS constellation type for navigation solution
  class Q_constellation_type : public Input_message_with_subid, public with_response {
  public:
    Q_constellation_type(void) :
      Input_message_with_subid(0x64, 0x1A)
    {}

    RESPONSE2(0x64, 0x8C);

  }; // class Q_constellation_type


  //! CONFIGURE GPS/UTC LEAP SECONDS - Configure GPS/UTC leap seconds of GNSS receiver
  class Config_leap_seconds : public Input_message_with_subid {
  private:
    int8_t _seconds;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 2);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param s Leap seconds (seconds)
      \param ut Update type
     */
    Config_leap_seconds(int8_t s, UpdateType ut) :
      Input_message_with_subid(0x64, 0x1f),
      _seconds(s), _update_type(ut)
    {}

    GETTER_SETTER(int8_t, seconds, _seconds);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_leap_seconds


  //! QUERY GPS TIME - Query GPS time of GNSS receiver
  class Q_GPS_time : public Input_message_with_subid, public with_response {
  public:
    Q_GPS_time(void) :
      Input_message_with_subid(0x64, 0x20)
    {}

    RESPONSE2(0x64, 0x8E);

  }; // class Q_GPS_time


  //! CONFIGURE 1PPS PULSE WIDTH - Configure 1PPS pulse width of GNSS receiver
  class Config_1PPS_pulse_width : public Input_message_with_subid {
  private:
    uint32_t _width;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 5);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor from raw integer values
    /*!
      \param w Pulse width (microseconds, 1~100000)
      \param ut Update type
     */
    Config_1PPS_pulse_width(uint32_t w, UpdateType ut) :
      Input_message_with_subid(0x65, 0x01),
      _width(w), _update_type(ut)
    {}

    //! Constructor from floating-point values
    /*!
      \param w Pulse width (seconds, 1~100000 us)
      \param ut Update type
     */
    Config_1PPS_pulse_width(double w, UpdateType ut) :
      Input_message_with_subid(0x65, 0x01),
      _width(w * 1e+6), _update_type(ut)
    {}

    GETTER_SETTER_MOD(double, width, _width, _width * 1.0e-6, val * 1e+6);
    GETTER_SETTER_RAW(int32_t, width, _width);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_1PPS_pulse_width


  //! QUERY 1PPS PULSE WIDTH - Query 1PPS pulse width of GNSS receiver
  //! - Answer with GNSS_1PPS_pulse_width
  class Q_1PPS_pulse_width : public Input_message_with_subid, public with_response {
  public:
    Q_1PPS_pulse_width(void) :
      Input_message_with_subid(0x65, 0x02)
    {}

    RESPONSE2(0x65, 0x80);

  }; // class Q_1PPS_pulse_width


  //! CONFIGURE 1PPS FREQUENCY OUTPUT - Configure frequency output of 1PPS
  //! Supported only in Flash V8 version
  class Config_1PPS_freq_output : public Input_message_with_subid {
  private:
    uint32_t _frequency;
    UpdateType _update_type;

    GETTER(Payload_length, body_length, 5);
    virtual void body_to_buf(unsigned char* buffer) const;

  public:
    //! Constructor
    /*!
      \param freq Output frequency, in Hz (0~10000000)
      \param ut Update type
    */
    Config_1PPS_freq_output(uint32_t freq, UpdateType ut) :
      Input_message_with_subid(0x65, 0x03),
      _frequency(freq), _update_type(ut)
    {}

    GETTER_SETTER(uint32_t, frequency, _frequency);
    GETTER_SETTER(UpdateType, update_type, _update_type);

  }; // class Config_1PPS_freq_output


  //! QUERY 1PPS FREQUENCY OUTPUT - Query 1PPS frequency output of the GNSS receive
  //! Supported only in Flash V8 version
  class Q_1PPS_freq_output : public Input_message_with_subid, public with_response {
  public:
    Q_1PPS_freq_output(void) :
      Input_message_with_subid(0x65, 0x04)
    {}
  }; // class Q_1PPS_freq_output

}; // namespace SkyTraqBin

#endif // __SKYTRAQBIN_INPUTS_WITH_SUBID_HH__
