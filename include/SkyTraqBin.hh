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

#include <memory>
#include <sstream>
#include <vector>
#include <math.h>

namespace SkyTraqBin {

  typedef uint16_t Payload_length;

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


  //! Base class for a binary message
  class Message {
  protected:
    uint8_t _msg_id;

  public:
    //! Constructor
    Message(uint8_t id) :
      _msg_id(id)
    {}

    //! Virtual destructor to force polymorphism
    inline virtual ~Message() {}

    inline const uint8_t message_id(void) const { return _msg_id; }

    //! Check the type of an object
    template <typename T>
    inline bool isa(void) const { return typeid(*this) == typeid(T); }

    //! Recast this object to another type
    template <typename T>
    inline T* cast_as(void) { return dynamic_cast<T*>(this); }

    typedef std::shared_ptr<Message> ptr;
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
  std::vector<Output_message::ptr> parse_messages(unsigned char* buffer, std::streamsize len);


  //! Base class for messages that go to the GPS receiver
  class Input_message : public Message {
  protected:
    //! The length of the body (not including message id)
    virtual const Payload_length body_length(void) const = 0;

    //! Write body fields into a pre-allocated buffer
    virtual void body_to_buf(unsigned char* buffer) const = 0;

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


}; // SkyTraqBin

#include "SkyTraqBin_inputs.hh"
#include "SkyTraqBin_outputs.hh"

#endif // __SKYTRAQBIN_HH__
