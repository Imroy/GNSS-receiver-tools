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
#pragma once
#include <stdexcept>
#include <typeinfo>
#include <type_traits>
#include <stdint.h>
#include <math.h>

#define GETTER(type, name, field) inline const type name(void) const { return field; }

namespace GPS {

  //! Base class for subframe information
  class Subframe {
  protected:
    uint8_t _prn;
    // TLM word
    uint8_t _preamble;
    // Hand over word
    uint32_t _tow_count; // 17 bits
    bool _momentum_or_alert_flag, _sync_or_antispoof_flag;
    uint8_t _subframe_num; // 3 bits

    template<typename T>
    const T _bits(const uint8_t* bytes, uint8_t start, uint8_t len = sizeof(T) * 8) const {
      if (start >= 240)
	throw std::out_of_range("Can only start at bits 0~239");
      if (start + len >= 240)
	throw std::out_of_range("Can only end at bits 0~239");
      if (len > sizeof(T) * 8)
	throw std::out_of_range("Can only read " + std::to_string(sizeof(T) * 8) + " bits into " + typeid(T).name());

      // TODO: Make this more efficient by working on whole bytes instead of bits
      T ret = 0;
      for (uint8_t from_bit = start, to_bit = 0; to_bit < len; from_bit++, to_bit++) {
	uint8_t from_byte = from_bit / 8, from_byte_bit = from_bit % 8;
	int8_t shift = from_byte_bit - to_bit;	// right shift; left shift if negative

	uint8_t mask = 1 << from_byte_bit;
	if (shift < 0)
	  ret |= (bytes[from_byte] & mask) << -shift;
	else
	  ret |= (bytes[from_byte] & mask) >> shift;
      }

      if (std::is_signed<T>::value && (len < sizeof(T) * 8)) {
	uint8_t i = len - 1;
	uint8_t mask = 1 << i;
	if (ret & mask)
	  for (i = len; i <= sizeof(T) * 8; i++)
	    ret |= 1 << i;
      }

      return ret;
    }

  public:
    //! Constructor
    /*!
      \param prn PRN of satellite this subframe came from
      \param bytes Pointer to bytes of subframe data
      \param len Length of subframe data
     */
    Subframe(uint8_t prn, const uint8_t *bytes, uint8_t len=30) :
      _prn(prn),
      _preamble(_bits<uint8_t>(bytes, 0, 8)),
      _tow_count(_bits<uint32_t>(bytes, 24, 17)),
      _momentum_or_alert_flag(_bits<bool>(bytes, 41, 1)),
      _sync_or_antispoof_flag(_bits<bool>(bytes, 42, 1)),
      _subframe_num(_bits<uint8_t>(bytes, 43, 3))
    {}

    GETTER(uint8_t, PRN, _prn);
    GETTER(uint8_t, preamble, _preamble);
    GETTER(uint32_t, TOW_count, _tow_count);
    GETTER(bool, momentum_flag,_momentum_or_alert_flag);
    GETTER(bool, alert_flag,_momentum_or_alert_flag);
    GETTER(bool, sync_flag, _sync_or_antispoof_flag);
    GETTER(bool, antispoof_flag, _sync_or_antispoof_flag);
    GETTER(uint8_t, subframe_number, _subframe_num);

  }; // class Subframe


  enum class SignalComponentHealth : uint8_t {
    All_ok = 0,
      Is_temporarily_out = 28,
      Will_be_temporarily_out = 29,
      Spare = 30,
      Bad = 31,	// ?
      Problems,	// ?
  }; // class SignalComponentHealth


  //! Subframe 1
  class Sat_clock_and_health : public Subframe {
  private:
    uint16_t _week_num;		// 10 bits
    uint8_t _ura;		// 4 bits
    // Satellite health
    bool _nav_data_ok;
    SignalComponentHealth _health;
    // five others
    uint16_t _iodc;		// 10 bits
    int8_t _t_gd;
    uint16_t _t_oc;
    int8_t _a_f2;
    int16_t _a_f1;
    int32_t _a_f0;		// 22 bits

  public:
    Sat_clock_and_health(uint8_t prn, const uint8_t *bytes, uint8_t len=30) :
      Subframe(prn, bytes, len),
      _week_num(_bits<uint16_t>(bytes, 48, 10)),
      _ura(_bits<uint8_t>(bytes, 60, 4)),
      _nav_data_ok(_bits<uint16_t>(bytes, 64, 1)),
      _iodc((_bits<uint16_t>(bytes, 70, 2) << 8) | _bits<uint16_t>(bytes, 168, 8)),
      _t_gd(_bits<int8_t>(bytes, 160, 8)),
      _t_oc(_bits<uint16_t>(bytes, 176, 16)),
      _a_f2(_bits<int8_t>(bytes, 192, 8)),
      _a_f1(_bits<int16_t>(bytes, 200, 16)),
      _a_f0(_bits<int32_t>(bytes, 216, 22))
    {
      uint8_t h = _bits<uint8_t>(bytes, 64, 6);
      switch (h) {
      case 0:
      case 28:
      case 29:
      case 30:
      case 31:
	_health = (SignalComponentHealth)h;
	break;

      default:
	_health = SignalComponentHealth::Problems;
      }
    }

