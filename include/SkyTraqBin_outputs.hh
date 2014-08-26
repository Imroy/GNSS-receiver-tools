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

#include <typeinfo>
#include <type_traits>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "SkyTraqBin.hh"

namespace greg = boost::gregorian;

/*
  Sources:
  https://store-lgdi92x.mybigcommerce.com/content/AN0028_1.4.31.pdf	(Binary messages of Skytraq Venus 8)
  https://store-lgdi92x.mybigcommerce.com/content/AN0024_v07.pdf	(Raw measurement binary messages of Skytraq 6 & 8)
  https://store-lgdi92x.mybigcommerce.com/content/SUP800F_v0.6.pdf	(Skytraq SUP800F datasheet)
  https://store-lgdi92x.mybigcommerce.com/content/AN0008_v1.4.17.pdf    (Datalogging extension for Venus 8)
*/

namespace SkyTraqBin {

  /* All output message class names shall start with a noun

     Common words shortened:
      configure => config
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

  struct PackedVersion {
    uint8_t X, Y, Z;

    inline PackedVersion(uint8_t x, uint8_t y, uint8_t z) :
      X(x), Y(y), Z(z)
    {}
  }; // struct PackedVersion


  //! SOFTWARE VERSION - Software version of the GNSS receiver
  /*! Answer to Q_sw_ver message */
  class Sw_ver : public Output_message {
  private:
    SwType _sw_type;
    PackedVersion _kernel_ver, _odm_ver;
    greg::date _revision;

  public:
    //! Constructor from a binary buffer
    Sw_ver(unsigned char* payload, Payload_length payload_len);

    GETTER(SwType, software_type, _sw_type);
    GETTER(PackedVersion, kernel_version, _kernel_ver);
    GETTER(PackedVersion, ODM_version, _odm_ver);
    GETTER(greg::date, revision, _revision);

  }; // class Sw_ver


  //! SOFTWARE CRC -Software CRC of the GNSS receiver
  /*! Answer to Q_sw_CRC message */
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
  /*! Answer to Q_pos_update_rate message */
  class Pos_update_rate : public Output_message {
  private:
    uint8_t _update_rate;

  public:
    //! Constructor from a binary buffer
    Pos_update_rate(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, update_rate, _update_rate);

  }; // class Pos_update_rate


  //! GPS ALMANAC Data - almanac data of GPS receiver
  /*! Answer to Get_almanac, only on NavSpark-Raw boards */
  class GPS_almanac_data : public Output_message {
  private:
    uint8_t _prn;
    uint32_t _words[8];
    int16_t _week_num;

  public:
    GPS_almanac_data(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, PRN, _prn);
    GETTER(uint32_t*, words, _words);
    inline const uint32_t word(int i) const { return _words[i+3]; }
    GETTER(int16_t, week_number, _week_num);

  }; // class GPS_almanac_data


  //! GNSS NMEA TALKER ID - NMEA talker ID of GNSS receiver
  /*! Answer to Q_NMEA_talker_ID message */
  class NMEA_talker_ID : public Output_message {
  private:
    TalkerID _talker_id;

  public:
    //! Constructor from a binary buffer
    NMEA_talker_ID(unsigned char* payload, Payload_length payload_len);

    GETTER(TalkerID, talker_id, _talker_id);

  }; //class NMEA_talker_ID


  //! LOG STATUS OUTPUT - Output Status of the Log Buffer
  class Log_status_output : public Output_message {
  private:
    uint32_t _wr_ptr;
    uint16_t _sectors_left, _total_sectors;
    uint32_t _max_time, _min_time, _max_dist, _min_dist, _max_speed, _min_speed;
    bool _datalog;
    uint8_t _fifo_mode;

  public:
    Log_status_output(unsigned char* payload, Payload_length payload_len);

    GETTER(uint32_t, write_pointer, _wr_ptr);
    GETTER(uint16_t, sectors_left, _sectors_left);
    GETTER(uint16_t, total_sectors, _total_sectors);

    GETTER(uint32_t, max_time, _max_time);
    GETTER(uint32_t, min_time, _min_time);
    GETTER(uint32_t, max_distance, _max_dist);
    GETTER(uint32_t, min_distance, _min_dist);
    GETTER(uint32_t, max_speed, _max_speed);
    GETTER(uint32_t, min_speed, _min_speed);
    GETTER(bool, datalog, _datalog);
    GETTER(uint8_t, fifo_mode, _fifo_mode);

  }; // class Log_status_output


  //! NAVIGATION DATA MESSAGE - Message of user navigation data in binary format
  class Nav_data_msg : public Output_message {
  private:
    FixType _fix_type;
    uint8_t _num_sv;
    uint16_t _week_num;
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
    GETTER(uint16_t, week_number, _week_num);

    GETTER_MOD(double, time_of_week, _tow * 1e-02);
    GETTER_RAW(uint32_t, time_of_week, _tow);

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
  /*! Answer to Q_datum message */
  class GNSS_datum : public Output_message {
  private:
    uint16_t _datum_index;

  public:
    GNSS_datum(unsigned char* payload, Payload_length payload_len);

    GETTER(uint16_t, datum_index, _datum_index);

  }; // class GNSS_datum


  //! GNSS DOP MASK - DOP Mask used by the GNSS receiver
  /*! Answer to Q_DOP_mask message */
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
  /*! Answer to Q_elevation_CNR_mask message */
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
  /*! Answer to Get_GPS_ephemeris message */
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
  /*! Answer to Q_pos_pinning */
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
  /*! Answer to Q_power_mode message */
  class GNSS_power_mode_status : public Output_message {
  private:
    PowerMode _power_mode;

