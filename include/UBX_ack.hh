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
#pragma once

#include "boost/date_time/posix_time/posix_time_types.hpp"
#include <string.h>
#include "UBX.hh"

namespace greg = boost::gregorian;
namespace ptime = boost::posix_time;

/*
  Sources:
*/

namespace UBX {

  namespace Ack {

    class Nak : public Output_message {
    private:
      uint8_t _msg_cls, _msg_id;

    public:
      //! Constructor from a binary buffer
      Nak(unsigned char* payload, Length payload_len);

      GETTER(uint8_t, message_class, _msg_cls);
      GETTER(uint8_t, message_id, _msg_id);
    }; // class Nak


    class Ack : public Output_message {
    private:
      uint8_t _msg_cls, _msg_id;

    public:
      //! Constructor from a binary buffer
      Ack(unsigned char* payload, Length payload_len);

      GETTER(uint8_t, message_class, _msg_cls);
      GETTER(uint8_t, message_id, _msg_id);
    }; // class Ack


  }; // namespace Ack

}; // namespace UBX
