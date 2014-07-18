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
#include <functional>
#include <iostream>
#include <iomanip>
#include <map>
#include <stdexcept>
#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include "SkyTraqBin.hh"
#include "BE.hh"

namespace SkyTraqBin {

  uint8_t checksum(unsigned char* buffer, Payload_length len) {
    uint8_t cs = 0;
    while (len) {
      cs ^= *buffer;
      buffer++;
      len--;
    }

    return cs;
  }


  void Input_message::to_buf(unsigned char* buffer) const {
    append_be<uint8_t>(buffer, 0xa0);
    append_be<uint8_t>(buffer, 0xa1);

    Payload_length payload_len = body_length() + MsgID_len;
    append_be(buffer, payload_len);

    unsigned char *payload = buffer;
    append_be(buffer, _msg_id);
    body_to_buf(buffer);
    buffer = payload + payload_len;

    append_be(buffer, checksum(payload, payload_len));
    append_be<uint8_t>(buffer, 0x0d);
    append_be<uint8_t>(buffer, 0x0a);
  }


  void Input_message_with_subid::to_buf(unsigned char* buffer) const {
    append_be<uint8_t>(buffer, 0xa0);
    append_be<uint8_t>(buffer, 0xa1);

    Payload_length payload_len = body_length() + MsgID_len + MsgSubID_len;
    append_be(buffer, payload_len);

    unsigned char *payload = buffer;
    append_be(buffer, _msg_id);
    append_be(buffer, _msg_subid);
    body_to_buf(buffer);
    buffer = payload + payload_len;

    append_be(buffer, checksum(payload, payload_len));
    append_be<uint8_t>(buffer, 0x0d);
    append_be<uint8_t>(buffer, 0x0a);
  }


  typedef std::function<Output_message::ptr(uint8_t*, Payload_length)> output_message_factory;
#define OUTPUT1(ID, CLASS) std::make_pair<uint16_t, output_message_factory>((ID), [](uint8_t* payload, Payload_length len) { return std::make_shared<CLASS>(payload, len); })
#define OUTPUT2(ID, SUBID, CLASS) std::make_pair<uint16_t, output_message_factory>(((ID) << 8) | (SUBID), [](uint8_t* payload, Payload_length len) { return std::make_shared<CLASS>(payload, len); })

  std::map<uint16_t, output_message_factory> output_message_factories = {
    OUTPUT2(0x62, 0x80, GNSS_SBAS_status),
    OUTPUT2(0x62, 0x81, GNSS_QZSS_status),
    OUTPUT2(0x63, 0x80, GNSS_SAEE_status),
    OUTPUT2(0x64, 0x80, GNSS_boot_status),
    OUTPUT2(0x64, 0x81, GNSS_extended_NMEA_msg_interval),
    OUTPUT2(0x64, 0x83, GNSS_interference_detection_status),
    OUTPUT2(0x64, 0x8B, GNSS_nav_mode),
    OUTPUT2(0x64, 0x8C, GNSS_constellation_type),
    OUTPUT2(0x64, 0x8E, GNSS_time),
    OUTPUT2(0x65, 0x80, GNSS_1PPS_pulse_width),
    OUTPUT1(0x80, Sw_ver),
    OUTPUT1(0x81, Sw_CRC),
    OUTPUT1(0x83, Ack),
    OUTPUT1(0x84, Nack),
    OUTPUT1(0x86, Pos_update_rate),
    OUTPUT1(0x87, GPS_almanac_data),
    OUTPUT1(0x93, NMEA_talker_ID),
    OUTPUT1(0xA8, Nav_data_msg),
    OUTPUT1(0xAE, GNSS_datum),
    OUTPUT1(0xAF, GNSS_DOP_mask),
    OUTPUT1(0xB0, GNSS_elevation_CNR_mask),
    OUTPUT1(0xB1, GPS_ephemeris_data),
    OUTPUT1(0xB4, GNSS_pos_pinning_status),
    OUTPUT1(0xB9, GNSS_power_mode_status),
    OUTPUT1(0xBB, GNSS_1PPS_cable_delay),
    OUTPUT2(0xCF, 0x01, Sensor_data),
    OUTPUT1(0xDC, Measurement_time),
    OUTPUT1(0xDD, Raw_measurements),
    OUTPUT1(0xDE, SV_channel_status),
    OUTPUT1(0xDF, Rcv_state),
    OUTPUT1(0xE0, Subframe_data),
  };
#undef OUTPUT1
#undef OUTPUT2


