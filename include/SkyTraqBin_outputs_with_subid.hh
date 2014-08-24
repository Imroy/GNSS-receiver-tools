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
#ifndef __SKYTRAQBIN_OUTPUTS_WITH_SUBID_HH__
#define __SKYTRAQBIN_OUTPUTS_WITH_SUBID_HH__

#include "SkyTraqBin.hh"

/*
  Sources:
  https://store-lgdi92x.mybigcommerce.com/content/AN0028_1.4.31.pdf	(Binary messages of Skytraq Venus 8)
  https://store-lgdi92x.mybigcommerce.com/content/AN0024_v07.pdf	(Raw measurement binary messages of Skytraq 6 & 8)
  https://store-lgdi92x.mybigcommerce.com/content/SUP800F_v0.6.pdf	(Skytraq SUP800F datasheet)
*/

namespace SkyTraqBin {

  /* All output message class names shall start with a noun

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

  //! Base class for messages that come from the GPS receiver with a sub-ID
  class Output_message_with_subid : public Output_message, public with_subid {
  protected:

  public:
    //! Constructor from a binary buffer
    inline Output_message_with_subid(unsigned char* payload, Payload_length payload_len) :
      Output_message(payload, payload_len),
      with_subid(payload_len > 1 ? payload[1] : 0)
    {}

    typedef std::shared_ptr<Output_message_with_subid> ptr;
  }; // class Output_message_with_subid


  //! SBAS STATUS - SBAS status of GNSS receiver
  //! - Answer to Q_SBAS_status
  class GNSS_SBAS_status : public Output_message_with_subid {
  private:
    bool _enabled;
    EnableOrAuto _ranging;
    uint8_t _ranging_ura_mask;
    bool _correction;
    uint8_t _num_channels;
    bool _waas, _egnos, _msas;

  public:
    GNSS_SBAS_status(unsigned char* payload, Payload_length payload_len);

    GETTER(bool, enabled, _enabled);
    GETTER(EnableOrAuto, ranging, _ranging);
    GETTER(uint8_t, ranging_URA_mask, _ranging_ura_mask);
    GETTER(bool, correction, _correction);
    GETTER(uint8_t, num_channels, _num_channels);
    GETTER(bool, WAAS_enabled, _waas);
    GETTER(bool, EGNOS_enabled, _egnos);
    GETTER(bool, MSAS_enabled, _msas);

  }; // class GNSS_SBAS_status


  //! QZSS STATUS - QZSS status of GNSS receiver
  class GNSS_QZSS_status : public Output_message_with_subid {
  private:
    bool _enabled;
    uint8_t _num_channels;

  public:
    GNSS_QZSS_status(unsigned char* payload, Payload_length payload_len);

    GETTER(bool, enabled, _enabled);
    GETTER(uint8_t, num_channels, _num_channels);

  }; // class GNSS_QZSS_status


  //! SAEE STATUS - SAEE status of GNSS receiver
  class GNSS_SAEE_status : public Output_message_with_subid {
  private:
    DefaultOrEnable _enabled;

  public:
    GNSS_SAEE_status(unsigned char* payload, Payload_length payload_len);

    GETTER(DefaultOrEnable, enabled, _enabled);


  }; // class GNSS_SAEE_status


  //! GNSS BOOT STATUS - Boot status of GNSS receiver
  //! - Answer to Q_GNSS_boot_status
  class GNSS_boot_status : public Output_message_with_subid {
  private:
    BootStatus _status;
    bool _winbond, _eon, _parallel;

  public:
    GNSS_boot_status(unsigned char* payload, Payload_length payload_len);

    GETTER(BootStatus, status, _status);

    GETTER_MOD(bool, ROM, !(_winbond || _eon || _parallel));
    GETTER(bool, Winbond_flash, _winbond);
    GETTER(bool, EON_flash, _eon);
    GETTER(bool, parallel_flash, _parallel);

  }; // class GNSS_boot_status


  //! EXTENDED NMEA MESSAGE INTERVAL - Extended NMEA message interval of the GNSS receiver
  class GNSS_extended_NMEA_msg_interval : public Output_message_with_subid {
  private:
    uint8_t _gga, _gsa, _gsv, _gll, _rmc, _vtg, _zda, _gns, _gbs, _grs, _dtm, _gst;

  public:
    GNSS_extended_NMEA_msg_interval(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, GGA_interval, _gga);
    GETTER(uint8_t, GSA_interval, _gsa);
    GETTER(uint8_t, GSV_interval, _gsv);
    GETTER(uint8_t, GLL_interval, _gll);
    GETTER(uint8_t, RMC_interval, _rmc);
    GETTER(uint8_t, VTG_interval, _vtg);
    GETTER(uint8_t, ZDA_interval, _zda);
    GETTER(uint8_t, GNS_interval, _gns);
    GETTER(uint8_t, GBS_interval, _gbs);
    GETTER(uint8_t, GRS_interval, _grs);
    GETTER(uint8_t, DTM_interval, _dtm);
    GETTER(uint8_t, GST_interval, _gst);

  }; // class GNSS_extended_NMEA_msg_interval


  //! INTERFERENCE DETECTION STATUS - Interference detection status of GNSS receiver
  //! - Answer to Q_interference_detection_status
  class GNSS_interference_detection_status : public Output_message_with_subid {
  private:
    bool _enabled;
    InterferenceStatus _status;

  public:
    GNSS_interference_detection_status(unsigned char* payload, Payload_length payload_len);

    GETTER(bool, enabled, _enabled);
    GETTER(InterferenceStatus, status, _status);

  }; // class GNSS_interference_detection_status


  //! GPS PARAMETER SEARCH ENGINE NUMBER - Number of parameter search engine of GPS receiver
  //! - Answer to Q_GPS_param_search_engine_num
  class GPS_param_search_engine_num : public Output_message_with_subid {
  private:
    ParameterSearchEngineMode _pse_mode;

  public:
    GPS_param_search_engine_num(unsigned char* payload, Payload_length payload_len);

    GETTER(ParameterSearchEngineMode, pse_mode, _pse_mode);

  }; // class GPS_param_search_engine_num


  //! GNSS NAVIGATION MODE - Navigation mode of the GNSS receiver
  //! - Answer to Q_GNSS_nav_mode
  class GNSS_nav_mode : public Output_message_with_subid {
  private:
    NavigationMode _mode;

  public:
    GNSS_nav_mode(unsigned char* payload, Payload_length payload_len);

    GETTER(NavigationMode, navigation_mode, _mode);

  }; // class GNSS_nav_mode


  //! GNSS CONSTELLATION TYPE FOR NAVIGATION SOLUTION - GNSS constellation type for navigation solution
  //! - Answer to Q_constellation_type
  class GNSS_constellation_type : public Output_message_with_subid {
  private:
    bool _gps, _glonass, _galileo, _beidou;

  public:
    GNSS_constellation_type(unsigned char* payload, Payload_length payload_len);

    GETTER(bool, GPS, _gps);
    GETTER(bool, GLONASS, _glonass);
    GETTER(bool, Galileo, _galileo);
    GETTER(bool, Beidou, _beidou);

  }; // class GNSS_constellation_type


  //! GPS TIME - GPS time of GNSS receiver
  class GNSS_time : public Output_message_with_subid {
  private:
    uint32_t _tow_ms, _tow_ns;
    uint16_t _week_num;
    int8_t _def_leap_secs, _curr_leap_secs;
    bool _tow_valid, _wn_valid, _ls_valid;

  public:
    GNSS_time(unsigned char* payload, Payload_length payload_len);

    GETTER_RAW(uint32_t, time_in_week, _tow_ms);
    GETTER_RAW(uint32_t, subtime_in_week, _tow_ns);
    GETTER_MOD(double, time_in_week, (_tow_ms * 1e-6) + (_tow_ns * 1e-9));

    GETTER(uint16_t, week_number, _week_num);
    GETTER(int8_t, default_leap_seconds, _def_leap_secs);
    GETTER(int8_t, current_leap_seconds, _curr_leap_secs);
    GETTER(bool, time_in_week_valid, _tow_valid);
    GETTER(bool, week_number_valid, _wn_valid);
    GETTER(bool, leap_seconds_valid, _ls_valid);

  }; // class GNSS_time


  //! 1PPS PULSE WIDTH - 1PPS pulse width of GNSS receiver
  //! - Answer to Q_1PPS_pulse_width
  class GNSS_1PPS_pulse_width : public Output_message_with_subid {
  private:
    uint32_t _width;

  public:
    GNSS_1PPS_pulse_width(unsigned char* payload, Payload_length payload_len);

    GETTER_MOD(double, width, _width * 1.0e-06);
    GETTER_RAW(uint32_t, width, _width);

  }; // class GNSS_1PPS_pulse_width


  //! Message of accelerometer, magnetometer, pressure and temperature data
  class Sensor_data : public Output_message_with_subid {
  private:
    float _gx, _gy, _gz, _mx, _my, _mz;
    uint32_t _pres;
    float _temp;

  public:
    Sensor_data(unsigned char* payload, Payload_length payload_len);

    GETTER(float, Gx, _gx);
    GETTER(float, Gy, _gy);
    GETTER(float, Gz, _gz);

    GETTER_MOD(float, Mx, _mx * 1e-6);
    GETTER_RAW(float, Mx, _mx);
    GETTER_MOD(float, My, _my * 1e-6);
    GETTER_RAW(float, My, _my);
    GETTER_MOD(float, Mz, _mz * 1e-6);
    GETTER_RAW(float, Mz, _mz);

    GETTER(uint32_t, pressure, _pres);
    GETTER(float, temperature, _temp);

  }; // class Sensor_data


}; // namespace SkyTraqBin

#endif // __SKYTRAQBIN_OUTPUTS_WITH_SUBID_HH__
