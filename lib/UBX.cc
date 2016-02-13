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
#include <functional>
#include <iostream>
#include <iomanip>
#include <map>
#include <stdexcept>
#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include "UBX.hh"
#include "LE.hh"

/*
  Sources:
*/

namespace UBX {

  struct checksum_t {
    uint8_t A, B;
    operator uint16_t() const { return A | ((uint16_t)B << 8); }
  };

  checksum_t checksum(unsigned char* buffer, Length len) {
    checksum_t ck = { 0, 0};

    for (uint16_t i = 0; i < len; i++) {
      uint8_t val = *buffer++;
      ck.A += val;
      ck.B += ck.A;
    }

    return ck;
  }

  void Input_message::to_buf(unsigned char* buffer) const {
    append_le(buffer, SyncChar[0]);
    append_le(buffer, SyncChar[1]);

    append_le(buffer, (uint8_t)_cls);
    append_le(buffer, _id);

    Length payload_len = payload_length();
    append_le(buffer, payload_len);

    unsigned char *payload = buffer;
    body_to_buf(buffer);
    buffer = payload + payload_len;

    checksum_t cs = checksum(payload, payload_len);
    append_le(buffer, cs.A);
    append_le(buffer, cs.B);
  }


 typedef std::function<Output_message::ptr(uint8_t*, Length)> output_message_factory;
#define OUTPUT(CLS, ID, CLASS) std::make_pair<uint16_t, output_message_factory>(((CLS) << 8) | (ID), [](uint8_t* payload, Length len) { return std::make_shared<CLASS>(payload, len); })
  std::map<uint16_t, output_message_factory> output_message_factories = {
    OUTPUT(0x05, 0x00, Ack::Nak),
    OUTPUT(0x05, 0x01, Ack::Ack),
  };

#undef OUTPUT

  Output_message::ptr parse_message(unsigned char* buffer, std::size_t len) {
    Length payload_len = extract_le<uint16_t>(buffer, SyncChar_len + ClassID_len);
    std::size_t end = SyncChar_len + ClassID_len + Length_len + payload_len + Checksum_len;
    if (len < end)
      throw GNSS::InsufficientData();

    if ((buffer[0] != SyncChar[0])
	|| (buffer[1] != SyncChar[1]))
      throw GNSS::InvalidMessage();

    checksum_t cs = { buffer[end - 1], buffer[end] };
    {
      checksum_t ccs = checksum(buffer + SyncChar_len, ClassID_len + payload_len);
      if ((cs.A != ccs.A) && (cs.B != ccs.B))
	throw ChecksumMismatch(ccs, cs);
    }

    uint8_t cls = buffer[SyncChar_len], id = buffer[SyncChar_len + 1];
    uint16_t clsid = ((uint16_t)cls << 8) | id;
    if (output_message_factories.count(clsid) == 0)
      throw UnknownMessageID(cls, id);

    unsigned char *payload = buffer + SyncChar_len + ClassID_len + Length_len;

    return output_message_factories[clsid](payload, payload_len);
  }

}; // namespace UBX