  Output_message::ptr parse_message(unsigned char* buffer, std::size_t len) {
    Payload_length payload_len = extract_be<uint16_t>(buffer, 2);
    std::size_t end = StartSeq_len + PayloadLength_len + payload_len + Checksum_len + EndSeq_len;
    if (len < end)
      throw InsufficientData();

    if ((buffer[0] != 0xa0)
	|| (buffer[1] != 0xa1)
	|| (buffer[end - 2] != 0x0d)
	|| (buffer[end - 1] != 0x0a))
      throw InvalidMessage();

    unsigned char *payload = buffer + StartSeq_len + PayloadLength_len;
    uint16_t id = payload[0];
    if ((id >= 0x62) && (id <= 0x65))	// construct a composite id if the message has a sub-ID
      id = (payload[0] << 8) | payload[1];

    if (output_message_factories.count(id) == 0)
      throw UnknownMessageID(id);

    uint8_t cs = buffer[end - 3];
    {
      uint8_t ccs = checksum(payload, payload_len);
      if (cs != ccs)
	throw ChecksumMismatch(ccs, cs);
    }

    return output_message_factories[id](payload, payload_len);
  }


  std::ostream& operator<< (std::ostream& out, StartMode mode) {
    out << std::to_string(mode);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, SwType st) {
    out << std::to_string(st);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, BaudRate rate){
    out << std::to_string(rate);
    return out;
  }

  BaudRate rate_to_BaudRate(unsigned int rate) {
    if (rate > 921600)
      throw std::invalid_argument("Baud rate too high");

    if (rate > 460800)
      return BaudRate::Baud921600;

    if (rate > 230400)
      return BaudRate::Baud460800;

    if (rate > 115200)
      return BaudRate::Baud230400;

    if (rate > 57600)
      return BaudRate::Baud115200;

    if (rate > 38400)
      return BaudRate::Baud57600;

    if (rate > 19200)
      return BaudRate::Baud38400;

    if (rate > 9600)
      return BaudRate::Baud19200;

    if (rate > 4800)
      return BaudRate::Baud9600;

    return BaudRate::Baud4800;
  }

  unsigned int BaudRate_rate(BaudRate br) {
    switch (br) {
    case BaudRate::Baud4800:
      return 4800;
      break;
    case BaudRate::Baud9600:
      return 9600;
      break;
    case BaudRate::Baud19200:
      return 19200;
      break;
    case BaudRate::Baud38400:
      return 38400;
      break;
    case BaudRate::Baud57600:
      return 57600;
      break;
    case BaudRate::Baud115200:
      return 115200;
      break;
    case BaudRate::Baud230400:
      return 230400;
      break;
    case BaudRate::Baud460800:
      return 460800;
      break;
    case BaudRate::Baud921600:
      return 921600;
      break;
    }

    throw std::invalid_argument("Unrecognised baud rate");
  }

  std::ostream& operator<< (std::ostream& out, UpdateType ut) {
    out << std::to_string(ut);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, MessageType mt) {
    out << std::to_string(mt);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, FlashType ft) {
    out << std::to_string(ft);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, BufferUsed bu) {
    out << std::to_string(bu);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, PowerMode pm) {
    out << std::to_string(pm);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, OutputRate o) {
    out << std::to_string(o);
    return out;
  }

  OutputRate Hz_to_OutputRate(unsigned int hz) {
    if (hz < 2)
      return OutputRate::Rate1Hz;

    if (hz < 4)
      return OutputRate::Rate2Hz;

    if (hz < 5)
      return OutputRate::Rate4Hz;

    if (hz < 10)
      return OutputRate::Rate5Hz;

    if (hz < 20)
      return OutputRate::Rate10Hz;

    if (hz == 20)
      return OutputRate::Rate20Hz;

    throw std::invalid_argument("Output rate too high");
  }

  unsigned int OutputRate_Hz(OutputRate r) {
    switch (r) {
    case OutputRate::Rate1Hz:
      return 1;
      break;
    case OutputRate::Rate2Hz:
      return 2;
      break;
    case OutputRate::Rate4Hz:
      return 4;
      break;
    case OutputRate::Rate5Hz:
      return 5;
      break;
    case OutputRate::Rate10Hz:
      return 10;
      break;
    case OutputRate::Rate20Hz:
      return 20;
      break;
    }

    throw std::invalid_argument("Unrecognised output rate");
  }

