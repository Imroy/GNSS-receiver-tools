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
#include <stdlib.h>
#include "SkyTraqBin.hh"
#include "BE.hh"

namespace SkyTraqBin {

  std::ostream& operator<< (std::ostream& out, MessageType mt) {
    out << std::to_string(mt);
    return out;
  }


  void Restart_sys::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, (uint8_t)_start_mode);
    append_be(buffer, _utc_year);
    append_be(buffer, _utc_month);
    append_be(buffer, _utc_day);
    append_be(buffer, _utc_hour);
    append_be(buffer, _utc_minute);
    append_be(buffer, _utc_second);
    append_be(buffer, _lattitude);
    append_be(buffer, _longitude);
    append_be(buffer, _altitude);
  }


  void Q_sw_ver::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, (uint8_t)_sw_type);
  }


  void Q_sw_CRC::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, (uint8_t)_sw_type);
  }


  void Set_factory_defaults::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, (uint8_t)_reset);
  }


  void Config_serial_port::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _com_port);
    append_be(buffer, static_cast<uint8_t>(_baud_rate));
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_NMEA_msg::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _gga_int);
    append_be(buffer, _gsa_int);
    append_be(buffer, _gsv_int);
    append_be(buffer, _gll_int);
    append_be(buffer, _rmc_int);
    append_be(buffer, _vtg_int);
    append_be(buffer, _zda_int);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_msg_type::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_msg_type));
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Sw_img_download::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_baud_rate));
    append_be(buffer, static_cast<uint8_t>(_flash_type));
    append_be(buffer, _flash_id);
    append_be(buffer, static_cast<uint8_t>(_buffer_used));
  }


  void Config_sys_power_mode::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_power_mode));
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_sys_pos_rate::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _rate);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_nav_data_msg_interval::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _interval);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Get_almanac::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _sv_num);
  }


  void Config_bin_measurement_output_rates::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_output_rate));
    append_be(buffer, _meas_time);
    append_be(buffer, _raw_meas);
    append_be(buffer, _sv_ch_status);
    append_be(buffer, _rcv_state);
    append_be(buffer, _subframe);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_datum::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _datum_index);
    append_be(buffer, _ellip_index);
    append_be(buffer, _delta_x);
    append_be(buffer, _delta_y);
    append_be(buffer, _delta_z);
    append_be(buffer, _semi_major_axis);
    append_be(buffer, _inv_flattening);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_DOP_mask::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_dop_mode));
    append_be(buffer, _pdop);
    append_be(buffer, _hdop);
    append_be(buffer, _gdop);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_elevation_CNR_mask::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_mode_select));
    append_be(buffer, _el_mask);
    append_be(buffer, _cnr_mask);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Get_GPS_ephemeris::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _sv_num);
  }


  void Config_pos_pinning::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_pinning));
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_pos_pinning_params::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _pin_speed);
    append_be(buffer, _pin_count);
    append_be(buffer, _unpin_speed);
    append_be(buffer, _unpin_count);
    append_be(buffer, _unpin_dist);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Set_GPS_ephemeris::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _sv_num);
    int i;
    for (i = 0; i < 28; i++)
      append_be(buffer, _subframe1[i]);
    for (i = 0; i < 28; i++)
      append_be(buffer, _subframe2[i]);
    for (i = 0; i < 28; i++)
      append_be(buffer, _subframe3[i]);
  }


  void Config_1PPS_cable_delay::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _delay);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_NMEA_talker_ID::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_talker_id));
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  /**************************
   * Messages with a sub-ID *
   **************************/


  void Config_SBAS::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_enable));
    append_be(buffer, static_cast<uint8_t>(_ranging));
    append_be(buffer, _ranging_ura_mask);
    append_be(buffer, static_cast<uint8_t>(_correction));
    append_be(buffer, _num_channels);
    append_be(buffer, static_cast<uint8_t>(_waas) | (static_cast<uint8_t>(_egnos) << 1) | (static_cast<uint8_t>(_msas) << 2));
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_QZSS::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_enable));
    append_be(buffer, _num_channels);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_SAEE::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_enable));
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_extended_NMEA_msg_interval::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _gga);
    append_be(buffer, _gsa);
    append_be(buffer, _gsv);
    append_be(buffer, _gll);
    append_be(buffer, _rmc);
    append_be(buffer, _vtg);
    append_be(buffer, _zda);
    append_be(buffer, _gns);
    append_be(buffer, _gbs);
    append_be(buffer, _grs);
    append_be(buffer, _dtm);
    append_be(buffer, _gst);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_interference_detection::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_enable));
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_GNSS_nav_mode::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_mode));
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_1PPS_pulse_width::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _width);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


}; // namespace SkyTraqBin

namespace std {

  std::string to_string(SkyTraqBin::MessageType mt) {
    switch (mt) {
    case SkyTraqBin::MessageType::None:
      return "none";
      break;
    case SkyTraqBin::MessageType::NMEA0183:
      return "NMEA-0183";
      break;
    case SkyTraqBin::MessageType::Binary:
      return "binary";
      break;
    }
    return "";
  }

}; // namespace std
