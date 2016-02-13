/*
        Copyright 2016 Ian Tester

        This file is part of GNSS receiver tools.

        GNSS receiver tools is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        GNSS receiver tools is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with GNSS receiver tools.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdlib.h>
#include "UBX.hh"
#include "LE.hh"

/*
  Sources:
*/

namespace UBX {

  namespace Cfg {

    void Q_prt::body_to_buf(unsigned char* buffer) const {
      if (!_current_port)
	append_le(buffer, (uint8_t)_portid);
    }

    void Prt::body_to_buf(unsigned char* buffer) const {
      append_le(buffer, (uint8_t)_portid);
      append_le<uint8_t>(buffer, 0);	// reserved

      uint16_t txReady = 0;
      if (_txr_en) {
	txReady = 1;
	txReady |= (uint16_t)_txr_pol << 1;
	txReady |= (uint16_t)(_txr_pin & 0x1f) << 2;
	txReady |= (_txr_thres & 0x1ff) << 7;
      }
      append_le(buffer, txReady);

      uint32_t mode = 0;
      mode |= ((uint32_t)_mode_charlen & 0x03) << 6;
      mode |= ((uint32_t)_mode_parity & 0x07) << 9;
      mode |= ((uint32_t)_mode_stopbits & 0x03) << 12;
      append_le(buffer, mode);

      append_le(buffer, _baudrate);
      append_le<uint16_t>(buffer, _inproto_ubx | ((uint16_t)_inproto_nmea << 1) | ((uint16_t)_inproto_rtcm << 2));
      append_le<uint16_t>(buffer, _outproto_ubx | ((uint16_t)_outproto_nmea << 1));
      append_le<uint16_t>(buffer, (uint16_t)_flags_exttxtimeout << 1);
      append_le<uint8_t>(buffer, 0);	// reserved[2]
      append_le<uint8_t>(buffer, 0);
    }

    void Q_msg::body_to_buf(unsigned char* buffer) const {
      append_le(buffer, _msg_class);
      append_le(buffer, _msg_id);
    }

    void Msg::body_to_buf(unsigned char* buffer) const {
      append_le(buffer, _msg_class);
      append_le(buffer, _msg_id);
      if (_current_port)
	append_le(buffer, _rates[0]);
      else
	for (uint8_t i = 0; i < 6; i++)
	  append_le(buffer, _rates[i]);
    }

  }; // namespace Cfg

}; // namespace UBX
