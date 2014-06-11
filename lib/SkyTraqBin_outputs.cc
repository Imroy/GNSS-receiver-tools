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

namespace SkyTraqBin {

  GNSS_boot_status::GNSS_boot_status(unsigned char* payload, Payload_length payload_len) :
    Output_message_with_subid(payload, payload_len),
    _status(payload[2]),
    _flash_type(payload[3])
  {}


  Sw_ver::Sw_ver(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _sw_type((SwType)payload[1]),
    _kernel_ver(payload[3], payload[4], payload[5]),
    _odm_ver(payload[7], payload[8], payload[9]),
    _revision(payload[11], payload[12], payload[13])
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


  NMEA_talker_ID::NMEA_talker_ID(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _talker_id((TalkerID)payload[1])
  {}


  Nav_data_msg::Nav_data_msg(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _fix_type((FixType)payload[1]),
    _num_sv(payload[2]),
    _week_no(extract_be<uint16_t>(payload, 3)),
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


  GNSS_power_mode_status::GNSS_power_mode_status(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _power_mode((PowerMode)payload[1])
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
    for (int i = 3; i <= payload_len - 23; i += 23)
      _measurements.push_back(RawMeasurement(payload[i],
					     payload[i + 1],
					     extract_be<double>(payload, i + 2),
					     extract_be<double>(payload, i + 10),
					     extract_be<float>(payload, i + 18),
					     payload[i + 22]));
  }


  SV_channel_status::SV_channel_status(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _issue(payload[1]),
    _num_sv(payload[2])
  {
    for (int i = 3; i <= payload_len - 10; i += 10)
      _statuses.push_back(SvStatus(payload[i],
				   payload[i + 1],
				   payload[i + 2],
				   payload[i + 3],
				   payload[i + 4],
				   extract_be<int16_t>(payload, i + 5),
				   extract_be<int16_t>(payload, i + 7),
				   payload[i + 9]));
  }


  Subframe_data::Subframe_data(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _prn(payload[1]),
    _subframe_num(payload[2])
  {
    for (int i = 0; i < 10; i++)
      _words[i] = extract_be24(payload, 3 + i * 3);
  }


}; // namespace SkyTraqBin
