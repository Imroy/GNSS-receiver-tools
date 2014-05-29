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


}; // namespace SkyTraqBin
