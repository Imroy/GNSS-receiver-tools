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


  void Config_sys_power_mode::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_power_mode));
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_sys_pos_rate::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _rate);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_NMEA_talker_ID::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_talker_id));
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Config_nav_data_msg_interval::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, _interval);
    append_be(buffer, static_cast<uint8_t>(_update_type));
  }


  void Sw_img_download::body_to_buf(unsigned char* buffer) const {
    append_be(buffer, static_cast<uint8_t>(_baud_rate));
    append_be(buffer, static_cast<uint8_t>(_flash_type));
    append_be(buffer, _flash_id);
    append_be(buffer, static_cast<uint8_t>(_buffer_used));
  }


}; // namespace SkyTraqBin
