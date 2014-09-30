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
#include <cerrno>
#include <cfenv>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <typeinfo>
#include <type_traits>
#include <stdint.h>

#define GETTER(type, name, field) inline const type name(void) const { return field; }
#define GETTER_RAW(type, name, field) inline const type name##_raw(void) const { return field; }
#define GETTER_MOD(type, name, code) inline const type name(void) const { \
    errno = 0; std::feclearexcept(FE_ALL_EXCEPT);			\
    type ret = code;							\
    if (errno == EDOM) throw std::domain_error(std::strerror(errno));	\
    if (std::fetestexcept(FE_INVALID)) throw std::domain_error("Domain error"); \
    if (errno == ERANGE) throw std::invalid_argument(std::strerror(errno)); \
    if (std::fetestexcept(FE_DIVBYZERO)) throw std::invalid_argument("Pole error"); \
    if (std::fetestexcept(FE_OVERFLOW)) throw std::overflow_error("Overflow error"); \
    if (std::fetestexcept(FE_UNDERFLOW)) throw std::underflow_error("Underflow error"); \
    return ret;								\
  }

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
    static const T _bits(const uint8_t* bytes, uint8_t start, uint8_t len = sizeof(T) * 8) {
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
    static uint32_t extract_tow_count(const uint8_t *bytes, uint8_t len=30) {
      //      return _bits<uint32_t>(bytes, len - 216, 17);
      // TODO _bits() didn't work
      return ((uint32_t)bytes[len-27] << 9)
	| ((uint32_t)bytes[len-26] << 1)
	| ((uint32_t)bytes[len-25] >> 7);
    }
    static uint8_t extract_subframe_number(const uint8_t *bytes, uint8_t len=30) { return 1 + (extract_tow_count(bytes, len) + 4) % 5; }

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
      _subframe_num(extract_subframe_number(bytes, len))
    {}

    //! Virtual deconstructor to force polymorphism
    virtual ~Subframe() {}

    //! Satellite PRN number
    GETTER(uint8_t, PRN, _prn);

    //! Preamble of the TLM word at start of each subframe
    GETTER(uint8_t, preamble, _preamble);

    //! The 17 MSB's of the time-of-week (TOW) count in the hand-over word (HOW)
    GETTER(uint32_t, TOW_count, _tow_count);

    //! Roll momentum dump flag in satellites designated by configuration code 000
    GETTER(bool, momentum_flag,_momentum_or_alert_flag);

    //! Alert flag in satellites designated by configuration code 001
    GETTER(bool, alert_flag,_momentum_or_alert_flag);

    //! Synchronisation flag in satellites designated by configuration code 000
    GETTER(bool, sync_flag, _sync_or_antispoof_flag);

    //! Anti-spoof flag in satellites designated by configuration code 001
    GETTER(bool, antispoof_flag, _sync_or_antispoof_flag);

    //! Subframe number
    GETTER(uint8_t, subframe_number, _subframe_num);

    typedef std::shared_ptr<Subframe> ptr;

    //! Check the type of an object
    template <typename T>
    inline bool isa(void) const { return typeid(*this) == typeid(T); }

    //! Recast this object to another type
    template <typename T>
    inline T* cast_as(void) {
      T *a = dynamic_cast<T*>(this);
      if (a == nullptr)
	throw std::bad_cast();
      return a;
    }

  }; // class Subframe


  Subframe::ptr parse_subframe(uint8_t prn, const uint8_t *bytes, uint8_t len=30);


  //! Health of satellite signal components
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
      _nav_data_ok(_bits<bool>(bytes, 64, 1)),
      _iodc((_bits<uint16_t>(bytes, 70, 2) << 8) | _bits<uint16_t>(bytes, 168, 8)),
      _t_gd(_bits<int8_t>(bytes, 160, 8)),
      _t_oc(_bits<uint16_t>(bytes, 176, 16)),
      _a_f2(_bits<int8_t>(bytes, 192, 8)),
      _a_f1(_bits<int16_t>(bytes, 200, 16)),
      _a_f0(_bits<int32_t>(bytes, 216, 22))
    {
      uint8_t h = _bits<uint8_t>(bytes, 65, 5);
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

    //! The 10 MSB's of the 29-bit Z-count
    GETTER(uint16_t, week_number, _week_num);

    //! Predicted user range accuracy
    GETTER(uint8_t, URA, _ura);

    //! MSB of the 6-bit health indicator
    GETTER(bool, navigation_data_ok, _nav_data_ok);

    //! The 5 LSB's of the 6-bit health indicator
    GETTER(SignalComponentHealth, health, _health);

    //! Issue of data; clock
    GETTER(uint16_t, IODC, _iodc);

    //! Estimated group delay differential, raw value
    GETTER_RAW(int8_t, T_GD, _t_gd);
    //! Estimated group delay differential, seconds
    GETTER_MOD(double, T_GD, _t_gd * pow(2, -31));

    //! Satellite clock correction parameter, raw value
    GETTER_RAW(uint16_t, t_OC, _t_oc);
    //! Satellite clock correction parameter, seconds
    GETTER_MOD(uint32_t, t_OC, _t_oc * 16);

    //! Satellite clock correction second-order term, raw value
    GETTER_RAW(int8_t, a_f2, _a_f2);
    //! Satellite clock correction second-order term, seconds/second^2
    GETTER_MOD(double, a_f2, _a_f2 * pow(2, -55));

    //! Satellite clock correction first-order term, raw value
    GETTER_RAW(int16_t, a_f1, _a_f1);
    //! Satellite clock correction first-order term, seconds/second
    GETTER_MOD(double, a_f1, _a_f1 * pow(2, -43));

    //! Satellite clock correction constant term, raw value
    GETTER_RAW(int32_t, a_f0, _a_f0);
    //! Satellite clock correction constant term, seconds
    GETTER_MOD(double, a_f0, _a_f0 * pow(2, -31));

  }; // class Sat_clock_and_health


  //! Subframe 2
  class Ephemeris1 : public Subframe {
  private:
    uint8_t _iode;
    int16_t _c_rs, _delta_n;
    int32_t _m_0;
    int16_t _c_uc;
    uint32_t _e;
    int16_t _c_us;
    uint32_t _sqrt_a;
    uint16_t _t_oe;

  public:
    Ephemeris1(uint8_t prn, const uint8_t *bytes, uint8_t len=30) :
      Subframe(prn, bytes, len),
      _iode(_bits<uint8_t>(bytes, 48, 8)),
      _c_rs(_bits<int16_t>(bytes, 56, 16)),
      _delta_n(_bits<int16_t>(bytes, 72, 16)),
      _m_0(_bits<int32_t>(bytes, 88, 32)),
      _c_uc(_bits<int16_t>(bytes, 120, 16)),
      _e(_bits<uint32_t>(bytes, 136, 32)),
      _c_us(_bits<int16_t>(bytes, 168, 16)),
      _sqrt_a(_bits<uint32_t>(bytes, 184, 32)),
      _t_oe(_bits<uint16_t>(bytes, 216, 16))
    {}

    //! Issue of data; ephemeris
    GETTER(uint8_t, IODE, _iode);

    //! Amplitude of the sine harmonic correction term to the orbit radius, raw value
    GETTER_RAW(int16_t, C_rs, _c_rs);
    //! Amplitude of the sine harmonic correction term to the orbit radius, metres
    GETTER_MOD(double, C_rs, _c_rs * pow(2, -5));

    //! Mean motion difference from computed value, raw value
    GETTER_RAW(int16_t, delta_n, _delta_n);
    //! Mean motion difference from computed value, semi-circles/second
    GETTER_MOD(double, delta_n, _delta_n * pow(2, -43));

    //! Mean anomaly at reference time, raw value
    GETTER_RAW(int32_t, M_0, _m_0);
    //! Mean anomaly at reference time, semi-circles
    GETTER_MOD(double, M_0, _m_0 * pow(2, -31));

    //! Amplitude of the cosine harmonic correction term to the argument of latitude, raw value
    GETTER_RAW(int16_t, C_uc, _c_uc);
    //! Amplitude of the cosine harmonic correction term to the argument of latitude, radians
    GETTER_MOD(double, C_uc, _c_uc * pow(2, -29));

    //! Eccentricity, raw value
    GETTER_RAW(uint32_t, e, _e);
    //! Eccentricity, real value
    GETTER_MOD(double, e, _e * pow(2, -33));

    //! Amplitude of the sine harmonic correction term to the argument of latitude, raw value
    GETTER_RAW(int16_t, C_us, _c_us);
    //! Amplitude of the sine harmonic correction term to the argument of latitude, radians
    GETTER_MOD(double, C_us, _c_us * pow(2, -29));

    //! Square root of the semi-major axis, raw value
    GETTER_RAW(uint32_t, sqrt_A, _sqrt_a);
    //! Square root of the semi-major axis, metres^(1/2)
    GETTER_MOD(double, sqrt_A, _sqrt_a * pow(2, -19));

    //! Reference time ephemeris, raw value
    GETTER_RAW(uint16_t, t_oe, _t_oe);
    //! Reference time ephemeris, seconds
    GETTER_MOD(uint32_t, t_oe, _t_oe << 4);

  }; // class Ephemeris1


  //! Subframe 3
  class Ephemeris2 : public Subframe {
  private:
    int16_t _c_ic;
    int32_t _omega_0;
    int16_t _c_is;
    int32_t _i_0;
    int16_t _c_rc;
    int32_t _omega, _omegadot;	// omegadot is 24 bits
    uint8_t _iode;
    int16_t _idot;		// 14 bits

  public:
    Ephemeris2(uint8_t prn, const uint8_t *bytes, uint8_t len=30) :
      Subframe(prn, bytes, len),
      _c_ic(_bits<int16_t>(bytes, 48, 16)),
      _omega_0(_bits<int32_t>(bytes, 64, 32)),
      _c_is(_bits<int16_t>(bytes, 96, 16)),
      _i_0(_bits<int32_t>(bytes, 112, 32)),
      _c_rc(_bits<int16_t>(bytes, 144, 16)),
      _omega(_bits<int32_t>(bytes, 160, 32)),
      _omegadot(_bits<int32_t>(bytes, 192, 24)),
      _iode(_bits<uint8_t>(bytes, 216, 8)),
      _idot(_bits<int16_t>(bytes, 224, 14))
    {}

    //! Amplitude of the cosine harmonic correction term to the angle of inclination, raw value
    GETTER_RAW(int16_t, C_ic, _c_ic);
    //! Amplitude of the cosine harmonic correction term to the angle of inclination, radians
    GETTER_MOD(double, C_ic, _c_ic * pow(2, -29));

    //! Longitude of ascending node of orbit plane at weekly epoch, raw value
    GETTER_RAW(int32_t, OMEGA_0, _omega_0);
    //! Longitude of ascending node of orbit plane at weekly epoch, semi-circles
    GETTER_MOD(double, OMEGA_0, _omega_0 * pow(2, -31));

    //! Amplitude of the sine harmonic correction term to the angle of inclination, raw value
    GETTER_RAW(int16_t, C_is, _c_is);
    //! Amplitude of the sine harmonic correction term to the angle of inclination, radians
    GETTER_MOD(double, C_is, _c_is * pow(2, -29));

    //! Inclination angle at reference time, raw value
    GETTER_RAW(int32_t, i_0, _i_0);
    //! Inclination angle at reference time, semi-circles
    GETTER_MOD(double, i_0, _i_0 * pow(2, -31));

    //! Amplitude of the cosine harmonic correction term to the orbit radius, raw value
    GETTER_RAW(int16_t, C_rc, _c_rc);
    //! Amplitude of the cosine harmonic correction term to the orbit radius, metres
    GETTER_MOD(double, C_rc, _c_rc * pow(2, -5));

    //! Argument of perigee, raw value
    GETTER_RAW(int32_t, omega, _omega);
    //! Argument of perigee, semi-circles
    GETTER_MOD(double, omega, _omega * pow(2, -31));

    //! Rate of right ascension, raw value
    GETTER_RAW(int32_t, OMEGADOT, _omegadot);
    //! Rate of right ascension, semi-circles/second
    GETTER_MOD(double, OMEGADOT, _omegadot * pow(2, -43));

    //! Issue of data; ephemeris
    GETTER(uint8_t, IODE, _iode);

    //! Rate of inclination angle, raw value
    GETTER_RAW(int16_t, IDOT, _idot);
    //! Rate of inclination angle, semi-circles/second
    GETTER_MOD(double, IDOT, _idot * pow(2, -43));

  }; // class Ephemeris2


  //! Base class for various "pages" in subframes 4 and 5
  class Subframe_4_or_5 : public Subframe {
  private:
    uint8_t _page_num;
    uint8_t _data_id, _sat_id;	// 2 bits and 6 bits

  public:
    static uint8_t extract_page_number(const uint8_t *bytes, uint8_t len=30) {
      uint8_t subframe_num = extract_subframe_number(bytes, len);
      uint32_t tow_count = extract_tow_count(bytes, len);
      return 1 + ((tow_count - subframe_num + 1) / 20) % 25;
    }

    Subframe_4_or_5(uint8_t prn, const uint8_t *bytes, uint8_t len=30) :
      Subframe(prn, bytes, len),
      _page_num(extract_page_number(bytes, len)),
      _data_id(_bits<uint8_t>(bytes, 48, 2)),
      _sat_id(_bits<uint8_t>(bytes, 50, 2))
    {}

    GETTER(uint8_t, page_number, _page_num);
    GETTER(uint8_t, data_id, _data_id);
    GETTER(uint8_t, satellite_id, _sat_id);

  }; // class Subframe_4_or_5


  //! Subframe 4, pages 1, 6, 11~16, 19~24
  class Reserved_and_spare : public Subframe_4_or_5 {
  public:
    Reserved_and_spare(uint8_t prn, const uint8_t *bytes, uint8_t len=30) :
      Subframe_4_or_5(prn, bytes, len)
    {}

  }; // class Reserved_and_spare


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

    //! Ionospheric model constant term, raw value
    GETTER_RAW(int8_t, alpha_0, _alpha_0);
    //! Ionospheric model constant term, seconds
    GETTER_MOD(double, alpha_0, _alpha_0 * pow(2, -30));

    //! Ionospheric model first-order term, raw value
    GETTER_RAW(int8_t, alpha_1, _alpha_1);
    //! Ionospheric model first-order term, seconds/semi-circle
    GETTER_MOD(double, alpha_1, _alpha_1 * pow(2, -27));

    //! Ionospheric model second-order term, raw value
    GETTER_RAW(int8_t, alpha_2, _alpha_2);
    //! Ionospheric model second-order term, seconds/semi-circle^2
    GETTER_MOD(double, alpha_2, _alpha_2 * pow(2, -24));

    //! Ionospheric model third-order term, raw value
    GETTER_RAW(int8_t, alpha_3, _alpha_3);
    //! Ionospheric model third-order term, seconds/semi-circle^3
    GETTER_MOD(double, alpha_3, _alpha_3 * pow(2, -24));

    //! Ionospheric model constant term, raw value
    GETTER_RAW(int8_t, beta_0, _beta_0);
    //! Ionospheric model constant term, seconds
    GETTER_MOD(int32_t, beta_0, _beta_0 << 11);

    //! Ionospheric model first-order term, raw value
    GETTER_RAW(int8_t, beta_1, _beta_1);
    //! Ionospheric model first-order term, seconds/semi-circle
    GETTER_MOD(int32_t, beta_1, _beta_1 << 14);

    //! Ionospheric model second-order term, raw value
    GETTER_RAW(int8_t, beta_2, _beta_2);
    //! Ionospheric model second-order term, seconds/semi-circle^2
    GETTER_MOD(int32_t, beta_2, _beta_2 << 16);

    //! Ionospheric model third-order term, raw value
    GETTER_RAW(int8_t, beta_3, _beta_3);
    //! Ionospheric model third-order term, seconds/semi-circle^3
    GETTER_MOD(int32_t, beta_3, _beta_3 << 16);

    //! UTC correction constant term, raw value
    GETTER_RAW(int32_t, A_0, _a_0);
    //! UTC correction constant term, seconds
    GETTER_MOD(double, A_0, _a_0 * pow(2, -30));

    //! UTC correction first-order term, raw value
    GETTER_RAW(int32_t, A_1, _a_1);
    //! UTC correction first-order term, seconds/second
    GETTER_MOD(double, A_1, _a_1 * pow(2, -50));

    //! Delta time due to leap seconds, seconds
    GETTER(uint8_t, delta_t_LS, _delta_t_ls);

    //! Reference time for UTC data, raw value
    GETTER_RAW(uint8_t, t_ot, _t_ot);
    //! Reference time for UTC data, seconds
    GETTER_MOD(uint32_t, t_ot, _t_ot << 12);

    //! UTC reference week number, weeks
    GETTER(uint8_t, WN_t, _wn_t);
    //! Week number of future leap-second event, weeks
    GETTER(uint8_t, WN_LSF, _wn_lsf);
    //! Day number of future leap-second event, days
    GETTER(uint8_t, DN, _dn);
    //! Delta time due to future leap seconds, seconds
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
#undef GETTER_RAW
#undef GETTER_MOD
#undef ENUM_OSTREAM_OPERATOR
#endif
