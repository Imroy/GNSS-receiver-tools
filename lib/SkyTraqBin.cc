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

    append_be(buffer, checksum(payload, payload_len));
    append_be<uint8_t>(buffer, 0x0d);
    append_be<uint8_t>(buffer, 0x0a);
  }


  typedef Output_message::ptr (*output_message_factory)(uint8_t* payload, Payload_length payload_len);
#define OUTPUT(ID, CLASS) std::make_pair<uint8_t, output_message_factory>(ID, [](uint8_t* payload, Payload_length len) -> Output_message::ptr { return std::make_shared<CLASS>(payload, len); })

  std::map<uint8_t, output_message_factory> output_message_factories = {
    OUTPUT(0x80, Sw_ver),
    OUTPUT(0x81, Sw_CRC),
    OUTPUT(0x83, Ack),
    OUTPUT(0x84, Nack),
    OUTPUT(0x86, Pos_update_rate),
    OUTPUT(0x93, NMEA_talker_id),
    OUTPUT(0xdc, Measurement_time),
    OUTPUT(0xdd, Raw_measurements),
    OUTPUT(0xde, SV_channel_status),
  };


  unsigned char *parse_buffer = NULL;
  std::streamsize parse_length = 0;
  std::vector<Output_message::ptr> parse_messages(unsigned char* buffer, std::streamsize len) {
    parse_buffer = (unsigned char*)realloc(parse_buffer, parse_length + len);
    memcpy(parse_buffer + parse_length, buffer, len);
    parse_length += len;

    std::vector<Output_message::ptr> messages;

    std::streamsize start = -1;
    for (std::streamsize i = 0; i < parse_length - 1; i++)
      if ((parse_buffer[i] == 0xa0)
	  && (parse_buffer[i + 1] == 0xa1)) {
	start = i;
	break;
      }

    if (start > -1) {
      do {
	Payload_length payload_len = extract_be<uint16_t>(parse_buffer, start + 2);

	std::streamsize end = start + 2 + 2 + payload_len + 1 + 2;
	if (parse_length >= end) {
	  if ((parse_buffer[end - 2] != 0x0d)
	      || (parse_buffer[end - 1] != 0x0a)) {
	    std::cerr << "Last two bytes: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)parse_buffer[end - 2] << ", 0x" << (int)parse_buffer[end - 1] << std::dec << std::endl;
	    std::cerr << "Removing " << 2 << " bytes from start of parse buffer, now " << (parse_length - 2) << " bytes." << std::endl;;
	    memmove(parse_buffer, parse_buffer + 2, parse_length - 2);
	    parse_length -= 2;
	    start = 0;
	    parse_buffer = (unsigned char*)realloc(parse_buffer, parse_length);

	    throw InvalidMessage();
	  }

	  uint8_t cs = parse_buffer[end - 3];
	  unsigned char *payload = parse_buffer + start + 2 + 2;
	  {
	    uint8_t ccs = checksum(payload, payload_len);
	    if (cs != ccs)
	      throw ChecksumMismatch(ccs, cs);
	  }

	  uint8_t id = payload[0];

	  if (output_message_factories.count(id) > 0)
	    messages.push_back((*output_message_factories[id])(payload, payload_len));
	  else
	    std::cerr << "Unknown message id 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)id << std::dec << std::endl;

	  // Remove this packet from the parse buffer
	  if (parse_length == end) {
	    free(parse_buffer);
	    parse_buffer = NULL;
	    parse_length = 0;
	  } else {
	    memmove(parse_buffer, parse_buffer + end, parse_length - end);
	    parse_length -= end;
	    start = 0;
	    parse_buffer = (unsigned char*)realloc(parse_buffer, parse_length);
	  }
	} else {
	  if (start > 0) {
	    // parse buffer is not yet large enough for whole message, remove preceding bytes
	    memmove(parse_buffer, parse_buffer + start, parse_length - start);
	    parse_length -= start;
	    start = 0;
	    parse_buffer = (unsigned char*)realloc(parse_buffer, parse_length);
	  }
	  break;
	}
      } while ((parse_buffer != nullptr) && (parse_length > 7) && (parse_buffer[start] == 0xa0) && (parse_buffer[start + 1] == 0xa1));
    } else {
      // No start sequence found
      free(parse_buffer);
      parse_buffer = NULL;
      parse_length = 0;
    }

    return messages;
  }


}; // namespace SkyTraqBin
