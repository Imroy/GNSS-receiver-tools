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

#include <cstdio>
#include <memory>
#include <sstream>
#include <vector>
#include <math.h>
#include "GNSS.hh"

/*
  Sources:
*/

/* All input message class names shall start with a verb
   Common words shortened:
   configure => cfg
   download => dl
   image => img
   message => msg
   navigation => nav
   position => pos
   query => q
   software => sw
   system => sys
   version => ver
*/

namespace UBX {

  //! Type for the binary message length, limited to 64 KiB
  typedef uint16_t Length;

  const uint8_t SyncChar[2] = { 0xB5, 0x62 };
  const Length SyncChar_len = 2;
  const Length Length_len = 2;
  const Length ClassID_len = 2;
  const Length Checksum_len = 2;


  enum class GNSS_ID : uint8_t {
    GPS = 0,
      SBAS,
      Galileo,
      BeiDou,
      IMES,
      QZSS,
      GLONASS,
  }; // enum class GNSS_ID

  enum class Class_ID : uint8_t {
    None = 0,
      NAV = 1,
      RXM = 2,
      INF = 4,
      ACK = 5,
      CFG = 6,
      UPD = 9,
      MON = 10,
      AID = 11,
      TIM = 13,
      MGA = 19,
      LOG = 33,
  }; // enum class Class_ID

  enum class PortID : uint8_t {
    DDC = 0,
      I2C = 0,
      UART = 1,
      USB = 3,
      SPI = 4,
  }; // enum class PortID


  //! Exception class for when checksums don't match
  class ChecksumMismatch : public std::exception {
  private:
    uint16_t _computed_cs, _stream_cs;

  public:
    ChecksumMismatch(uint8_t ccs, uint8_t scs) :
      _computed_cs(ccs), _stream_cs(scs)
    {}

    const char* what() const throw() {
      std::ostringstream oss;
      oss.width(2);
      oss << "Checksum mismatch in SkyTraq binary stream - computed 0x" << std::hex << _computed_cs << ", found 0x" << _stream_cs;
      return oss.str().c_str();
    }
  }; // class ChecksumMismatch


  //! Exception class for when we can't find the message ID in our list
  class UnknownMessageID : public std::exception {
  private:
    uint8_t _cls, _id;

  public:
    UnknownMessageID(uint8_t cls, uint8_t id) :
      _cls(cls), _id(id)
    {}

    const char* what() const throw() {
      std::ostringstream oss;
      oss.width(2);
      oss.fill('0');
      oss << "Unknown message class 0x" << std::hex << (int)_cls << " / id 0x" << std::hex << (int)_id;
      return oss.str().c_str();
    }
  }; // class UnknownMessageID


  //! Base class for a binary message
  class Message : public GNSS::Message {
  protected:
    Class_ID _cls;
    uint8_t _id;

    //! Constructor
    /*!
      \param cls Class for this message
      \param id ID for this message
    */
    Message(Class_ID cls, uint8_t id) :
      _cls(cls), _id(id)
    {}

  public:
    //! Getter method for the message class
    inline const Class_ID cls(void) const { return _cls; }

    //! Getter method for the message ID
    inline const uint8_t id(void) const { return _id; }

  }; // class Message


  //! Base class for messages that come from the GPS receiver
  class Output_message : public Message {
  protected:

  public:
    //! Constructor from a binary buffer
    /*!
      \param payload Pointer to start of payload
      \param len Length of payload
     */
    inline Output_message(unsigned char* payload, Length len) :
      Message(len > 0 ? (Class_ID)payload[0] : Class_ID::None, len > 1 ? payload[1] : 0)
    {}

    typedef std::shared_ptr<Output_message> ptr;
  }; // class Output_message


  //! Parser
  Output_message::ptr parse_message(unsigned char* buffer, std::size_t len);


  //! Base class for messages that go to the GPS receiver
  class Input_message : public Message {
  protected:
    //! The length of the payload (not including message class/ID)
    virtual inline const Length payload_length(void) const { return 0; }

    //! Write body fields into a pre-allocated buffer
    virtual inline void body_to_buf(unsigned char* buffer) const { }

    //! Constructor from a message class and ID
    Input_message(Class_ID cls, uint8_t id) :
      Message(cls, id)
    {}

  public:
    //! The total length of the message
    inline const Length message_length(void) const { return SyncChar_len + ClassID_len + Length_len + payload_length() + Checksum_len; }

    //! Write the message into a buffer
    /*!
      \param buffer Pointer to a buffer, allocated and freed by the caller. Use
      message_length() to know how big the buffer needs to be.
     */
    virtual void to_buf(unsigned char *buffer) const;

    typedef std::shared_ptr<Input_message> ptr;
  }; // class Input_message

#define GETTER(type, name, field) inline const type name(void) const { return field; }

#define GETTER_SETTER(type, name, field) inline const type name(void) const { return field; } \
inline void set_##name(type val) { field = val; }

#define SETTER_BOOL(name, field) inline void set_##name(bool val=true) { field = val; } \
inline void unset_##name(void) { field = false; }

#define GETTER_RAW(type, name, field) inline const type name##_raw(void) const { return field; }

#define GETTER_SETTER_RAW(type, name, field) inline const type name##_raw(void) const { return field; } \
inline void set_##name##_raw(type val) { field = val; }

#define GETTER_MOD(type, name, code) inline const type name(void) const { return code; }

#define GETTER_SETTER_MOD(type, name, field, code_get, code_set) inline const type name(void) const { return code_get; } \
inline void set_##name(type val) { field = code_set; }


}; // namepace UBX

#include "UBX_ack.hh"
#include "UBX_cfg.hh"

// Undefine our macros here, unless Doxygen is reading this
#ifndef DOXYGEN_SKIP_FOR_USERS
#undef ENUM__OSTREAM_OPERATOR
#undef GETTER
#undef GETTER_SETTER
#undef SETTER_BOOL
#undef GETTER_RAW
#undef GETTER_SETTER_RAW
#undef GETTER_MOD
#undef GETTER_SETTER_MOD

#endif
