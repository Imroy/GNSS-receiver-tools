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
#include "SkyTraqBin.hh"
#include "BE.hh"

/*
  Sources:
  https://store-lgdi92x.mybigcommerce.com/content/AN0028_1.4.31.pdf	(Binary messages of Skytraq Venus 8)
  https://store-lgdi92x.mybigcommerce.com/content/AN0024_v07.pdf	(Raw measurement binary messages of Skytraq 6 & 8)
  https://store-lgdi92x.mybigcommerce.com/content/SUP800F_v0.6.pdf	(Skytraq SUP800F datasheet)
*/

namespace SkyTraqBin {

  Sw_ver::Sw_ver(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _sw_type((SwType)payload[1]),
    _kernel_ver(payload[3], payload[4], payload[5]),
    _odm_ver(payload[7], payload[8], payload[9]),
    _revision(greg::date(payload[11] + 1900, payload[12], payload[13]))
  {}


  Sw_CRC::Sw_CRC(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _sw_type((SwType)payload[1]),
    _crc(extract_be<uint16_t>(payload, 2))
  {}


  Ack::Ack(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _ack_id(payload[1]),
    _has_subid(payload_len > 2),
    _ack_subid(payload_len > 2 ? payload[2] : 0)
  {}


  Nack::Nack(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _nack_id(payload[1]),
    _has_subid(payload_len > 2),
    _nack_subid(payload_len > 2 ? payload[2] : 0)
  {}


  Pos_update_rate::Pos_update_rate(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _update_rate(payload[1])
  {}


  GPS_almanac_data::GPS_almanac_data(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _prn(payload[1]),
    _week_num(extract_be<int16_t>(payload, 26))
  {
    for (int i = 0; i < 8; i++)
      _words[i] = extract_be24(payload, 2 + i * 3);
  }


  NMEA_talker_ID::NMEA_talker_ID(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _talker_id((TalkerID)payload[1])
  {}


  Nav_data_msg::Nav_data_msg(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _fix_type((FixType)payload[1]),
    _num_sv(payload[2]),
    _week_num(extract_be<uint16_t>(payload, 3)),
    _tow(extract_be<uint32_t>(payload, 5)),
    _lat(extract_be<int32_t>(payload, 9)), _lon(extract_be<int32_t>(payload, 13)),
    _e_alt(extract_be<int32_t>(payload, 17)), _alt(extract_be<int32_t>(payload, 21)),
    _gdop(extract_be<uint16_t>(payload, 25)), _pdop(extract_be<uint16_t>(payload, 27)),
    _hdop(extract_be<uint16_t>(payload, 29)), _vdop(extract_be<uint16_t>(payload, 31)),
    _tdop(extract_be<uint16_t>(payload, 33)),
    _ecef_x(extract_be<int32_t>(payload, 35)), _ecef_y(extract_be<int32_t>(payload, 39)), _ecef_z(extract_be<int32_t>(payload, 43)),
    _ecef_vx(extract_be<int32_t>(payload, 47)), _ecef_vy(extract_be<int32_t>(payload, 51)), _ecef_vz(extract_be<int32_t>(payload, 55))
  {}


  GNSS_datum::GNSS_datum(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _datum_index(extract_be<uint16_t>(payload, 1))
  {}


  GNSS_DOP_mask::GNSS_DOP_mask(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _dop_mode((DOPmode)payload[1]),
    _pdop(extract_be<int16_t>(payload, 2)),
    _hdop(extract_be<int16_t>(payload, 4)),
    _gdop(extract_be<int16_t>(payload, 6))
  {}


  GNSS_elevation_CNR_mask::GNSS_elevation_CNR_mask(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _mode_select((ElevationCNRmode)payload[1]),
    _el_mask(payload[2]),
    _cnr_mask(payload[3])
  {}


  GPS_ephemeris_data::GPS_ephemeris_data(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _sv_num(extract_be<uint16_t>(payload, 1))
  {
    for (unsigned char i = 0; i < 3; i++)
      memcpy(_subframe[i], payload + 3 + (i * 28), 28);
  }


  GNSS_pos_pinning_status::GNSS_pos_pinning_status(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _status((DefaultOrEnable)payload[1]),
    _pin_speed(extract_be<uint16_t>(payload, 2)),
    _pin_count(extract_be<uint16_t>(payload, 4)),
    _unpin_speed(extract_be<uint16_t>(payload, 6)),
    _unpin_count(extract_be<uint16_t>(payload, 8)),
    _unpin_dist(extract_be<uint16_t>(payload, 10))
  {}


  GNSS_power_mode_status::GNSS_power_mode_status(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _power_mode((PowerMode)payload[1])
  {}


  GNSS_1PPS_cable_delay::GNSS_1PPS_cable_delay(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _delay(extract_be<int32_t>(payload, 1))
  {}


  Measurement_time::Measurement_time(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _issue(payload[1]),
    _weeknumber(extract_be<uint16_t>(payload, 2)),
    _time_in_week(extract_be<uint32_t>(payload, 4)),
    _period(extract_be<uint16_t>(payload, 8))
  {}


  Raw_measurements::Raw_measurements(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _issue(payload[1]),
    _num_meas(payload[2])
  {
    uint8_t ci;
    for (int i = 3; i <= payload_len - 23; i += 23) {
      ci = payload[i + 22];
      _measurements.push_back(RawMeasurement(payload[i],
					     payload[i + 1],
					     extract_be<double>(payload, i + 2),
					     extract_be<double>(payload, i + 10),
					     extract_be<float>(payload, i + 18),
					     ci & 0x01, ci & 0x02, ci & 0x04, ci & 0x08, ci & 0x10));
    }
  }


  SV_channel_status::SV_channel_status(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _issue(payload[1]),
    _num_sv(payload[2])
  {
    uint8_t sv_status, chan_status;
    for (int i = 3; i <= payload_len - 10; i += 10) {
      sv_status = payload[i + 2];
      chan_status = payload[i + 9];
      _statuses.push_back(SvStatus(payload[i],
				   payload[i + 1],
				   sv_status & 0x01, sv_status & 0x02, sv_status & 0x04,
				   payload[i + 3],
				   payload[i + 4],
				   extract_be<int16_t>(payload, i + 5),
				   extract_be<int16_t>(payload, i + 7),
				   chan_status & 0x01, chan_status & 0x02,
				   chan_status & 0x04, chan_status & 0x08,
				   chan_status & 0x10, chan_status & 0x20));
    }
  }


  Rcv_state::Rcv_state(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _issue(payload[1]),
    _nav_state((NavigationState)payload[2]),
    _weeknum(extract_be<uint16_t>(payload, 3)),
    _tow(extract_be<double>(payload, 5)),
    _ecef_x(extract_be<double>(payload, 13)),
    _ecef_y(extract_be<double>(payload, 21)),
    _ecef_z(extract_be<double>(payload, 29)),
    _ecef_vx(extract_be<float>(payload, 37)),
    _ecef_vy(extract_be<float>(payload, 41)),
    _ecef_vz(extract_be<float>(payload, 45)),
    _clock_bias(extract_be<double>(payload, 49)),
    _clock_drift(extract_be<float>(payload, 57)),
    _gdop(extract_be<float>(payload, 61)),
    _pdop(extract_be<float>(payload, 65)),
    _hdop(extract_be<float>(payload, 69)),
    _vdop(extract_be<float>(payload, 73)),
    _tdop(extract_be<float>(payload, 77))
  {}


  Subframe_data::Subframe_data(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _prn(payload[1]),
    _subframe_num(payload[2])
  {
    for (int i = 0; i < 30; i++)
      _bytes[i] = extract_be<unsigned char>(payload, 3 + i);
  }


  /**************************
   * Messages with a sub-ID *
   **************************/


  GNSS_SBAS_status::GNSS_SBAS_status(unsigned char* payload, Payload_length payload_len) :
    Output_message_with_subid(payload, payload_len),
    _enabled((bool)payload[2]),
    _ranging((EnableOrAuto)payload[3]),
    _ranging_ura_mask(payload[4]),
    _correction((bool)payload[5]),
    _num_channels(payload[6])
  {
    uint8_t sub_mask = payload[7];
    _waas = sub_mask & 0x01;
    _egnos = sub_mask & 0x02;
    _msas = sub_mask & 0x04;
  }


  GNSS_QZSS_status::GNSS_QZSS_status(unsigned char* payload, Payload_length payload_len) :
    Output_message_with_subid(payload, payload_len),
    _enabled((bool)payload[2]),
    _num_channels(payload[3])
  {}


  GNSS_SAEE_status::GNSS_SAEE_status(unsigned char* payload, Payload_length payload_len) :
    Output_message_with_subid(payload, payload_len),
    _enabled((DefaultOrEnable)payload[2])
  {}


  GNSS_boot_status::GNSS_boot_status(unsigned char* payload, Payload_length payload_len) :
    Output_message_with_subid(payload, payload_len),
    _status((BootStatus)payload[2])
  {
    uint8_t flash_type = payload[3];
    _winbond = flash_type & 0x01;
    _eon = flash_type & 0x02;
    _parallel = flash_type & 0x04;
  }


  GNSS_extended_NMEA_msg_interval::GNSS_extended_NMEA_msg_interval(unsigned char* payload, Payload_length payload_len) :
    Output_message_with_subid(payload, payload_len),
    _gga(payload[2]), _gsa(payload[3]), _gsv(payload[4]), _gll(payload[5]),
    _rmc(payload[6]), _vtg(payload[7]), _zda(payload[8]), _gns(payload[9]),
    _gbs(payload[10]), _grs(payload[11]), _dtm(payload[12]), _gst(payload[13])
  {}


  GNSS_interference_detection_status::GNSS_interference_detection_status(unsigned char* payload, Payload_length payload_len) :
    Output_message_with_subid(payload, payload_len),
    _enabled((bool)payload[2]),
    _status((InterferenceStatus)payload[3])
  {}


  GNSS_nav_mode::GNSS_nav_mode(unsigned char* payload, Payload_length payload_len) :
    Output_message_with_subid(payload, payload_len),
    _mode((NavigationMode)payload[2])
  {}


  GNSS_constellation_type::GNSS_constellation_type(unsigned char* payload, Payload_length payload_len) :
    Output_message_with_subid(payload, payload_len)
  {
    uint16_t mode = extract_be<uint16_t>(payload, 2);
    _gps = mode & 0x0001;
    _glonass = mode & 0x0002;
    _galileo = mode & 0x0004;
    _beidou = mode & 0x0008;
  }


  GNSS_time::GNSS_time(unsigned char* payload, Payload_length payload_len) :
    Output_message_with_subid(payload, payload_len),
    _tow_ms(extract_be<uint32_t>(payload, 2)),
    _tow_ns(extract_be<uint32_t>(payload, 6)),
    _week_num(extract_be<uint16_t>(payload, 10)),
    _def_leap_secs(payload[12]),
    _curr_leap_secs(payload[13])
  {
    uint8_t valid = payload[14];
    _tow_valid = valid & 0x01;
    _wn_valid = valid & 0x02;
    _ls_valid = valid & 0x04;
  }

  GNSS_1PPS_pulse_width::GNSS_1PPS_pulse_width(unsigned char* payload, Payload_length payload_len) :
    Output_message_with_subid(payload, payload_len),
    _width(extract_be<uint32_t>(payload, 2))
  {}


  Sensor_data::Sensor_data(unsigned char* payload, Payload_length payload_len) :
    Output_message_with_subid(payload, payload_len),
    _gx(extract_be<float>(payload, 2)),
    _gy(extract_be<float>(payload, 6)),
    _gz(extract_be<float>(payload, 10)),
    _mx(extract_be<float>(payload, 14)),
    _my(extract_be<float>(payload, 18)),
    _mz(extract_be<float>(payload, 22)),
    _pres(extract_be<uint32_t>(payload, 26)),
    _temp(extract_be<float>(payload, 30))
  {}


}; // namespace SkyTraqBin
