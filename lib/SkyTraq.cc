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
#include "SkyTraq.hh"
#include "NMEA-0183.hh"
#include "SkyTraqBin.hh"

namespace SkyTraq {

  unsigned char *parse_buffer = NULL;
  std::streamsize parse_length = 0;
  std::vector<Message::ptr> parse_messages(unsigned char* buffer, std::streamsize len) {
    parse_buffer = (unsigned char*)realloc(parse_buffer, parse_length + len);
    memcpy(parse_buffer + parse_length, buffer, len);
    parse_length += len;

    std::vector<Message::ptr> messages;

    std::streamsize end = -1;	// actually the start of the next message

    do {
      for (std::streamsize i = 0; i < parse_length - 1; i++)
	if ((parse_buffer[i] == 0x0d)
	    && (parse_buffer[i + 1] == 0x0a)) {
	  end = i + 2;
	  break;
	}
      if (end == -1)
	break;

      try {
	if (parse_buffer[0] == '$')
	  messages.push_back(NMEA0183::parse_sentence(std::string((const char*)parse_buffer, end - 2)));
	else if ((parse_buffer[0] == 0xa0) && (parse_buffer[1] == 0xa1))
	  messages.push_back(SkyTraqBin::parse_message(parse_buffer, end));

	// Catch the harmless exceptions
      } catch (const NMEA0183::InvalidSentence &e) {
      } catch (const SkyTraqBin::InvalidMessage &e) {
      } catch (const SkyTraqBin::InsufficientData &e) {
      }

      // Remove this packet from the parse buffer
      if (parse_length < end)
	memmove(parse_buffer, parse_buffer + end, parse_length - end);
      parse_length -= end;
      parse_buffer = (unsigned char*)realloc(parse_buffer, parse_length);
    } while (parse_length > 0);

    return messages;
  }


}; // namespace SkyTraq
