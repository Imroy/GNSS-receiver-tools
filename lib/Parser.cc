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
#include <string.h>
#include "Parser.hh"

namespace SkyTraq {

  Parser::Parser() :
    _parse_buffer(nullptr),
    _parse_buflen(0)
  {}

  Parser::~Parser() {
    if (_parse_buflen > 0)
      free(_parse_buffer);
  }

  void Parser::reset_buffer(void) {
    if (_parse_buflen > 0) {
      free(_parse_buffer);
      _parse_buffer = nullptr;
      _parse_buflen = 0;
    }
  }

  void Parser::add_bytes(unsigned char* buffer, std::size_t buffer_len) {
    _parse_buffer = (unsigned char*)realloc(_parse_buffer, _parse_buflen + buffer_len);
    memcpy(_parse_buffer + _parse_buflen, buffer, buffer_len);
    _parse_buflen += buffer_len;
  }

  std::vector<Message::ptr> Parser::parse_messages(void) {
    std::vector<Message::ptr> messages;

    std::size_t end;	// actually the start of the next message
    do {
      end = 0;
      for (std::size_t i = 0; i < _parse_buflen - 1; i++)
	if ((_parse_buffer[i] == 0x0d)
	    && (_parse_buffer[i + 1] == 0x0a)) {
	  end = i + 2;
	  break;
	}
      if (end == 0)
	break;

      std::exception_ptr exp = nullptr;
      try {
	if (_parse_buffer[0] == '$')
	  messages.push_back(NMEA0183::parse_sentence(std::string((const char*)_parse_buffer, end - 2)));
	else if ((_parse_buffer[0] == 0xa0) && (_parse_buffer[1] == 0xa1))
	  messages.push_back(SkyTraqBin::parse_message(_parse_buffer, end));

	// Catch the harmless exceptions
      } catch (const NMEA0183::InvalidSentence &e) {
      } catch (const SkyTraqBin::InvalidMessage &e) {
      } catch (const SkyTraqBin::InsufficientData &e) {

      } catch (const std::exception) {
	exp = std::current_exception();
      }

      // Remove this packet from the parse buffer
      if (_parse_buflen > end)
	memmove(_parse_buffer, _parse_buffer + end, _parse_buflen - end);
      _parse_buflen -= end;
      _parse_buffer = (unsigned char*)realloc(_parse_buffer, _parse_buflen);

      if (exp != nullptr)
	std::rethrow_exception(exp);
    } while (_parse_buflen > 0);

    return messages;
  }


  Interface::Interface(std::FILE* f, Listener::ptr l) :
    _file(f), _listener(l), _response_pending(false)
  {}

  // A table mapping the id of input messages to the id of their output message responses
#define RES1(a, b) std::make_pair<uint16_t, uint16_t>(a, b)
#define RES2(a1, a2, b1, b2) std::make_pair<uint16_t, uint16_t>(((a1) << 8) | a2, ((b1) << 8) | b2)
  std::map<uint16_t, uint16_t> response_ids = {
    RES1(0x02, 0x80),
    RES1(0x03, 0x81),
    RES1(0x10, 0x86),
    RES1(0x15, 0xB9),
    RES1(0x2D, 0xAE),
    RES1(0x2E, 0xAF),
    RES1(0x2F, 0xB0),
    RES1(0x30, 0xB1),
    RES1(0x3A, 0xB4),
    RES1(0x46, 0xBB),
    RES1(0x4F, 0x93),
    RES2(0x64, 0x01, 0x64, 0x80),
    RES2(0x64, 0x03, 0x64, 0x81),
    RES2(0x64, 0x07, 0x64, 0x83),
    RES2(0x64, 0x18, 0x64, 0x8B),
    RES2(0x64, 0x1A, 0x64, 0x8C),
    RES2(0x65, 0x02, 0x65, 0x80),
  };
#undef RES1
#undef RES2

  void Interface::_send_from_queue(void) {
    if (_output_queue.empty())
      return;

    auto msg = _output_queue.front();
    fwrite(msg.first, 1, msg.second, _file);
    fflush(_file);
    free(msg.first);

    _output_queue.pop();
    if (_output_queue.empty())
      _response_pending = false;
  }