  public:
    GNSS_power_mode_status(unsigned char* payload, Payload_length payload_len);

    GETTER(PowerMode, power_mode, _power_mode);

  }; // class GNSS_power_mode_status


  //! GNSS 1PPS CABLE DELAY - 1PPS cable delay of the GNSS receiver
  /*! Answer to Q_1PPS_cable_delay */
  class GNSS_1PPS_cable_delay : public Output_message {
  private:
    int32_t _delay;

  public:
    GNSS_1PPS_cable_delay(unsigned char* payload, Payload_length payload_len);

    GETTER_MOD(double, delay, _delay * 1.0e-11);
    GETTER_RAW(int32_t, delay, _delay);

  }; // class GNSS_1PPS_cable_delay


  //! GNSS 1PPS TIMING - 1PPS timing information of the GNSS receiver
  /*! Answer to Q_1PPS_timing */
  class GNSS_1PPS_timing : public Output_message {
  private:
    SkyTraq::TimingMode _timing_mode;
    uint32_t _survey_len, _std_dev;
    double _lat, _lon;
    float _alt;
    SkyTraq::TimingMode _curr_timing_mode;
    uint32_t _curr_survey_len;

  public:
    GNSS_1PPS_timing(unsigned char* payload, Payload_length payload_len);

    GETTER(SkyTraq::TimingMode, saved_timing_mode, _timing_mode);
    GETTER(uint32_t, saved_survey_length, _survey_len);
    GETTER(uint32_t, saved_standard_deviation, _std_dev);
    GETTER(double, saved_lattitude, _lat);
    GETTER(double, saved_longitude, _lon);
    GETTER(float, saved_altitude, _alt);
    GETTER(SkyTraq::TimingMode, current_timing_mode, _curr_timing_mode);
    GETTER(uint32_t, current_survey_length, _curr_survey_len);

  }; // class GNSS_1PPS_timing


  //! MEAS_TIME - Measurement time information
  /*! Periodic message only on NavSpark-Raws */
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
    double pseudo_range;
    double carrier_phase;
    float doppler_freq;
    bool has_pseudo_range, has_doppler_freq, has_carrier_phase, will_cycle_slip, coherent_integration_time;

    RawMeasurement(uint8_t p, uint8_t c, double pr, double cph, float df, bool hpr, bool hdf, bool hcp, bool wcs, bool cit) :
      PRN(p), CN0(c), pseudo_range(pr), carrier_phase(cph), doppler_freq(df),
      has_pseudo_range(hpr), has_doppler_freq(hdf), has_carrier_phase(hcp), will_cycle_slip(wcs), coherent_integration_time(cit)
    {}
  }; // struct RawMeasurement


  //! RAW_MEAS - Raw measurements from each channel
  /*! Periodic message only on NavSpark-Raws */
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
    const RawMeasurement measurement(uint8_t i) const { return _measurements[i]; }

  }; // class Raw_measurements


  struct SvStatus {
    uint8_t channel_id, PRN;
    bool sv_has_almanac, sv_has_ephemeris, sv_is_healthy;
    uint8_t URA;
    int8_t CN0;
    int16_t elevation, azimuth;
    bool chan_has_pull_in, chan_has_bit_sync, chan_has_frame_sync, chan_has_ephemeris, chan_for_normal, chan_for_differential;

    SvStatus(uint8_t cid, uint8_t p, bool sha, bool she, bool sih, uint8_t u, int8_t c, int16_t el, int16_t az, bool chpi, bool chbs, bool chfs, bool che, bool cfn, bool cfd) :
      channel_id(cid), PRN(p),
      sv_has_almanac(sha), sv_has_ephemeris(she), sv_is_healthy(sih),
      URA(u), CN0(c), elevation(el), azimuth(az),
      chan_has_pull_in(chpi), chan_has_bit_sync(chbs), chan_has_frame_sync(chfs), chan_has_ephemeris(che), chan_for_normal(cfn), chan_for_differential(cfd)
    {}
  }; // struct SvStatus


  //! SV_CH_STATUS - SV and channel status
  /*! Periodic message only on NavSpark-Raws */
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
    const SvStatus status(uint8_t i) const { return _statuses[i]; }

  };


  //! RCV_STATE - Receiver navigation status
  /*! Periodic message only on NavSpark-Raws */
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
  /*! Periodic message only on NavSpark-Raws */
  class Subframe_data : public Output_message {
  private:
    uint8_t _prn, _subframe_num;
    uint8_t _bytes[30];

  public:
    //! Constructor from a binary buffer
    Subframe_data(unsigned char* payload, Payload_length payload_len);

    GETTER(uint8_t, PRN, _prn);
    GETTER(uint8_t, subframe_num, _subframe_num);
    GETTER(uint8_t*, bytes, _bytes);
    inline const uint8_t byte(int i) const {
      if ((i < 0) || (i >= 30))
	throw std::out_of_range("Can only access byte 0~29");
      return _bytes[i];
    }

    //! Extract arbitrary bits from the subframe buffer
    /*!
      \param start Bit index to start at
      \param len Number of bits to extract, defaults to size of template argument
     */
    template<typename T>
    const T data(uint8_t start, uint8_t len = sizeof(T) * 8) const {
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
	  ret |= (_bytes[from_byte] & mask) << -shift;
	else
	  ret |= (_bytes[from_byte] & mask) >> shift;
      }

      return ret;
    }

  }; // class Subframe_data


}; // namespace SkyTraqBin

#endif // __SKYTRAQBIN_OUTPUTS_HH__
