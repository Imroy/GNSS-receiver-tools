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
#include <endian.h>
#include "SkyTraqBin.hh"

namespace SkyTraqBin {

  template <typename T>
  void add_to_buf(unsigned char* &buffer, T val);

  template <>
  void add_to_buf<int8_t>(unsigned char* &buffer, int8_t val) {
    buffer[0] = val;
    buffer++;
  }

  template <>
  void add_to_buf<uint8_t>(unsigned char* &buffer, uint8_t val) {
    buffer[0] = val;
    buffer++;
  }

  template <>
  void add_to_buf<int16_t>(unsigned char* &buffer, int16_t val) {
    buffer[0] = val >> 8;
    buffer[1] = val & 0xff;
    buffer += 2;
  }

  template <>
  void add_to_buf<uint16_t>(unsigned char* &buffer, uint16_t val) {
    buffer[0] = val >> 8;
    buffer[1] = val & 0xff;
    buffer += 2;
  }

  template <>
  void add_to_buf<int32_t>(unsigned char* &buffer, int32_t val) {
    buffer[0] = val >> 24;
    buffer[1] = (val >> 16) & 0xff;
    buffer[2] = (val >> 8) & 0xff;
    buffer[2] = val & 0xff;
    buffer += 4;
  }

  template <>
  void add_to_buf<uint32_t>(unsigned char* &buffer, uint32_t val) {
    buffer[0] = val >> 24;
    buffer[1] = (val >> 16) & 0xff;
    buffer[2] = (val >> 8) & 0xff;
    buffer[2] = val & 0xff;
    buffer += 4;
  }

  template <>
  void add_to_buf<float>(unsigned char* &buffer, float val) {
    unsigned char *mem = (unsigned char*)&val;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    buffer[0] = mem[3];
    buffer[1] = mem[2];
    buffer[2] = mem[1];
    buffer[3] = mem[0];
#elif __BYTE_ORDER == __BIG_ENDIAN
    buffer[0] = mem[0];
    buffer[1] = mem[1];
    buffer[2] = mem[2];
    buffer[3] = mem[3];
#endif
    buffer += 4;
  }

  template <>
  void add_to_buf<double>(unsigned char* &buffer, double val) {
    unsigned char *mem = (unsigned char*)&val;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    buffer[0] = mem[7];
    buffer[1] = mem[6];
    buffer[2] = mem[5];
    buffer[3] = mem[4];
    buffer[4] = mem[3];
    buffer[5] = mem[2];
    buffer[6] = mem[1];
    buffer[7] = mem[0];
#elif __BYTE_ORDER == __BIG_ENDIAN
    buffer[0] = mem[0];
    buffer[1] = mem[1];
    buffer[2] = mem[2];
    buffer[3] = mem[3];
    buffer[4] = mem[4];
    buffer[5] = mem[5];
    buffer[6] = mem[6];
    buffer[7] = mem[7];
#endif
    buffer += 8;
  }

  unsigned char checksum(unsigned char* buffer, Payload_length len) {
    unsigned char cs = 0;
    while (len) {
      cs ^= *buffer;
      buffer++;
      len--;
    }

    return cs;
  }

  void Input_message::to_buf(unsigned char* buffer) const {
    add_to_buf<unsigned char>(buffer, 0xa0);
    add_to_buf<unsigned char>(buffer, 0xa1);

    Payload_length payload_len = body_length() + 1;
    add_to_buf(buffer, payload_len);

    unsigned char *payload = buffer;
    add_to_buf(buffer, _msg_id);
    body_to_buf(buffer);

    add_to_buf(buffer, checksum(payload, payload_len));
    add_to_buf<unsigned char>(buffer, 0x0d);
    add_to_buf<unsigned char>(buffer, 0x0a);
  }


  void Restart_sys::body_to_buf(unsigned char* buffer) const {
    add_to_buf(buffer, (uint8_t)_start_mode);
    add_to_buf(buffer, _utc_year);
    add_to_buf(buffer, _utc_month);
    add_to_buf(buffer, _utc_day);
    add_to_buf(buffer, _utc_hour);
    add_to_buf(buffer, _utc_minute);
    add_to_buf(buffer, _utc_second);
    add_to_buf(buffer, _lattitude);
    add_to_buf(buffer, _longitude);
    add_to_buf(buffer, _altitude);
  }


  void Q_sw_ver::body_to_buf(unsigned char* buffer) const {
    add_to_buf(buffer, (uint8_t)_sw_type);
  }


  void Q_sw_CRC::body_to_buf(unsigned char* buffer) const {
    add_to_buf(buffer, (uint8_t)_sw_type);
  }


  void Set_factory_defaults::body_to_buf(unsigned char* buffer) const {
    add_to_buf(buffer, (uint8_t)_reset);
  }



  typedef Output_message::ptr (*output_message_factory)(unsigned char* payload, Payload_length payload_len);
#define OUTPUT(CLASS) [](unsigned char* payload, Payload_length len) -> Output_message::ptr { return std::make_shared<CLASS>(payload, payload_len); }
  output_message_factory output_message_factories[] = {
  };

}; // namespace SkyTraqBin