  std::ostream& operator<< (std::ostream& out, DOPmode mode) {
    out << std::to_string(mode);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, ElevationCNRmode mode) {
    out << std::to_string(mode);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, DefaultOrEnable doe) {
    out << std::to_string(doe);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, TalkerID id) {
    out << std::to_string(id);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, EnableOrAuto eoa) {
    out << std::to_string(eoa);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, NavigationMode mode) {
    out << std::to_string(mode);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, BootStatus bs) {
    out << std::to_string(bs);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, InterferenceStatus is) {
    out << std::to_string(is);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, FixType ft) {
    out << std::to_string(ft);
    return out;
  }

  std::ostream& operator<< (std::ostream& out, NavigationState ns) {
    out << std::to_string(ns);
    return out;
  }


}; // namespace SkyTraqBin

namespace std {

  string to_string(SkyTraqBin::StartMode mode) {
    switch (mode) {
    case SkyTraqBin::StartMode::HotStart:
      return "hot start";
    case SkyTraqBin::StartMode::WarmStart:
      return "warm start";
    case SkyTraqBin::StartMode::ColdStart:
      return "cold start";
    }
    throw invalid_argument("Unrecognised value for StartMode");
  }

  string to_string(SkyTraqBin::SwType sw) {
    switch (sw) {
    case SkyTraqBin::SwType::SystemCode:
      return "system code";
    }
    throw invalid_argument("Unrecognised value for SwType");
  }

  string to_string(SkyTraqBin::BaudRate r) {
    return to_string(SkyTraqBin::BaudRate_rate(r));
  }

  string to_string(SkyTraqBin::UpdateType ut) {
    switch (ut) {
    case SkyTraqBin::UpdateType::SRAM:
      return "SRAM";
    case SkyTraqBin::UpdateType::SRAM_and_flash:
      return "SRAM and Flash";
    case SkyTraqBin::UpdateType::Temporary:
      return "temporary";
    }
    throw invalid_argument("Unrecognised value for UpdateType");
  }

  string to_string(SkyTraqBin::MessageType mt) {
    switch (mt) {
    case SkyTraqBin::MessageType::None:
      return "none";
      break;
    case SkyTraqBin::MessageType::NMEA0183:
      return "NMEA-0183";
      break;
    case SkyTraqBin::MessageType::Binary:
      return "binary";
      break;
    }
    throw invalid_argument("Unrecognised value for SwType");
  }

  string to_string(SkyTraqBin::FlashType ft) {
    switch (ft) {
    case SkyTraqBin::FlashType::Auto:
      return "auto";
    case SkyTraqBin::FlashType::QSPI_Winbond:
      return "QSPI Winbond";
    case SkyTraqBin::FlashType::QSPI_EON:
      return "QSPI Eon";
    case SkyTraqBin::FlashType::Parallel_Numonyx:
      return "parallel Flash Numonyx";
    case SkyTraqBin::FlashType::Parallel_EON:
      return "parallel Flash Eon";
    }
    throw invalid_argument("Unrecognised value for FlashType");
  }

  string to_string(SkyTraqBin::BufferUsed bu) {
    switch (bu) {
    case SkyTraqBin::BufferUsed::Size8K:
      return "8 KiB";
    case SkyTraqBin::BufferUsed::Size16K:
      return "16 KiB";
    case SkyTraqBin::BufferUsed::Size24K:
      return "24 KiB";
    case SkyTraqBin::BufferUsed::Size32K:
      return "32 KiB";
    }
    throw invalid_argument("Unrecognised value for BufferUsed");
  }

  string to_string(SkyTraqBin::PowerMode pm) {
    switch (pm) {
    case SkyTraqBin::PowerMode::Normal:
      return "normal";
    case SkyTraqBin::PowerMode::PowerSave:
      return "power save";
    }
    throw invalid_argument("Unrecognised value for PowerMode");
  }

  string to_string(SkyTraqBin::OutputRate o) {
    switch (o) {
    case SkyTraqBin::OutputRate::Rate1Hz:
      return "1 Hz";
    case SkyTraqBin::OutputRate::Rate2Hz:
      return "2 Hz";
    case SkyTraqBin::OutputRate::Rate4Hz:
      return "4 Hz";
    case SkyTraqBin::OutputRate::Rate5Hz:
      return "5 Hz";
    case SkyTraqBin::OutputRate::Rate10Hz:
      return "10 Hz";
    case SkyTraqBin::OutputRate::Rate20Hz:
      return "20 Hz";
    }
    throw invalid_argument("Unrecognised value for OutputRate");
  }

