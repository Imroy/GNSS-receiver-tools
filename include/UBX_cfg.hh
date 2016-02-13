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

  namespace Cfg {

    class Q_prt : public Input_message {
    private:
      PortID _portid;
      bool _current_port;

      GETTER(Length, body_length, _current_port ? 0 : 1);
      virtual void body_to_buf(unsigned char* buffer) const;

    public:
      //! Constructor for polling current port
      inline Q_prt(void) :
	Input_message(Class_ID::CFG, 0x00),
	_current_port(true)
      {}

      //! Constructor
      /*!
	\param p Port ID to query
      */
      inline Q_prt(PortID p) :
	Input_message(Class_ID::CFG, 0x00),
	_portid(p),
	_current_port(false)
      {}

    }; // class Q_prt


    enum class txReady_polarity : bool {
      Hi_active = false,
	Lo_active = true,
    };

    enum class Mode_Charlen : uint8_t {
      Bits_5 = 0,
	Bits_6,
	Bits_7,
	Bits_8,
    };

    enum class Mode_Parity : uint8_t {
      Even,
	Odd,
	Reserved = 2,	// actually x1x
	None = 4,	// actually 10x
    };

    enum class Mode_Stopbits : uint8_t {
      Bits_1,		// 1 stop bit
	Bits_1_5,	// 1.5 stop bits
	Bits_2,		// 2 stop bits
	Bits_0_5,	// 0.5 stop bit
    };

    class Prt : public Input_message {
    private:
      PortID _portid;
      bool _txr_en, _txr_pol;
      uint8_t _txr_pin;
      uint16_t _txr_thres;
      Mode_Charlen _mode_charlen;
      Mode_Parity _mode_parity;
      Mode_Stopbits _mode_stopbits;
      uint32_t _baudrate;
      bool _inproto_ubx, _inproto_nmea, _inproto_rtcm;
      bool _outproto_ubx, _outproto_nmea;
      bool _flags_exttxtimeout;

      GETTER(Length, body_length, 20);
      virtual void body_to_buf(unsigned char* buffer) const;

    public:
      inline Prt(PortID p) :
	Input_message(Class_ID::CFG, 0x00),
	_portid(p),
	_txr_en(false),
	_mode_charlen(Mode_Charlen::Bits_8), _mode_parity(Mode_Parity::None), _mode_stopbits(Mode_Stopbits::Bits_1),
	_baudrate(9600),
	_inproto_ubx(true), _inproto_nmea(true), _inproto_rtcm(false),
	_outproto_ubx(false), _outproto_nmea(true),
	_flags_exttxtimeout(false)
      {}

      GETTER(Mode_Charlen, mode_charlen, _mode_charlen);
      GETTER(Mode_Parity, mode_parity, _mode_parity);
      GETTER(Mode_Stopbits, mode_stopbits, _mode_stopbits);

      inline void step_mode(Mode_Charlen cl, Mode_Parity p, Mode_Stopbits sb) {
	_mode_charlen = cl;
	_mode_parity = p;
	_mode_stopbits = sb;
      }

      GETTER_SETTER(uint32_t, baudrate, _baudrate);
      GETTER_SETTER(bool, inproto_ubx, _inproto_ubx);
      GETTER_SETTER(bool, inproto_nmea, _inproto_nmea);
      GETTER_SETTER(bool, inproto_rtcm, _inproto_rtcm);
      GETTER_SETTER(bool, outproto_ubx, _outproto_ubx);
      GETTER_SETTER(bool, outproto_nmea, _outproto_nmea);

    }; // class Prt


    class Q_msg : public Input_message {
    private:
      uint8_t _msg_class, _msg_id;

      GETTER(Length, body_length, 2);
      virtual void body_to_buf(unsigned char* buffer) const;

    public:
      //! Constructor
      /*!
	\param p_cls Message class
	\param p_id Message ID
      */
      inline Q_msg(uint8_t p_cls, uint8_t p_id) :
	Input_message(Class_ID::CFG, 0x01),
	_msg_class(p_cls), _msg_id(p_id)
      {}

      GETTER_SETTER(uint8_t, message_class, _msg_class);
      GETTER_SETTER(uint8_t, message_id, _msg_id);
    }; // class Q_msg


    class Msg : public Input_message {
    private:
      uint8_t _msg_class, _msg_id, _rates[6];
      bool _current_port;

      GETTER(Length, body_length, _current_port ? 3 : 8);
      virtual void body_to_buf(unsigned char* buffer) const;

    public:
      //! Constructor for setting rates on all six ports
      /*!
	\param m_cls Message class
	\param m_id Message ID
	\param rates Rates array
      */
      inline Msg(uint8_t m_cls, uint8_t m_id, uint8_t rates[6]) :
	Input_message(Class_ID::CFG, 0x01),
	_msg_class(m_cls), _msg_id(m_id),
	_rates{ rates[0], rates[1], rates[2], rates[3], rates[4], rates[5] },
	_current_port(false)
      {}

      //! Constructor for setting rate on current port
      /*!
	\param m_cls Message class
	\param m_id Message ID
	\param rate Rate for current port
      */
      inline Msg(uint8_t m_cls, uint8_t m_id, uint8_t rate) :
	Input_message(Class_ID::CFG, 0x01),
	_msg_class(m_cls), _msg_id(m_id),
	_rates{ rate, 0, 0, 0, 0, 0 },
	_current_port(true)
      {}

      GETTER_SETTER(uint8_t, message_class, _msg_class);
      GETTER_SETTER(uint8_t, message_id, _msg_id);
      GETTER(bool, is_current_port, _current_port);

      inline const uint8_t rate(void) const { return _rates[0]; }
      inline void set_rate(uint8_t r) { _rates[0] = r; _current_port = true; }

      inline const uint8_t rate(PortID p) const { return _rates[(int)p]; }
      inline void set_rate(PortID p, uint8_t r) { _rates[(int)p] = r; _current_port = false; }
    }; // class Msg

  }; // namespace Cfg

}; // namespace UBX
