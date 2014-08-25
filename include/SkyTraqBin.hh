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

/*
  Sources:
  https://store-lgdi92x.mybigcommerce.com/content/AN0028_1.4.31.pdf	(Binary messages of Skytraq Venus 8)
  https://store-lgdi92x.mybigcommerce.com/content/AN0024_v07.pdf	(Raw measurement binary messages of Skytraq 6 & 8)
  https://store-lgdi92x.mybigcommerce.com/content/SUP800F_v0.6.pdf	(Skytraq SUP800F datasheet)
*/

namespace SkyTraqBin {

  //! Type for the binary message payload length, limited to 64 KiB
  typedef uint16_t Payload_length;

  const Payload_length StartSeq_len = 2;
  const Payload_length PayloadLength_len = 2;
  const Payload_length MsgID_len = 1;
  const Payload_length MsgSubID_len = 1;
  const Payload_length Checksum_len = 1;
  const Payload_length EndSeq_len = 2;

  //! Exception signifying that the parser buffer isn't big enough for the message
  //! Completely harmless - read some more data into the buffer and try again
  class InsufficientData : public std::exception {
  private:

  public:
    InsufficientData() {}

    const char* what() const throw() {
      return "Insufficient data";
    }
  }; // class InsufficientData


  //! Exception signifying that the start and/or end sequence bytes of a message are wrong
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


  //! Exception class for when we can't find the message ID in our list
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
    /*!
      \param id ID for this message
    */
    Message(uint8_t id) :
      _msg_id(id)
    {}

    //! Getter method for the message ID
    inline const uint8_t message_id(void) const { return _msg_id; }

  }; // class Message


  //! Base class for messages that come from the GPS receiver
  class Output_message : public Message {
  protected:

  public:
    //! Constructor from a binary buffer
    /*!
      \param payload Pointer to start of payload
      \param payload_len Length of payload
     */
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
    //! The length of the body (not including message ID or sub-ID)
    virtual inline const Payload_length body_length(void) const { return 0; }

    //! Write body fields into a pre-allocated buffer
    virtual inline void body_to_buf(unsigned char* buffer) const { }

  public:
    //! Constructor from a message ID
    Input_message(uint8_t id) :
      Message(id)
    {}

    //! The total length of the message
    inline const Payload_length message_length(void) const { return StartSeq_len + PayloadLength_len + MsgID_len + body_length() + Checksum_len + EndSeq_len; }

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


  //! Role base class for adding a message sub-ID to message classes
  class with_subid {
  protected:
    uint8_t _msg_subid;

  public:
    //! Constructor
    /*!
      \param subid Message sub-ID
    */
    with_subid(uint8_t subid) :
      _msg_subid(subid)
    {}

    //! Getter method for message sub-ID
    GETTER(uint8_t, message_subid, _msg_subid);

  }; // class with_subid


  //! Role base class for input messages that have a response message
  class with_response {
  public:
    //! Return the message ID of the response message
    virtual uint8_t response_id(void) const = 0;

    //! Does the response message type have a sub-ID?
    virtual bool has_response_subid(void) const = 0;

    //! Return the message sub-ID of the response message
    virtual uint8_t response_subid(void) const = 0;

  }; // class with_response

#define RESPONSE1(id)  uint8_t response_id(void) const { return id; } \
    bool has_response_subid(void) const { return false; } \
    uint8_t response_subid(void) const { return 0; }

#define RESPONSE2(id, subid)  uint8_t response_id(void) const { return id; }	\
    bool has_response_subid(void) const { return true; } \
    uint8_t response_subid(void) const { return subid; }


  enum class StartMode : uint8_t {
    HotStart = 1,
      WarmStart,
      ColdStart,
  }; // class StartMode


  enum class SwType : uint8_t {
    SystemCode = 1,
  }; // class SwType


  enum class BaudRate : uint8_t {
    Baud4800 = 0,
    Baud9600,
    Baud19200,
    Baud38400,
    Baud57600,
    Baud115200,
    Baud230400,
    Baud460800,
    Baud921600,
  }; // class BaudRate

  //! Convert from an integer baud rate to a BaudRate enum
  BaudRate rate_to_BaudRate(unsigned int rate);

  //! Convert from a BaudRate enum to an integer value
  unsigned int BaudRate_rate(BaudRate br);


  enum class UpdateType : uint8_t {
    SRAM = 0,
      SRAM_and_flash,
      Temporary,
  }; // class UpdateType


  enum class MessageType : uint8_t {
    None = 0,
      NMEA0183,
      Binary,
  }; // class MessageType


  enum class FlashType : uint8_t {
    Auto = 0,
      QSPI_Winbond,
      QSPI_EON,
      Parallel_Numonyx,
      Parallel_EON,
  }; // class FlashType


  enum class BufferUsed : uint8_t {
    Size8K = 0,
      Size16K,
      Size24K,
      Size32K,
  }; // class BufferUsed


  enum class PowerMode : uint8_t {
    Normal = 0,
      PowerSave,
  }; // class PowerMode


  enum class OutputRate : uint8_t {
    Rate1Hz = 0,
      Rate2Hz,
      Rate4Hz,
      Rate5Hz,
      Rate10Hz,
      Rate20Hz,
  }; // class OutputRate

  //! Convert from an integer rate to an OutputRate enum
  OutputRate Hz_to_OutputRate(unsigned int hz);

  //! Convert from an OutputRate enum to an integer value
  unsigned int OutputRate_Hz(OutputRate r);


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


  enum class DefaultOrEnable : uint8_t {
    Default = 0,
      Enable,
      Disable,
  }; // class DefaultOrEnable


  enum class TalkerID : uint8_t {
    GP = 0,
      GN,
  }; // class TalkerID


  enum class EnableOrAuto : uint8_t {
    Disable = 0,
      Enable,
      Auto,
  }; // class EnableOrAuto


  enum class ParameterSearchEngineMode : uint8_t {
    Default = 0,
      Low,
      Mid,
      High,
      Full,
  }; // class ParameterSearchEngineMode


  enum class NavigationMode : uint8_t {
    Auto = 0,
      Pedestrian,
      Car,
      Marine,
      Balloon,
      Airborne,
  }; // class NavigationMode


  enum class BootStatus : uint8_t {
    FromFlash = 0,
      FromROM,
  }; // class BootStatus


  enum class InterferenceStatus : uint8_t {
    Unknown = 0,
      None,
      Little,
      Critical,
  }; // class InterferenceStatus


  enum class FixType : uint8_t {
    None = 0,
      TwoDimensional,
      ThreeDimensional,
      Differential,
  }; // class FixType


  enum class NavigationState : uint8_t {
    NoFix = 0,
      Predicted,
      TwoDimensional,
      ThreeDimensional,
      Differential,
  }; // class NavigationState


}; // SkyTraqBin

