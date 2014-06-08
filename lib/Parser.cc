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
    _parse_buffer(NULL),
    _parse_buflen(0)
  {}

  Parser::~Parser() {
    if (_parse_buflen > 0)
      free(_parse_buffer);
  }

  void Parser::reset_buffer(void) {
    if (_parse_buflen > 0) {
      free(_parse_buffer);
      _parse_buffer = NULL;
      _parse_buflen = 0;
    }
  }

  void Parser::add_bytes(unsigned char* buffer, std::streamsize buffer_len) {
    _parse_buffer = (unsigned char*)realloc(_parse_buffer, _parse_buflen + buffer_len);
    memcpy(_parse_buffer + _parse_buflen, buffer, buffer_len);
    _parse_buflen += buffer_len;
  }

  std::vector<Message::ptr> Parser::parse_messages(void) {
    std::vector<Message::ptr> messages;

    std::streamsize end;	// actually the start of the next message
    do {
      end = -1;
      for (std::streamsize i = 0; i < _parse_buflen - 1; i++)
	if ((_parse_buffer[i] == 0x0d)
	    && (_parse_buffer[i + 1] == 0x0a)) {
	  end = i + 2;
	  break;
	}
      if (end == -1)
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


  Reader::Reader(std::istream &is, Listener::ptr l) :
    _is(&is), _listener(l)
  {}


  void Reader::read(void) {
    _is->read((char*)_buffer, 16);
    _parser.add_bytes(_buffer, _is->gcount());

    try {
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
	  msg->cast_as<SkyTraqBin::Output_message>();	// throw a std::bas_cast exception if it can't be done

	  FIRE_IF(SkyTraqBin::Measurement_time, Measurement_time)
	  else FIRE_IF(SkyTraqBin::Raw_measurements, Raw_measurements)
	  else FIRE_IF(SkyTraqBin::SV_channel_status, SV_channel_status)
	  else FIRE_IF(SkyTraqBin::Subframe_data, Subframe_data);

	} catch (std::bad_cast) {
	}

#undef FIRE_IF
      }
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
  }



}; // namespace SkyTraq