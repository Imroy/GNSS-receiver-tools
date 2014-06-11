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
#include <functional>
#include <iostream>
#include <iomanip>
#include <map>
#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include "SkyTraqBin.hh"
#include "BE.hh"

namespace SkyTraqBin {

  uint8_t checksum(unsigned char* buffer, Payload_length len) {
    uint8_t cs = 0;
    while (len) {
      cs ^= *buffer;
      buffer++;
      len--;
    }

    return cs;
  }


  void Input_message::to_buf(unsigned char* buffer) const {
    append_be<uint8_t>(buffer, 0xa0);
    append_be<uint8_t>(buffer, 0xa1);

    Payload_length payload_len = body_length() + 1; // include message ID
    append_be(buffer, payload_len);

    unsigned char *payload = buffer;
    append_be(buffer, _msg_id);
    body_to_buf(buffer);
    buffer = payload + payload_len;

    append_be(buffer, checksum(payload, payload_len));
    append_be<uint8_t>(buffer, 0x0d);
    append_be<uint8_t>(buffer, 0x0a);
  }


  void Input_message_with_subid::to_buf(unsigned char* buffer) const {
    append_be<uint8_t>(buffer, 0xa0);
    append_be<uint8_t>(buffer, 0xa1);

    Payload_length payload_len = body_length() + 1 + 1; // include message ID and sub-ID
    append_be(buffer, payload_len);

    unsigned char *payload = buffer;
    append_be(buffer, _msg_id);
    append_be(buffer, _msg_subid);
    body_to_buf(buffer);
    buffer = payload + payload_len;

    append_be(buffer, checksum(payload, payload_len));
    append_be<uint8_t>(buffer, 0x0d);
    append_be<uint8_t>(buffer, 0x0a);
  }


  typedef std::function<Output_message::ptr(uint8_t*, Payload_length)> output_message_factory;
#define OUTPUT1(ID, CLASS) std::make_pair<uint16_t, output_message_factory>((ID), [](uint8_t* payload, Payload_length len) { return std::make_shared<CLASS>(payload, len); })
#define OUTPUT2(ID, SUBID, CLASS) std::make_pair<uint16_t, output_message_factory>(((ID) << 8) | (SUBID), [](uint8_t* payload, Payload_length len) { return std::make_shared<CLASS>(payload, len); })

  std::map<uint16_t, output_message_factory> output_message_factories = {
    OUTPUT2(0x62, 0x80, GNSS_SBAS_status),
    OUTPUT2(0x62, 0x81, GNSS_QZSS_status),
    OUTPUT2(0x64, 0x80, GNSS_boot_status),
    OUTPUT2(0x64, 0x80, GNSS_1PPS_pulse_width),
    OUTPUT1(0x80, Sw_ver),
    OUTPUT1(0x81, Sw_CRC),
    OUTPUT1(0x83, Ack),
    OUTPUT1(0x84, Nack),
    OUTPUT1(0x86, Pos_update_rate),
    OUTPUT1(0x87, GPS_almanac_data),
    OUTPUT1(0x93, NMEA_talker_ID),
    OUTPUT1(0xA8, Nav_data_msg),
    OUTPUT1(0xAE, GNSS_datum),
    OUTPUT1(0xAF, GNSS_DOP_mask),
    OUTPUT1(0xB0, GNSS_elevation_CNR_mask),
    OUTPUT1(0xB1, GPS_ephemeris_data),
    OUTPUT1(0xB4, GNSS_pos_pinning_status),
    OUTPUT1(0xB9, GNSS_power_mode_status),
    OUTPUT1(0xBB, GNSS_1PPS_cable_delay),
    OUTPUT1(0xDC, Measurement_time),
    OUTPUT1(0xDD, Raw_measurements),
    OUTPUT1(0xDE, SV_channel_status),
    OUTPUT1(0xDF, Rcv_state),
    OUTPUT1(0xE0, Subframe_data),
  };
#undef OUTPUT1
#undef OUTPUT2


  Output_message::ptr parse_message(unsigned char* buffer, std::size_t len) {
    Payload_length payload_len = extract_be<uint16_t>(buffer, 2);
    std::size_t end = 2 + 2 + payload_len + 1 + 2;
    if (len < end)
      throw InsufficientData();

    if ((buffer[0] != 0xa0)
	|| (buffer[1] != 0xa1)
	|| (buffer[end - 2] != 0x0d)
	|| (buffer[end - 1] != 0x0a))
      throw InvalidMessage();

    unsigned char *payload = buffer + 2 + 2;
    uint16_t id = payload[0];
    if ((id >= 0x62) && (id <= 0x65))	// construct a composite id if the message has a sub-ID
      id = (payload[0] << 8) | payload[1];

    if (output_message_factories.count(id) == 0)
      throw UnknownMessageID(id);

    uint8_t cs = buffer[end - 3];
    {
      uint8_t ccs = checksum(payload, payload_len);
      if (cs != ccs)
	throw ChecksumMismatch(ccs, cs);
    }

    return output_message_factories[id](payload, payload_len);
  }


}; // namespace SkyTraqBin
