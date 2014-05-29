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



  template <typename T>
  T read_be(unsigned char* buffer, Payload_length offset);

  template <>
  int8_t read_be<int8_t>(unsigned char* buffer, Payload_length offset) {
    return (int8_t)buffer[offset];
  }

  template <>
  uint8_t read_be<uint8_t>(unsigned char* buffer, Payload_length offset) {
    return buffer[offset];
  }

  template <>
  int16_t read_be<int16_t>(unsigned char* buffer, Payload_length offset) {
    return ((int16_t)buffer[offset] << 8) | buffer[offset + 1];
  }

  template <>
  uint16_t read_be<uint16_t>(unsigned char* buffer, Payload_length offset) {
    return ((uint16_t)buffer[offset] << 8) | buffer[offset + 1];
  }

  template <>
  int32_t read_be<int32_t>(unsigned char* buffer, Payload_length offset) {
    return ((int32_t)(buffer[offset] << 24)
	    | ((int32_t)buffer[offset + 1] << 16)
	    | ((int32_t)buffer[offset + 2] << 8)
	    | (int32_t)buffer[offset + 3]);
  }

  template <>
  uint32_t read_be<uint32_t>(unsigned char* buffer, Payload_length offset) {
    return ((uint32_t)buffer[offset] << 24)
      | ((uint32_t)buffer[offset + 1] << 16)
      | ((uint32_t)buffer[offset + 2] << 8)
      | (uint32_t)buffer[offset + 3];
  }

  template <>
  float read_be<float>(unsigned char* buffer, Payload_length offset) {
    float val;
    unsigned char *mem = (unsigned char*)&val;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    mem[3] = buffer[offset];
    mem[2] = buffer[offset + 1];
    mem[1] = buffer[offset + 2];
    mem[0] = buffer[offset + 3];
#elif __BYTE_ORDER == __BIG_ENDIAN
    mem[0] = buffer[offset];
    mem[1] = buffer[offset + 1];
    mem[2] = buffer[offset + 2];
    mem[3] = buffer[offset + 3];
#endif
    return val;
  }

  template <>
  double read_be<double>(unsigned char* buffer, Payload_length offset) {
    double val;
    unsigned char *mem = (unsigned char*)&val;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    mem[7] = buffer[offset];
    mem[6] = buffer[offset + 1];
    mem[5] = buffer[offset + 2];
    mem[4] = buffer[offset + 3];
    mem[3] = buffer[offset + 4];
    mem[2] = buffer[offset + 5];
    mem[1] = buffer[offset + 6];
    mem[0] = buffer[offset + 7];
#elif __BYTE_ORDER == __BIG_ENDIAN
    mem[0] = buffer[offset];
    mem[1] = buffer[offset + 1];
    mem[2] = buffer[offset + 2];
    mem[3] = buffer[offset + 3];
    mem[4] = buffer[offset + 4];
    mem[5] = buffer[offset + 5];
    mem[6] = buffer[offset + 6];
    mem[7] = buffer[offset + 7];
#endif
    return val;
  }


  typedef Output_message::ptr (*output_message_factory)(unsigned char* payload, Payload_length payload_len);
#define OUTPUT(ID, CLASS) std::make_pair<uint8_t, output_message_factory>(ID, [](unsigned char* payload, Payload_length len) -> Output_message::ptr { return std::make_shared<CLASS>(payload, len); })

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
	Payload_length payload_len = read_be<uint16_t>(parse_buffer, start + 2);

	std::streamsize end = start + 2 + 2 + payload_len + 1 + 2;
	if (parse_length >= end) {
	  if ((parse_buffer[end - 2] != 0x0d)
	      || (parse_buffer[end - 1] != 0x0a)) {
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
	    std::cerr << "Freeing parse buffer..." << std::endl;
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


  Sw_ver::Sw_ver(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _sw_type((SwType)payload[1]),
    _kernel_ver(payload[3], payload[4], payload[5]),
    _odm_ver(payload[7], payload[8], payload[9]),
    _revision(payload[11], payload[12], payload[13])
  {}


  Sw_CRC::Sw_CRC(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _sw_type((SwType)payload[1]),
    _crc(read_be<uint16_t>(payload, 2))
  {}


  Ack::Ack(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _ack_id(payload[1]),
    _has_subid(payload_len > 2),
    _ack_subid(payload_len > 2 ? payload[2] : 0)
  {}


  Nack::Nack(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _ack_id(payload[1]),
    _has_subid(payload_len > 2),
    _ack_subid(payload_len > 2 ? payload[2] : 0)
  {}


  Pos_update_rate::Pos_update_rate(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _update_rate(payload[1])
  {}


  NMEA_talker_id::NMEA_talker_id(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _talker_id((TalkerID)payload[1])
  {}



  Measurement_time::Measurement_time(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _issue(payload[1]),
    _weeknumber(read_be<uint16_t>(payload, 2)),
    _time_in_week(read_be<uint32_t>(payload, 4)),
    _period(read_be<uint16_t>(payload, 8))
  {}



  Raw_measurements::Raw_measurements(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _issue(payload[1]),
    _num_meas(payload[2])
  {
    for (int i = 3; i <= payload_len - 23; i += 23)
      _measurements.push_back(RawMeasurement(payload[i],
					     payload[i+1],
					     read_be<double>(payload, i + 2),
					     read_be<double>(payload, i + 10),
					     read_be<float>(payload, i + 18),
					     payload[i + 22]));
  }


  SV_channel_status::SV_channel_status(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _issue(payload[1]),
    _num_sv(payload[2])
  {
    for (int i = 3; i <= payload_len - 10; i += 10)
      _statuses.push_back(SvStatus(payload[i],
				   payload[i+1],
				   payload[i+2],
				   payload[i+3],
				   payload[i+4],
				   read_be<int16_t>(payload, i + 5),
				   read_be<int16_t>(payload, i + 7),
				   payload[i+9]));
  }





}; // namespace SkyTraqBin