    GETTER(uint16_t, week_number, _week_num);
    GETTER(uint8_t, URA, _ura);

    GETTER(bool, navigation_data_ok, _nav_data_ok);
    GETTER(SignalComponentHealth, health, _health);

    GETTER(uint16_t, IODC, _iodc);

    GETTER(int8_t, T_GD_raw, _t_gd);
    GETTER(double, T_GD, _t_gd * pow(2, -31));

    GETTER(uint16_t, t_OC_raw, _t_oc);
    GETTER(uint32_t, t_OC, _t_oc * 16);

    GETTER(int8_t, a_f2_raw, _a_f2);
    GETTER(double, a_f2, _a_f2 * pow(2, -55));

    GETTER(int16_t, a_f1_raw, _a_f1);
    GETTER(double, a_f1, _a_f1 * pow(2, -43));

    GETTER(int32_t, a_f0_raw, _a_f0);
    GETTER(double, a_f0, _a_f0 * pow(2, -31));

  }; // class Sat_clock_and_health


  //! Base class for various "pages" in subframes 4 and 5
  class Subframe_4_or_5 : public Subframe {
  private:
    uint8_t _data_id, _sat_id;	// 2 bits and 6 bits

  public:
    Subframe_4_or_5(uint8_t prn, const uint8_t *bytes, uint8_t len=30) :
      Subframe(prn, bytes, len),
      _data_id(_bits<uint8_t>(bytes, 48, 2)),
      _sat_id(_bits<uint8_t>(bytes, 50, 2))
    {}

    GETTER(uint8_t, data_id, _data_id);
    GETTER(uint8_t, satellite_id, _sat_id);

  }; // class Subframe_4_or_5


  //! Subframe 4, page 18
  class Ionosphere_UTC : public Subframe_4_or_5 {
  private:
    int8_t _alpha_0, _alpha_1, _alpha_2, _alpha_3;
    int8_t _beta_0, _beta_1, _beta_2, _beta_3;
    int32_t _a_0, _a_1;	// _a_1 is only 24 bits
    uint8_t _delta_t_ls, _t_ot, _wn_t, _wn_lsf, _dn;
    int8_t _delta_t_lsf;

  public:
    Ionosphere_UTC(uint8_t prn, const uint8_t *bytes, uint8_t len=30) :
      Subframe_4_or_5(prn, bytes, len),
      _alpha_0(_bits<int8_t>(bytes, 56, 8)),
      _alpha_1(_bits<int8_t>(bytes, 64, 8)),
      _alpha_2(_bits<int8_t>(bytes, 72, 8)),
      _alpha_3(_bits<int8_t>(bytes, 80, 8)),
      _beta_0(_bits<int8_t>(bytes, 88, 8)),
      _beta_1(_bits<int8_t>(bytes, 96, 8)),
      _beta_2(_bits<int8_t>(bytes, 104, 8)),
      _beta_3(_bits<int8_t>(bytes, 112, 8)),
      _a_0(_bits<int32_t>(bytes, 120, 32)),
      _a_1(_bits<int32_t>(bytes, 152, 24)),
      _delta_t_ls(_bits<uint8_t>(bytes, 176, 8)),
      _t_ot(_bits<uint8_t>(bytes, 184, 8)),
      _wn_t(_bits<uint8_t>(bytes, 192, 8)),
      _wn_lsf(_bits<uint8_t>(bytes, 200, 8)),
      _dn(_bits<uint8_t>(bytes, 208, 8)),
      _delta_t_lsf(_bits<int8_t>(bytes, 216, 8))
    {}

    GETTER(int8_t, alpha_0, _alpha_0);
    GETTER(int8_t, alpha_1, _alpha_1);
    GETTER(int8_t, alpha_2, _alpha_2);
    GETTER(int8_t, alpha_3, _alpha_3);
    GETTER(int8_t, beta_0, _beta_0);
    GETTER(int8_t, beta_1, _beta_1);
    GETTER(int8_t, beta_2, _beta_2);
    GETTER(int8_t, beta_3, _beta_3);
    GETTER(int32_t, A_0, _a_0);
    GETTER(int32_t, A_1, _a_1);
    GETTER(uint8_t, delta_t_LS, _delta_t_ls);
    GETTER(uint8_t, t_ot, _t_ot);
    GETTER(uint8_t, WN_t, _wn_t);
    GETTER(uint8_t, WN_LSF, _wn_lsf);
    GETTER(uint8_t, DN, _dn);
    GETTER(int8_t, delta_t_LSF, _delta_t_lsf);

  }; // class Ionosphere_UTC

}; // namespace GPS

#define ENUM_OSTREAM_OPERATOR(type) inline std::ostream& operator<< (std::ostream& out, type val) { out << std::to_string(val); return out;  }

namespace std {
  std::string to_string(GPS::SignalComponentHealth health);
  ENUM_OSTREAM_OPERATOR(GPS::SignalComponentHealth);

}; // namespace std

// Undefine our macros here, unless Doxygen is reading this
#ifndef DOXYGEN_SKIP_FOR_USERS
#undef GETTER
#undef ENUM_OSTREAM_OPERATOR
#endif
