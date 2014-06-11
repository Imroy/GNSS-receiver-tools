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
#ifndef __SKYTRAQBIN_OUTPUTS_HH__
#define __SKYTRAQBIN_OUTPUTS_HH__

#include <stdexcept>
#include "SkyTraqBin.hh"

namespace SkyTraqBin {

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
  }; //class NavigationState


  struct PackedVersion {
    uint8_t X, Y, Z;

    inline PackedVersion(uint8_t x, uint8_t y, uint8_t z) :
      X(x), Y(y), Z(z)
    {}
  }; // struct PackedVersion


  struct PackedDate {
    uint8_t year, month, day;

    inline PackedDate(uint8_t y, uint8_t m, uint8_t d) :
      year(y), month(m), day(d)
    {}
  }; // struct PackedDate


  //! SOFTWARE VERSION - Software version of the GNSS receiver
  //! - Answer to Q_sw_ver message
  class Sw_ver : public Output_message {
  private:
    SwType _sw_type;
    PackedVersion _kernel_ver, _odm_ver;
    PackedDate _revision;

  public:
    //! Constructor from a binary buffer
    Sw_ver(unsigned char* payload, Payload_length payload_len);

    GETTER(SwType, software_type, _sw_type);
    GETTER(PackedVersion, kernel_version, _kernel_ver);
    GETTER(PackedVersion, ODM_version, _odm_ver);
    GETTER(PackedDate, revision, _revision);

  }; // class Sw_ver


  //! SOFTWARE CRC -Software CRC of the GNSS receiver
  //! - Answer to Q_sw_CRC message
  class Sw_CRC : public Output_message {
  private:
    SwType _sw_type;
    uint16_t _crc;

  public:
    //! Constructor from a binary buffer
    Sw_CRC(unsigned char* payload, Payload_length payload_len);

    GETTER(SwType, software_type, _sw_type);
    GETTER(uint16_t, CRC, _crc);

  }; // class Sw_CRC


  //! ACK - Acknowledgement to a Request Message
  class Ack : public Output_message {
  private:
    uint8_t _ack_id;
    bool _has_subid;
    uint8_t _ack_subid;

  public:
    //! Constructor from a binary buffer
    Ack(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, ack_id, _ack_id);
    GETTER(bool, has_subid, _has_subid);
    GETTER(uint8_t, ack_subid, _ack_subid);

  }; // class Ack


  //! NACK - Response to an unsuccessful request message
  class Nack : public Output_message {
  private:
    uint8_t _nack_id;
    bool _has_subid;
    uint8_t _nack_subid;

  public:
    //! Constructor from a binary buffer
    Nack(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, nack_id, _nack_id);
    GETTER(bool, has_subid, _has_subid);
    GETTER(uint8_t, nack_subid, _nack_subid);

  }; // class Ack


  //! POSITON UPDATE RATE - Position Update rate of the GNSS system
  //! - Answer to Q_pos_update_rate message
  class Pos_update_rate : public Output_message {
  private:
    uint8_t _update_rate;

  public:
    //! Constructor from a binary buffer
    Pos_update_rate(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, update_rate, _update_rate);

  }; // class Pos_update_rate


  //! GPS ALMANAC Data - almanac data of GPS receiver
  class GPS_almanac_data : public Output_message {
  private:
    uint8_t _prn;
    uint32_t _words[8];
    int16_t _week_no;

  public:
    GPS_almanac_data(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, PRN, _prn);
    GETTER(uint32_t*, words, _words);
    inline const uint32_t word(int i) const { return _words[i+3]; }
    GETTER(int16_t, week_no, _week_no);

  }; // class GPS_almanac_data


  //! GNSS NMEA TALKER ID - NMEA talker ID of GNSS receiver
  //! - Answer to Q_NMEA_talker_ID message
  class NMEA_talker_ID : public Output_message {
  private:
    TalkerID _talker_id;

  public:
    //! Constructor from a binary buffer
    NMEA_talker_ID(unsigned char* payload, Payload_length payload_len);

    GETTER(TalkerID, talker_id, _talker_id);

  }; //class NMEA_talker_ID


  //! NAVIGATION DATA MESSAGE - Message of user navigation data in binary format
  class Nav_data_msg : public Output_message {
  private:
    FixType _fix_type;
    uint8_t _num_sv;
    uint16_t _week_no;
    uint32_t _tow;
    int32_t _lat, _lon; // * 1e-7 degrees, negative is south/west
    int32_t _e_alt, _alt; // centimetres (this must be signed, right?)
    uint16_t _gdop, _pdop, _hdop, _vdop, _tdop; // * 1e-2
    int32_t _ecef_x, _ecef_y, _ecef_z; // centimetres
    int32_t _ecef_vx, _ecef_vy, _ecef_vz; // centimetres/s

  public:
    Nav_data_msg(unsigned char* payload, Payload_length payload_len);

    GETTER(FixType, fix_type, _fix_type);
    GETTER(uint8_t, num_sv, _num_sv);
    GETTER(uint16_t, week_no, _week_no);
    GETTER(uint32_t, time_of_week, _tow);

    GETTER_MOD(double, lat, _lat * 1e-7);
    GETTER_RAW(int32_t, lat, _lat);

    GETTER_MOD(double, lon, _lon * 1e-7);
    GETTER_RAW(int32_t, lon, _lon);

    GETTER_MOD(double, ellipsoid_alt, _e_alt * 0.01);
    GETTER_RAW(int32_t, ellipsoid_alt, _e_alt);

    GETTER_MOD(double, alt, _alt * 0.01);
    GETTER_RAW(int32_t, alt, _alt);

    GETTER_MOD(double, GDOP, _gdop * 0.01);
    GETTER_RAW(uint16_t, GDOP, _gdop);

    GETTER_MOD(double, PDOP, _pdop * 0.01);
    GETTER_RAW(uint16_t, PDOP, _pdop);

    GETTER_MOD(double, HDOP, _hdop * 0.01);
    GETTER_RAW(uint16_t, HDOP, _hdop);

    GETTER_MOD(double, VDOP, _vdop * 0.01);
    GETTER_RAW(uint16_t, VDOP, _vdop);

    GETTER_MOD(double, TDOP, _tdop * 0.01);
    GETTER_RAW(uint16_t, TDOP, _tdop);

    GETTER_MOD(double, ECEF_X, _ecef_x * 0.01);
    GETTER_RAW(int32_t, ECEF_X, _ecef_x);

    GETTER_MOD(double, ECEF_Y, _ecef_y * 0.01);
    GETTER_RAW(int32_t, ECEF_Y, _ecef_y);

    GETTER_MOD(double, ECEF_Z, _ecef_z * 0.01);
    GETTER_RAW(int32_t, ECEF_Z, _ecef_z);

    GETTER_MOD(double, ECEF_VX, _ecef_vx * 0.01);
    GETTER_RAW(int32_t, ECEF_VX, _ecef_vx);

    GETTER_MOD(double, ECEF_VY, _ecef_vy * 0.01);
    GETTER_RAW(int32_t, ECEF_VY, _ecef_vy);

    GETTER_MOD(double, ECEF_VZ, _ecef_vz * 0.01);
    GETTER_RAW(int32_t, ECEF_VZ, _ecef_vz);

  }; // class Nav_data_msg


  //! GNSS DATUM - datum used by the GNSS receiver
  //! - Answer to Q_datum message
  class GNSS_datum : public Output_message {
  private:
    uint16_t _datum_index;

  public:
    GNSS_datum(unsigned char* payload, Payload_length payload_len);

    GETTER(uint16_t, datum_index, _datum_index);

  }; // class GNSS_datum


  //! GNSS DOP MASK - DOP Mask used by the GNSS receiver
  //! - Answer to Q_DOP_mask message
  class GNSS_DOP_mask : public Output_message {
  private:
    DOPmode _dop_mode;
    uint16_t _pdop, _hdop, _gdop; // * 0.1

  public:
    GNSS_DOP_mask(unsigned char* payload, Payload_length payload_len);

    GETTER(DOPmode, DOP_mode, _dop_mode);

    GETTER_MOD(double, PDOP, _pdop * 0.1);
    GETTER_RAW(uint16_t, PDOP, _pdop);

    GETTER_MOD(double, HDOP, _hdop * 0.1);
    GETTER_RAW(uint16_t, HDOP, _hdop);

    GETTER_MOD(double, GDOP, _gdop * 0.1);
    GETTER_RAW(uint16_t, GDOP, _gdop);

  }; // class GNSS_DOP_mask


  //! GNSS ELEVATION AND CNR MASK - Elevation and CNR mask used by the GNSS receiver
  //! - Answer to Q_elevation_CNR_mask message
  class GNSS_elevation_CNR_mask : public Output_message {
  private:
    ElevationCNRmode _mode_select;
    uint8_t _el_mask, _cnr_mask;

  public:
    GNSS_elevation_CNR_mask(unsigned char* payload, Payload_length payload_len);

    GETTER(ElevationCNRmode, mode_select, _mode_select);
    GETTER(uint8_t, elevation_mask, _el_mask);
    GETTER(uint8_t, CNR_mask, _cnr_mask);

  }; // class GNSS_elevation_CNR_mask


  //! GPS EPHEMERIS DATA - GPS ephemeris data of the GPS receiver
  //! - Answer to Get_GPS_ephemeris message
  class GPS_ephemeris_data : public Output_message {
  private:
    uint16_t _sv_num;
    uint8_t _subframe[3][28];

  public:
    GPS_ephemeris_data(unsigned char* payload, Payload_length payload_len);

    GETTER(uint16_t, SV_num, _sv_num);

    inline const uint8_t* subframe(unsigned char i) const {
      if (i > 2)
	throw std::out_of_range(std::to_string(i) + " > 2");
      return _subframe[i];
    }

  }; // class GPS_ephemeris_data


  //! GNSS POSITON PINNING STATUS - Position pinning status of the GNSS receiver
  //! - Answer to Q_pos_pinning
  class GNSS_pos_pinning_status : public Output_message {
  private:
    DefaultOrEnable _status;
    uint16_t _pin_speed, _pin_count;
    uint16_t _unpin_speed, _unpin_count, _unpin_dist;

  public:
    GNSS_pos_pinning_status(unsigned char* payload, Payload_length payload_len);

    GETTER(DefaultOrEnable, status, _status);
    GETTER(uint16_t, pinning_speed, _pin_speed);
    GETTER(uint16_t, pinning_count, _pin_count);
    GETTER(uint16_t, unpinning_speed, _unpin_speed);
    GETTER(uint16_t, unpinning_count, _unpin_count);
    GETTER(uint16_t, unpinning_distance, _unpin_dist);

  }; // class GNSS_pos_pinning_status


  //! GNSS POWER MODE STATUS - Power mode status of the GNSS receiver
  //! - Answer to Q_power_mode message
  class GNSS_power_mode_status : public Output_message {
  private:
    PowerMode _power_mode;

  public:
    GNSS_power_mode_status(unsigned char* payload, Payload_length payload_len);

    GETTER(PowerMode, power_mode, _power_mode);

  }; // class GNSS_power_mode_status


  //! GNSS 1PPS CABLE DELAY - 1PPS cable delay of the GNSS receiver
  //! - Answer to Q_1PPS_cable_delay
  class GNSS_1PPS_cable_delay : public Output_message {
  private:
    int32_t _delay;

  public:
    GNSS_1PPS_cable_delay(unsigned char* payload, Payload_length payload_len);

    GETTER_MOD(double, delay, _delay * 1.0e-11);
    GETTER_RAW(int32_t, delay, _delay);

  }; // class GNSS_1PPS_cable_delay


  //! MEAS_TIME - Measurement time information (0xDC) (Periodic)
  class Measurement_time : public Output_message {
  private:
    uint8_t _issue;
    uint16_t _weeknumber;
    uint32_t _time_in_week;
    uint16_t _period;

  public:
    //! Constructor from a binary buffer
    Measurement_time(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, issue_of_data, _issue);
    GETTER(uint16_t, week_number, _weeknumber);
    GETTER(uint32_t, time_in_week, _time_in_week);
    GETTER(uint16_t, period, _period);

  }; // class Measurement_time


  struct RawMeasurement {
    uint8_t PRN, CN0;
    double pseudorange;
    double carrier_phase;
    float doppler_freq;
    uint8_t channel_indicator;

    RawMeasurement(uint8_t p, uint8_t c, double pr, double cph, float df, uint8_t ci) :
      PRN(p), CN0(c), pseudorange(pr), carrier_phase(cph), doppler_freq(df), channel_indicator(ci)
    {}
  }; // struct RawMeasurement


  //! RAW_MEAS - Raw measurements from each channel (0xDD) (Periodic)
  class Raw_measurements : public Output_message {
  private:
    uint8_t _issue;
    uint8_t _num_meas;
    std::vector<RawMeasurement> _measurements;

  public:
    //! Constructor from a binary buffer
    Raw_measurements(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, issue_of_data, _issue);
    GETTER(uint8_t, num_measurements, _num_meas);
    GETTER(std::vector<RawMeasurement>, measurements, _measurements);

  }; // class Raw_measurements


  struct SvStatus {
    uint8_t channel_id, PRN, SV_status, URA;
    int8_t CN0;
    int16_t elevation, azimuth;
    uint8_t channel_status;

    SvStatus(uint8_t cid, uint8_t p, uint8_t svs, uint8_t u, int8_t c, int16_t el, int16_t az, uint8_t cs) :
      channel_id(cid), PRN(p), SV_status(svs), URA(u), CN0(c), elevation(el), azimuth(az), channel_status(cs)
    {}
  }; // struct SvStatus


  //! SV_CH_STATUS - SV and channel status (0xDE) (Periodic)
  class SV_channel_status : public Output_message {
  private:
    uint8_t _issue;
    uint8_t _num_sv;
    std::vector<SvStatus> _statuses;

  public:
    //! Constructor from a binary buffer
    SV_channel_status(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, issue_of_data, _issue);
    GETTER(uint8_t, num_svs, _num_sv);
    GETTER(std::vector<SvStatus>, statuses, _statuses);

  };


  //! RCV_STATE - Receiver navigation status
  class Rcv_state : public Output_message {
  private:
    uint8_t _issue;
    NavigationState _nav_state;
    uint16_t _weeknum;
    double _tow, _ecef_x, _ecef_y, _ecef_z;
    float _ecef_vx, _ecef_vy, _ecef_vz;
    double _clock_bias;
    float _clock_drift;
    float _gdop, _pdop, _hdop, _vdop, _tdop;

  public:
    Rcv_state(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, issue_of_data, _issue);
    GETTER(NavigationState, navigation_state, _nav_state);
    GETTER(uint16_t, week_number, _weeknum);
    GETTER(double, time_of_week, _tow);
    GETTER(double, ECEF_X, _ecef_x);
    GETTER(double, ECEF_Y, _ecef_y);
    GETTER(double, ECEF_Z, _ecef_z);
    GETTER(float, ECEF_VX, _ecef_vx);
    GETTER(float, ECEF_VY, _ecef_vy);
    GETTER(float, ECEF_VZ, _ecef_vz);
    GETTER(double, clock_bias, _clock_bias);
    GETTER(float, clock_drift, _clock_drift);
    GETTER(float, GDOP, _gdop);
    GETTER(float, PDOP, _pdop);
    GETTER(float, HDOP, _hdop);
    GETTER(float, VDOP, _vdop);
    GETTER(float, TDOP, _tdop);

  }; // class Rcv_state


  //! SUBFRAME - Sub frame buffer data
  class Subframe_data : public Output_message {
  private:
    uint8_t _prn, _subframe_num;
    uint32_t _words[10];

  public:
    //! Constructor from a binary buffer
    Subframe_data(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, PRN, _prn);
    GETTER(uint8_t, subframe_num, _subframe_num);
    GETTER(uint32_t*, words, _words);
    inline const uint32_t word(int i) const { return _words[i]; }

  }; // class Subframe_data


  /**************************
   * Messages with a sub-ID *
   **************************/


  //! SBAS STATUS - SBAS status of GNSS receiver
  //! - Answer to Q_SBAS_status
  class GNSS_SBAS_status : public Output_message_with_subid {
  private:
    bool _enabled;
    EnableOrAuto _ranging;
    uint8_t _ranging_ura_mask;
    bool _correction;
    uint8_t _num_channels;
    bool _waas, _egnos, _msas;

  public:
    GNSS_SBAS_status(unsigned char* payload, Payload_length payload_len);

    GETTER(bool, enabled, _enabled);
    GETTER(EnableOrAuto, ranging, _ranging);
    GETTER(uint8_t, ranging_URA_mask, _ranging_ura_mask);
    GETTER(bool, correction, _correction);
    GETTER(uint8_t, num_channels, _num_channels);
    GETTER(bool, WAAS_enabled, _waas);
    GETTER(bool, EGNOS_enabled, _egnos);
    GETTER(bool, MSAS_enabled, _msas);

  }; // class GNSS_SBAS_status


  //! QZSS STATUS - QZSS status of GNSS receiver
  class GNSS_QZSS_status : public Output_message_with_subid {
  private:
    bool _enabled;
    uint8_t _num_channels;

  public:
    GNSS_QZSS_status(unsigned char* payload, Payload_length payload_len);

    GETTER(bool, enabled, _enabled);
    GETTER(uint8_t, num_channels, _num_channels);

  }; // class GNSS_QZSS_status


  //! SAEE STATUS - SAEE status of GNSS receiver
  class GNSS_SAEE_status : public Output_message_with_subid {
  private:
    DefaultOrEnable _enabled;

  public:
    GNSS_SAEE_status(unsigned char* payload, Payload_length payload_len);

    GETTER(DefaultOrEnable, enabled, _enabled);


  }; // class GNSS_SAEE_status


  //! GNSS BOOT STATUS - Boot status of GNSS receiver
  //! - Answer to Q_GNSS_boot_status
  class GNSS_boot_status : public Output_message_with_subid {
  private:
    uint8_t _status, _flash_type;

  public:
    GNSS_boot_status(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, status, _status);
    GETTER(uint8_t, flash_type, _flash_type);

  }; // class GNSS_boot_status


  //! 1PPS PULSE WIDTH - 1PPS pulse width of GNSS receiver
  //! - Answer to Q_1PPS_pulse_width
  class GNSS_1PPS_pulse_width : public Output_message_with_subid {
  private:
    uint32_t _width;

  public:
    GNSS_1PPS_pulse_width(unsigned char* payload, Payload_length payload_len);

    GETTER_MOD(double, width, _width * 1.0e-06);
    GETTER_RAW(uint32_t, width, _width);

  }; // class GNSS_1PPS_pulse_width


}; // namespace SkyTraqBin

#endif // __SKYTRAQBIN_OUTPUTS_HH__