#include "SkyTraqBin_inputs.hh"
#include "SkyTraqBin_inputs_with_subid.hh"
#include "SkyTraqBin_outputs.hh"
#include "SkyTraqBin_outputs_with_subid.hh"

#define ENUM_OSTREAM_OPERATOR(type) inline std::ostream& operator<< (std::ostream& out, type val) { out << std::to_string(val); return out;  }

namespace std {
  std::string to_string(SkyTraqBin::StartMode mode);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::StartMode);

  std::string to_string(SkyTraqBin::SwType sw);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::SwType);

  std::string to_string(SkyTraqBin::BaudRate r);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::BaudRate);

  std::string to_string(SkyTraqBin::UpdateType ut);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::UpdateType);

  std::string to_string(SkyTraqBin::MessageType mt);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::MessageType);

  std::string to_string(SkyTraqBin::FlashType ft);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::FlashType);

  std::string to_string(SkyTraqBin::BufferUsed bu);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::BufferUsed);

  std::string to_string(SkyTraqBin::PowerMode pm);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::PowerMode);

  std::string to_string(SkyTraqBin::OutputRate o);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::OutputRate);

  std::string to_string(SkyTraqBin::DOPmode mode);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::DOPmode);

  std::string to_string(SkyTraqBin::ElevationCNRmode mode);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::ElevationCNRmode);

  std::string to_string(SkyTraqBin::DefaultOrEnable doe);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::DefaultOrEnable);

  std::string to_string(SkyTraqBin::TalkerID id);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::TalkerID);

  std::string to_string(SkyTraqBin::EnableOrAuto eoa);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::EnableOrAuto);

  std::string to_string(SkyTraqBin::ParameterSearchEngineMode psem);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::ParameterSearchEngineMode);

  std::string to_string(SkyTraqBin::NavigationMode mode);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::NavigationMode);

  std::string to_string(SkyTraqBin::BootStatus bs);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::BootStatus);

  std::string to_string(SkyTraqBin::InterferenceStatus is);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::InterferenceStatus);

  std::string to_string(SkyTraqBin::FixType ft);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::FixType);

  std::string to_string(SkyTraqBin::NavigationState ns);
  ENUM_OSTREAM_OPERATOR(SkyTraqBin::NavigationState);
}; // namespace std

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

#undef RESPONSE1
#undef RESPONSE2
#endif

#endif // __SKYTRAQBIN_HH__
