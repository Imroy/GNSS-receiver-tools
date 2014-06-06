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

namespace SkyTraqBin {

  enum class FixType : uint8_t {
    None = 0,
      TwoDimensional,
      ThreeDimensional,
      Differential,
  }; // class FixType


  //! GNSS BOOT STATUS - Boot status of GNSS receiver
  class GNSS_boot_status : public Output_message_with_subid {
  private:
    uint8_t _status, _flash_type;

  public:
    GNSS_boot_status(unsigned char* payload, Payload_length payload_len);

    inline const uint8_t status(void) const { return _status; }
    inline const uint8_t flash_type(void) const { return _flash_type; }

  }; // class GNSS_boot_status


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
  class Sw_ver : public Output_message {
  private:
    SwType _sw_type;
    PackedVersion _kernel_ver, _odm_ver;
    PackedDate _revision;

  public:
    //! Constructor from a binary buffer
    Sw_ver(unsigned char* payload, Payload_length payload_len);

    inline const SwType software_type(void) const { return _sw_type; }
    inline PackedVersion kernel_version(void) const { return _kernel_ver; }
    inline const PackedVersion ODM_version(void) const { return _odm_ver; }
    inline PackedDate revision(void) const { return _revision; }

  }; // class Sw_ver


  //! SOFTWARE CRC -Software CRC of the GNSS receiver
  class Sw_CRC : public Output_message {
  private:
    SwType _sw_type;
    uint16_t _crc;

  public:
    //! Constructor from a binary buffer
    Sw_CRC(unsigned char* payload, Payload_length payload_len);

    inline const SwType software_type(void) const { return _sw_type; }
    inline uint16_t CRC(void) const { return _crc; }

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

    inline const uint8_t ack_id(void) const { return _ack_id; }
    inline const bool has_subid(void) const { return _has_subid; }
    inline const uint8_t ack_subid(void) const { return _ack_subid; }

  }; // class Ack


  //! NACK - Response to an unsuccessful request message
  class Nack : public Output_message {
  private:
    uint8_t _ack_id;
    bool _has_subid;
    uint8_t _ack_subid;

  public:
    //! Constructor from a binary buffer
    Nack(unsigned char* payload, Payload_length payload_len);

    inline const uint8_t ack_id(void) const { return _ack_id; }
    inline const bool has_subid(void) const { return _has_subid; }
    inline const uint8_t ack_subid(void) const { return _ack_subid; }

  }; // class Ack


  //! POSITON UPDATE RATE - Position Update rate of the GNSS system
  class Pos_update_rate : public Output_message {
  private:
    uint8_t _update_rate;

  public:
    //! Constructor from a binary buffer
    Pos_update_rate(unsigned char* payload, Payload_length payload_len);

    inline const uint8_t update_rate(void) const { return _update_rate; }

  }; // class Pos_update_rate


  //! GNSS NMEA TALKER ID - NMEA talker ID of GNSS receiver
  class NMEA_talker_ID : public Output_message {
  private:
    TalkerID _talker_id;

  public:
    //! Constructor from a binary buffer
    NMEA_talker_ID(unsigned char* payload, Payload_length payload_len);

    inline const TalkerID talker_id(void) const { return _talker_id; }

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

    inline const FixType fix_type(void) const { return _fix_type; }
    inline const uint8_t num_sv(void) const { return _num_sv; }
    inline const uint16_t week_no(void) const { return _week_no; }
    inline const uint32_t time_of_week(void) const { return _tow; }

    inline const double lat(void) const { return _lat * 1e-7; }
    inline const int32_t lat_raw(void) const { return _lat; }

    inline const double lon(void) const { return _lon * 1e-7; }
    inline const int32_t lon_raw(void) const { return _lon; }

    inline const double ellipsoid_alt(void) const { return _e_alt * 0.01; }
    inline const int32_t ellipsoid_alt_raw(void) const { return _e_alt; }

    inline const double alt(void) const { return _alt * 0.01; }
    inline const int32_t alt_raw(void) const { return _alt; }

    inline const double GDOP(void) const { return _gdop * 0.01; }
    inline const uint16_t GDOP_raw(void) const { return _gdop; }

    inline const double PDOP(void) const { return _pdop * 0.01; }
    inline const uint16_t PDOP_raw(void) const { return _pdop; }

    inline const double HDOP(void) const { return _hdop * 0.01; }
    inline const uint16_t HDOP_raw(void) const { return _hdop; }

    inline const double VDOP(void) const { return _vdop * 0.01; }
    inline const uint16_t VDOP_raw(void) const { return _vdop; }

    inline const double TDOP(void) const { return _tdop * 0.01; }
    inline const uint16_t TDOP_raw(void) const { return _tdop; }

    inline const double ECEF_X(void) const { return _ecef_x * 0.01; }
    inline const int32_t ECEF_X_raw(void) const { return _ecef_x; }

    inline const double ECEF_Y(void) const { return _ecef_y * 0.01; }
    inline const int32_t ECEF_Y_raw(void) const { return _ecef_y; }

    inline const double ECEF_Z(void) const { return _ecef_z * 0.01; }
    inline const int32_t ECEF_Z_raw(void) const { return _ecef_z; }

    inline const double ECEF_VX(void) const { return _ecef_vx * 0.01; }
    inline const int32_t ECEF_VX_raw(void) const { return _ecef_vx; }

    inline const double ECEF_VY(void) const { return _ecef_vy * 0.01; }
    inline const int32_t ECEF_VY_raw(void) const { return _ecef_vy; }

    inline const double ECEF_VZ(void) const { return _ecef_vz * 0.01; }
    inline const int32_t ECEF_VZ_raw(void) const { return _ecef_vz; }

  }; // class Nav_data_msg


  //! GNSS DATUM - datum used by the GNSS receiver
  class GNSS_datum : public Output_message {
  private:
    uint16_t _datum_index;

  public:
    GNSS_datum(unsigned char* payload, Payload_length payload_len);

    inline const uint16_t datum_index(void) const { return _datum_index; }

  }; // class GNSS_datum


  //! GNSS POWER MODE STATUS - Power mode status of the GNSS receiver
  class GNSS_power_mode_status : public Output_message {
  private:
    PowerMode _power_mode;

  public:
    GNSS_power_mode_status(unsigned char* payload, Payload_length payload_len);

    inline const PowerMode power_mode(void) const { return _power_mode; }

  }; // class GNSS_power_mode_status


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

    inline const uint8_t issue_of_data(void) const { return _issue; }
    inline const uint16_t week_number(void) const { return _weeknumber; }
    inline const uint32_t time_in_week(void) const { return _time_in_week; }
    inline const uint16_t period(void) const { return _period; }

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

    inline const uint8_t issue_of_data(void) const { return _issue; }
    inline const uint8_t num_measurements(void) const { return _num_meas; }
    inline const std::vector<RawMeasurement> measurements(void) const { return _measurements; }

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

    inline const uint8_t issue_of_data(void) const { return _issue; }
    inline const uint8_t num_svs(void) const { return _num_sv; }
    inline const std::vector<SvStatus> statuses(void) const { return _statuses; }

  };


}; // namespace SkyTraqBin

#endif // __SKYTRAQBIN_OUTPUTS_HH__
