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
    _ack_id(payload[1]),
    _has_subid(payload_len > 2),
    _ack_subid(payload_len > 2 ? payload[2] : 0)
  {}


  Pos_update_rate::Pos_update_rate(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _update_rate(payload[1])
  {}


  NMEA_talker_id::NMEA_talker_id(unsigned char* payload, Payload_length payload_len) :
    Output_message(payload, payload_len),
    _talker_id((TalkerID)payload[1])
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


}; // namespace SkyTraqBin