  string to_string(SkyTraqBin::DOPmode mode) {
    switch (mode) {
    case SkyTraqBin::DOPmode::Disable:
      return "disable";
    case SkyTraqBin::DOPmode::Auto:
      return "automatic";
    case SkyTraqBin::DOPmode::PDOP_only:
      return "PDOP only";
    case SkyTraqBin::DOPmode::HDOP_only:
      return "HDOP only";
    case SkyTraqBin::DOPmode::GDOP_only:
      return "GDOP only";
    }
    throw invalid_argument("Unrecognised value for DOPmode");
  }

  string to_string(SkyTraqBin::ElevationCNRmode mode) {
    switch (mode) {
    case SkyTraqBin::ElevationCNRmode::Disable:
      return "disable";
    case SkyTraqBin::ElevationCNRmode::ElevationCNR:
      return "elevation and CNR";
    case SkyTraqBin::ElevationCNRmode::Elevation_only:
      return "elevation only";
    case SkyTraqBin::ElevationCNRmode::CNR_only:
      return "CNR only";
    }
    throw invalid_argument("Unrecognised value for ElevationCNRmode");
  }

  string to_string(SkyTraqBin::DefaultOrEnable doe) {
    switch (doe) {
    case SkyTraqBin::DefaultOrEnable::Default:
      return "default";
    case SkyTraqBin::DefaultOrEnable::Enable:
      return "enable";
    case SkyTraqBin::DefaultOrEnable::Disable:
      return "disable";
    }
    throw invalid_argument("Unrecognised value for DefaultOrEnable");
  }

  string to_string(SkyTraqBin::TalkerID id) {
    switch (id) {
    case SkyTraqBin::TalkerID::GP:
      return "GP mode";
    case SkyTraqBin::TalkerID::GN:
      return "GN mode";
    }
    throw invalid_argument("Unrecognised value for TalkerID");
  }

  string to_string(SkyTraqBin::EnableOrAuto eoa) {
    switch (eoa) {
    case SkyTraqBin::EnableOrAuto::Disable:
      return "disable";
    case SkyTraqBin::EnableOrAuto::Enable:
      return "enable";
    case SkyTraqBin::EnableOrAuto::Auto:
      return "automatic";
    }
    throw invalid_argument("Unrecognised value for EnableOrAuto");
  }

  string to_string(SkyTraqBin::NavigationMode mode) {
    switch (mode) {
    case SkyTraqBin::NavigationMode::Auto:
      return "automatic";
    case SkyTraqBin::NavigationMode::Pedestrian:
      return "pedestrian";
    case SkyTraqBin::NavigationMode::Car:
      return "car";
    case SkyTraqBin::NavigationMode::Marine:
      return "marine";
    case SkyTraqBin::NavigationMode::Balloon:
      return "balloon";
    case SkyTraqBin::NavigationMode::Airborne:
      return "airborne";
    }
    throw invalid_argument("Unrecognised value for NavigatioMode");
  }

  string to_string(SkyTraqBin::BootStatus bs) {
    switch (bs) {
    case SkyTraqBin::BootStatus::FromFlash:
      return "boot from Flash";
    case SkyTraqBin::BootStatus::FromROM:
      return "boot from ROM due to Flash boot failure";
    }
    throw invalid_argument("Unrecognised value for BootStatus");
  }

  string to_string(SkyTraqBin::InterferenceStatus is) {
    switch (is) {
    case SkyTraqBin::InterferenceStatus::Unknown:
      return "unknown";
    case SkyTraqBin::InterferenceStatus::None:
      return "none";
    case SkyTraqBin::InterferenceStatus::Little:
      return "little";
    case SkyTraqBin::InterferenceStatus::Critical:
      return "critical";
    }
    throw invalid_argument("Unrecognised value for InterferenceStatus");
  }

  string to_string(SkyTraqBin::FixType ft) {
    switch (ft) {
    case SkyTraqBin::FixType::None:
      return "none";
    case SkyTraqBin::FixType::TwoDimensional:
      return "2D";
    case SkyTraqBin::FixType::ThreeDimensional:
      return "3D";
    case SkyTraqBin::FixType::Differential:
      return "differential";
    }
    throw invalid_argument("Unrecognised value for FixType");
  }

  string to_string(SkyTraqBin::NavigationState ns) {
    switch (ns) {
    case SkyTraqBin::NavigationState::NoFix:
      return "no fix";
    case SkyTraqBin::NavigationState::Predicted:
      return "predicted";
    case SkyTraqBin::NavigationState::TwoDimensional:
      return "2D";
    case SkyTraqBin::NavigationState::ThreeDimensional:
      return "3D";
    case SkyTraqBin::NavigationState::Differential:
      return "differential";
    }
    throw invalid_argument("Unrecognised value for NavigationState");
  }


}; // namespace std
