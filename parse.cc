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
#include <string>
#include <iostream>
#include "SkyTraq.hh"
#include "NMEA-0183.hh"
#include "SkyTraqBin.hh"

int main(int argc, char* argv[]) {
  unsigned char buffer[16];
  SkyTraq::Parser parser;

  while (1) {
    std::cin.read((char*)buffer, 16);
    parser.add_bytes(buffer, std::cin.gcount());

    try {
      auto messages = parser.parse_messages();

      for (auto msg : messages) {

	try {
	  NMEA0183::Sentence *s = msg->cast_as<NMEA0183::Sentence>();	// Will throw std::bad_cast if not possible
	  std::cout << "Talker id=\"" << s->talker_id() << "\", type=\"" << s->type() << "\"" << std::endl;

	  if (s->isa<NMEA0183::GGA>()) {
	    NMEA0183::GGA *gga = s->cast_as<NMEA0183::GGA>();
	    std::cout << "\tTime " << gga->UTC_time()
		      << ", longitude " << gga->longitude() << "°, lattitude " << gga->lattitude() << "°"
		      << ", \"" << gga->fix_quality() << "\""
		      << ", " << gga->num_sats_used() << " satellites used"
		      << ", HDOP " << gga->HDOP()
		      << ", altitude " << gga->altitude() << " m"
		      << std::endl;

	  } else if (s->isa<NMEA0183::GLL>()) {
	    NMEA0183::GLL *gll = s->cast_as<NMEA0183::GLL>();
	    std::cout << "\tLongitude " << gll->longitude() << "°, lattitude " << gll->lattitude() << "°"
		      << ", time " << gll->UTC_time()
		      << ", " << gll->receiver_mode()
		      << std::endl;

	  } else if (s->isa<NMEA0183::GSA>()) {
	    NMEA0183::GSA *gsa = s->cast_as<NMEA0183::GSA>();
	    std::cout << "\t" << gsa->mode()
		      << ", fix: " << gsa->fix_type()
		      << ", PDOP " << gsa->PDOP() << ", HDOP " << gsa->HDOP() << ", VDOP " << gsa->VDOP()
		      << std::endl;
	    std::cout << "\tSatellites:";
	    for (auto sat : gsa->satellite_ids())
	      std::cout << " " << sat;
	    std::cout << std::endl;

	  } else if (s->isa<NMEA0183::GSV>()) {
	    NMEA0183::GSV *gsv = s->cast_as<NMEA0183::GSV>();
	    std::cout << "\t" << gsv->message_seq() << "/" << gsv->num_messages() << " messages"
		      << ", " << gsv->satellites_in_view() << " satellites in view"
		      << std::endl;
	    std::cout << "\t";
	    for (auto sat : gsv->satellite_data()) {
	      std::cout << sat->id << "{el " << sat->elevation << "°, az " << sat->azimuth << "°";
	      if (sat->snr >= 0)
		std::cout << ", " << sat->snr << " dB";
	      std::cout << "} ";
	    }
	    std::cout << std::endl;

	  } else if (s->isa<NMEA0183::RMC>()) {
	    NMEA0183::RMC *rmc = s->cast_as<NMEA0183::RMC>();
	    std::cout << "\tTime " << rmc->UTC_time()
		      << ", " << (rmc->status() ? "data valid" : "warning")
		      << ", longitude " << rmc->longitude() << "°, lattitude " << rmc->lattitude() << "°"
		      << ", speed " << rmc->speed() << ", course " << rmc->course()
		      << ", date " << rmc->UTC_day() << "/" << rmc->UTC_month() << "/" << rmc->UTC_year()
		      << ", " << rmc->receiver_mode()
		      << std::endl;

	  } else if (s->isa<NMEA0183::VTG>()) {
	    NMEA0183::VTG *vtg = s->cast_as<NMEA0183::VTG>();
	    std::cout << "\tCourse " << vtg->true_course() << "°"
		      << ", speed " << vtg->speed() << " km/h"
		      << ", " << vtg->receiver_mode()
		      << std::endl;

	  } else if (s->isa<NMEA0183::ZDA>()) {
	    NMEA0183::ZDA *zda = s->cast_as<NMEA0183::ZDA>();
	    std::cout << "\tTime " << zda->UTC_time()
		      << ", date " << zda->UTC_day() << "/" << zda->UTC_month() << "/" << zda->UTC_year()
		      << ", TZ " << zda->TZ_hours() << ":" << zda->TZ_minutes()
		      << std::endl;

	  } else if (s->isa<NMEA0183::STI>()) {
	    NMEA0183::STI *sti = s->cast_as<NMEA0183::STI>();
	    std::cout << "\tProprietary " << sti->proprietary()
		      << ", PPS mode \"" << sti->PPS_mode() << "\""
		      << std::endl;
	  }
	} catch (std::bad_cast) {
	}

	try {
	  msg->cast_as<SkyTraqBin::Output_message>();	// throw a std::bas_cast exception if it can't be done

	  if (msg->isa<SkyTraqBin::Measurement_time>()) {
	    SkyTraqBin::Measurement_time *mt = msg->cast_as<SkyTraqBin::Measurement_time>();
	    std::cout << "\tMeasurement time, issue of data: " << (int)mt->issue_of_data()
		      << ", week " << mt->week_number()
		      << ", " << mt->time_in_week() << " ms in week"
		      << ", measurement period " << mt->period() << " ms" << std::endl;

	  } else if (msg->isa<SkyTraqBin::Raw_measurements>()) {
	    SkyTraqBin::Raw_measurements *rm = msg->cast_as<SkyTraqBin::Raw_measurements>();
	    std::cout << "\tRaw measurements, issue of data: " << (int)rm->issue_of_data() << ", " << (int)rm->num_measurements() << " raw measurements." << std::endl;
	    for (auto m : rm->measurements())
	      std::cout << "\t\tPRN " << (int)m.PRN
			<< ", CN0 " << (int)m.CN0 << " dBHz"
			<< ", pseudo-range " << m.pseudorange << " m"
			<< ", carrier phase " << m.carrier_phase << " cycles"
			<< ", doppler " << m.doppler_freq << " Hz"
			<< std::endl;

	  } else if (msg->isa<SkyTraqBin::SV_channel_status>()) {
	    SkyTraqBin::SV_channel_status *sv = msg->cast_as<SkyTraqBin::SV_channel_status>();
	    std::cout << "\tSV channel status, issue of data: " << (int)sv->issue_of_data() << ", " << (int)sv->num_svs() << " SV statuses." << std::endl;
	    for (auto s : sv->statuses())
	      std::cout << "\t\tChannel " << (int)s.channel_id
			<< ", PRN " << (int)s.PRN
			<< ", URA " << (int)s.URA
			<< ", CN0 " << (int)s.CN0 << " dBHz"
			<< ", elevation " << s.elevation << "°"
			<< ", azimuth " << s.azimuth << "°"
			<< std::endl;

	  } else if (msg->isa<SkyTraqBin::Subframe_data>()) {
	    SkyTraqBin::Subframe_data *sd = msg->cast_as<SkyTraqBin::Subframe_data>();
	    std::cout << "\tSubframe data, PRN " << (int)sd->PRN() << ", subframe #" << (int)sd->subframe_num() << std::endl;

	  }
	} catch (std::bad_cast) {
	}

      }
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
  }

  return 0;
}