  void Interface::read(void) {
    unsigned char buffer[16];
    std::size_t len = fread(buffer, 1, 16, _file);
    if (len == 0)
      return;
    _parser.add_bytes(buffer, len);

    auto messages = _parser.parse_messages();

    for (auto msg : messages) {

#define FIRE_IF(class, method) if (msg->isa<class>()) \
	_listener->method(*(msg->cast_as<class>()));

      try {
	msg->cast_as<NMEA0183::Sentence>();	// Will throw std::bad_cast if not possible

	FIRE_IF(NMEA0183::GGA, GGA)
	else FIRE_IF(NMEA0183::GLL, GLL)
	else FIRE_IF(NMEA0183::GSA, GSA)
	else FIRE_IF(NMEA0183::GSV, GSV)
	else FIRE_IF(NMEA0183::RMC, RMC)
	else FIRE_IF(NMEA0183::VTG, VTG)
	else FIRE_IF(NMEA0183::ZDA, ZDA)
	else FIRE_IF(NMEA0183::STI, STI);

      } catch (std::bad_cast) {
      }

      try {
	auto m = msg->cast_as<SkyTraqBin::Output_message>();	// Will throw a std::bad_cast exception if not possible

	if (msg->isa<SkyTraqBin::Ack>()) {
	  auto ack = msg->cast_as<SkyTraqBin::Ack>();
	  uint16_t id = ack->ack_id();
	  if (ack->has_subid())
	    id = (id << 8) | ack->ack_subid();

	  if (_response_handlers.count(id) > 0) {
	    // Call the response handler with a null message
	    _response_handlers[id](true, nullptr);

	    // If there is a response message type, move the handler to its id
	    if (response_ids.count(id) > 0)
	      _response_handlers[response_ids[id]] = _response_handlers[id];

	    _response_handlers.erase(id);
	  }

	  // If there is no additional "response" message incoming, it's okay to send another message
	  if (response_ids.count(id) == 0)
	    _send_from_queue();

	} else if (msg->isa<SkyTraqBin::Nack>()) {
	  // There's no additional "response" message, send another message
	  _send_from_queue();

	  auto nack = msg->cast_as<SkyTraqBin::Nack>();
	  uint16_t id = nack->nack_id();
	  if (nack->has_subid())
	    id = (id << 8) | nack->nack_subid();

	  if (_response_handlers.count(id) > 0) {
	    // Call the response handler with a null message
	    _response_handlers[id](false, nullptr);
	    _response_handlers.erase(id);
	  }

	} else FIRE_IF(SkyTraqBin::Measurement_time, Measurement_time)
	else FIRE_IF(SkyTraqBin::Raw_measurements, Raw_measurements)
	else FIRE_IF(SkyTraqBin::SV_channel_status, SV_channel_status)
	else FIRE_IF(SkyTraqBin::Subframe_data, Subframe_data)
	else {
	  // Assume this is a "response" ouput message, send another message from the queue
	  _send_from_queue();

	  uint16_t id = m->message_id();
	  try {
	    auto m_with_subid = m->cast_as<SkyTraqBin::with_subid>();
	    id = (id << 8) | m_with_subid->message_subid();
	  } catch (std::bad_cast) {
	  }

	  if (_response_handlers.count(id) > 0) {
	    // Call the response handler with the message
	    _response_handlers[id](true, m);
	    _response_handlers.erase(id);
	  }
	}

      } catch (std::bad_cast) {
      }

#undef FIRE_IF
    }
  }

  void Interface::send(SkyTraqBin::Input_message::ptr msg) {
    SkyTraqBin::Payload_length len = msg->message_length();

    unsigned char *buffer = (unsigned char*)malloc(len);
    msg->to_buf(buffer);

    if (!_response_pending) {
      fwrite(buffer, 1, len, _file);
      fflush(_file);
      free(buffer);
      _response_pending = true;
    } else
      _output_queue.push(std::make_pair(buffer, len));
  }

  void Interface::send(SkyTraqBin::Input_message::ptr msg, ResponseHandler rh) {
    send(msg);
    uint16_t id = msg->message_id();
    try {
      auto msg_with_subid = msg->cast_as<SkyTraqBin::with_subid>();
      id = (id << 8) | msg_with_subid->message_subid();
    } catch (std::bad_cast) {
    }
    _response_handlers[id] = rh;
  }


}; // namespace SkyTraq
