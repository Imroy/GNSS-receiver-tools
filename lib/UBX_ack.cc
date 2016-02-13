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

  namespace Ack {

    Nak::Nak(unsigned char* payload, Length payload_len) :
      Output_message(payload, payload_len),
      _msg_cls(payload[0]),
      _msg_id(payload[1])
    {}


    Ack::Ack(unsigned char* payload, Length payload_len) :
      Output_message(payload, payload_len),
      _msg_cls(payload[0]),
      _msg_id(payload[1])
    {}


  }; // namespace Ack

}; // namespace UBX
