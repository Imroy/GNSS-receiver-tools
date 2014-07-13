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
#include <thread>
#include <sys/stat.h>
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
  {
    struct stat st;
    fstat(fileno(_file), &st);
    _is_chrdev = S_ISCHR(st.st_mode);
  }

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

    if (len == 0) {
      if (_is_chrdev) {
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	return;
      } else {
	throw EndOfFile();
      }
    }

    _parser.add_bytes(buffer, len);

    auto messages = _parser.parse_messages();

    for (auto msg : messages) {

#define FIRE_IF(class, method) if (msg->isa<class>()) \
	_listener->method(this, *(msg->cast_as<class>()));

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

	  bool response_type = false;
	  uint16_t rid;
	  try {
	    auto *wr = msg->cast_as<SkyTraqBin::with_response>();
	    rid = wr->response_id();
	    if (wr->has_response_subid())
	      rid = (rid << 8) | wr->response_subid();
	    response_type = true;
	  } catch (std::bad_cast) {
	  }

	  if (_response_handlers.count(id) > 0) {
	    // Call the response handler with a null message
	    _response_handlers[id](true, nullptr);

	    // If there is a response message type, move the handler to its id
	    if (response_type)
	      _response_handlers[rid] = _response_handlers[id];

	    _response_handlers.erase(id);
	  }

	  // If there is no additional "response" message incoming, it's okay to send another message
	  if (!response_type)
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
	else FIRE_IF(SkyTraqBin::Rcv_state, Rcv_state)
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
    if (!is_sendable())
      throw NotSendable();

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
