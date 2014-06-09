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
#ifndef __SKYTRAQBIN_HH__
#define __SKYTRAQBIN_HH__

#include <cstdio>
#include <memory>
#include <sstream>
#include <vector>
#include <math.h>
#include "SkyTraq.hh"

namespace SkyTraqBin {

  typedef uint16_t Payload_length;

  class InsufficientData : public std::exception {
  private:

  public:
    InsufficientData() {}

    const char* what() const throw() {
      return "InsufficientData";
    }
  }; // class InsufficientData


  class InvalidMessage : public std::exception {
  private:

  public:
    InvalidMessage() {}

    const char* what() const throw() {
      return "Invalid message";
    }
  }; // class InvalidMessage


  //! Exception class for when checksums don't match
  class ChecksumMismatch : public std::exception {
  private:
    uint8_t _computed_cs, _stream_cs;

  public:
    ChecksumMismatch(uint8_t ccs, uint8_t scs) :
      _computed_cs(ccs), _stream_cs(scs)
    {}

    const char* what() const throw() {
      std::ostringstream oss;
      oss.width(2);
      oss << "Checksum mismatch in SkyTraq binary stream - computed 0x" << std::hex << (int)_computed_cs << ", found 0x" << (int)_stream_cs;
      return oss.str().c_str();
    }
  }; // class ChecksumMismatch


  class UnknownMessageID : public std::exception {
  private:
    uint8_t _id;

  public:
    UnknownMessageID(uint8_t i) :
      _id(i)
    {}

    const char* what() const throw() {
      std::ostringstream oss;
      oss.width(2);
      oss.fill('0');
      oss << "Unknown message id 0x" << std::hex << (int)_id;
      return oss.str().c_str();
    }
  }; // class UnknownMessageID


  //! Base class for a binary message
  class Message : public SkyTraq::Message {
  protected:
    uint8_t _msg_id;

  public:
    //! Constructor
    Message(uint8_t id) :
      _msg_id(id)
    {}

    inline const uint8_t message_id(void) const { return _msg_id; }

  }; // class Message


  //! Base class for messages that come from the GPS receiver
  class Output_message : public Message {
  protected:

  public:
    //! Constructor from a binary buffer
    inline Output_message(unsigned char* payload, Payload_length payload_len) :
      Message(payload_len > 0 ? payload[0] : 0)
    {}

    typedef std::shared_ptr<Output_message> ptr;
  }; // class Output_message


  //! Parser
  Output_message::ptr parse_message(unsigned char* buffer, std::size_t len);


  //! Base class for messages that go to the GPS receiver
  class Input_message : public Message {
  protected:
    //! The length of the body (not including message id)
    virtual inline const Payload_length body_length(void) const { return 0; }

    //! Write body fields into a pre-allocated buffer
    virtual inline void body_to_buf(unsigned char* buffer) const { }

  public:
    //! Constructor
    Input_message(uint8_t id) :
      Message(id)
    {}

    //! The total length of the message
    /*! This includes:
      - start sequence (2)
      - payload length (2)
      - message ID (1)
      - body length
      - checksum (1)
      - end sequence (2)
    */
    inline const Payload_length message_length(void) const { return 2 + 2 + 1 + body_length() + 1 + 2; }

    //! Write the message into a buffer
    /*!
      Use message_length() to know how big the buffer needs to be.
     */
    virtual void to_buf(unsigned char *buffer) const;

    typedef std::shared_ptr<Input_message> ptr;
  }; // class Input_message


  //! Role base class for adding a message sub-ID to message classes
  class with_subid {
  protected:
    uint8_t _msg_subid;

  public:
    //! Constructor
    with_subid(uint8_t subid) :
      _msg_subid(subid)
    {}

    inline const uint8_t message_subid(void) const { return _msg_subid; }
  }; // class with_subid


  //! Base class for messages that come from the GPS receiver with a sub-ID
  class Output_message_with_subid : public Output_message, public with_subid {
  protected:

  public:
    //! Constructor from a binary buffer
    inline Output_message_with_subid(unsigned char* payload, Payload_length payload_len) :
      Output_message(payload, payload_len),
      with_subid(payload_len > 1 ? payload[1] : 0)
    {}

    typedef std::shared_ptr<Output_message_with_subid> ptr;
  }; // class Output_message_with_subid


  //! Base class for messages that go to the GPS receiver with a sub-ID
  class Input_message_with_subid : public Input_message, public with_subid {
  protected:
    //! The length of the body (not including message id or sub-id)
    virtual const Payload_length body_length(void) const = 0;

    //! Write body fields into a pre-allocated buffer
    virtual void body_to_buf(unsigned char* buffer) const = 0;

  public:
    //! Constructor
    Input_message_with_subid(uint8_t id, uint8_t subid) :
      Input_message(id),
      with_subid(subid)
    {}

    //! The total length of the message
    /*! This includes:
      - start sequence (2)
      - payload length (2)
      - message ID (1)
      - message sub-ID (1)
      - body length
      - checksum (1)
      - end sequence (2)
    */
    inline const Payload_length message_length(void) const { return 2 + 2 + 1 + 1 + body_length() + 1 + 2; }

    //! Write the message into a buffer
    /*!
      Use message_length() to know how big the buffer needs to be.
     */
    virtual void to_buf(unsigned char *buffer) const;

    typedef std::shared_ptr<Input_message_with_subid> ptr;
  }; // class Input_message_with_subid


  enum class SwType : uint8_t {
    SystemCode = 1,
  }; // class SwType


  enum class TalkerID : uint8_t {
    GP = 0,
      GN,
  }; // class TalkerID


  enum class PowerMode : uint8_t {
    Normal = 0,
      PowerSave,
  }; // class PowerMode


  enum class DOPmode : uint8_t {
    Disable = 0,
      Auto,
      PDOP_only,
      HDOP_only,
      GDOP_only,
  }; // class DOPmode


  enum class ElevationCNRmode : uint8_t {
    Disable = 0,
      ElevationCNR,
      Elevation_only,
      CNR_only,
  }; // class ElevationCNRmode


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


}; // SkyTraqBin

#include "SkyTraqBin_inputs.hh"
#include "SkyTraqBin_outputs.hh"

#endif // __SKYTRAQBIN_HH__
