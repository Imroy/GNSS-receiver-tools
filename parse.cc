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
#include "Parser.hh"

class AppListener : public SkyTraq::Listener {
public:
  void GGA(const NMEA0183::GGA &gga) {
    std::cout << "\tTime " << gga.UTC_time()
	      << ", longitude " << gga.longitude() << "°, lattitude " << gga.lattitude() << "°"
	      << ", \"" << gga.fix_quality() << "\""
	      << ", " << gga.num_sats_used() << " satellites used"
	      << ", HDOP " << gga.HDOP()
	      << ", altitude " << gga.altitude() << " m"
	      << std::endl;
  }

  void GLL(const NMEA0183::GLL &gll) {
    std::cout << "\tLongitude " << gll.longitude() << "°, lattitude " << gll.lattitude() << "°"
	      << ", time " << gll.UTC_time()
	      << ", " << gll.receiver_mode()
	      << std::endl;
  }

  void GSA(const NMEA0183::GSA &gsa) {
    std::cout << "\t" << gsa.mode()
	      << ", fix: " << gsa.fix_type()
	      << ", PDOP " << gsa.PDOP() << ", HDOP " << gsa.HDOP() << ", VDOP " << gsa.VDOP()
	      << std::endl;
    std::cout << "\tSatellites:";
    for (auto sat : gsa.satellite_ids())
      std::cout << " " << sat;
    std::cout << std::endl;
  }

  void GSV(const NMEA0183::GSV &gsv) {
    std::cout << "\t" << gsv.message_seq() << "/" << gsv.num_messages() << " messages"
	      << ", " << gsv.satellites_in_view() << " satellites in view"
	      << std::endl;
    std::cout << "\t";
    for (auto sat : gsv.satellite_data()) {
      std::cout << sat->id << "{el " << sat->elevation << "°, az " << sat->azimuth << "°";
      if (sat->snr >= 0)
	std::cout << ", " << sat->snr << " dB";
      std::cout << "} ";
    }
    std::cout << std::endl;
  }

  void RMC(const NMEA0183::RMC &rmc) {
    std::cout << "\tTime " << rmc.UTC_time()
	      << ", " << (rmc.status() ? "data valid" : "warning")
	      << ", longitude " << rmc.longitude() << "°, lattitude " << rmc.lattitude() << "°"
	      << ", speed " << rmc.speed() << ", course " << rmc.course()
	      << ", date " << rmc.UTC_day() << "/" << rmc.UTC_month() << "/" << rmc.UTC_year()
	      << ", " << rmc.receiver_mode()
	      << std::endl;
  }

  void VTG(const NMEA0183::VTG &vtg) {
    std::cout << "\tCourse " << vtg.true_course() << "°"
	      << ", speed " << vtg.speed() << " km/h"
	      << ", " << vtg.receiver_mode()
	      << std::endl;
  }

  void ZDA(const NMEA0183::ZDA &zda) {
    std::cout << "\tTime " << zda.UTC_time()
	      << ", date " << zda.UTC_day() << "/" << zda.UTC_month() << "/" << zda.UTC_year()
	      << ", TZ " << zda.TZ_hours() << ":" << zda.TZ_minutes()
	      << std::endl;
  }

  void STI(const NMEA0183::STI &sti) {
    std::cout << "\tProprietary " << sti.proprietary()
	      << ", PPS mode \"" << sti.PPS_mode() << "\""
	      << std::endl;
  }

  void Measurement_time(const SkyTraqBin::Measurement_time &mt) {
    std::cout << "\tMeasurement time, issue of data: " << (int)mt.issue_of_data()
	      << ", week " << mt.week_number()
	      << ", " << mt.time_in_week() << " ms in week"
	      << ", measurement period " << mt.period() << " ms" << std::endl;
  }

  void Raw_measurements(const SkyTraqBin::Raw_measurements &rm) {
    std::cout << "\tRaw measurements, issue of data: " << (int)rm.issue_of_data() << ", " << (int)rm.num_measurements() << " raw measurements." << std::endl;
    for (auto m : rm.measurements())
      std::cout << "\t\tPRN " << (int)m.PRN
		<< ", CN0 " << (int)m.CN0 << " dBHz"
		<< ", pseudo-range " << m.pseudorange << " m"
		<< ", carrier phase " << m.carrier_phase << " cycles"
		<< ", doppler " << m.doppler_freq << " Hz"
		<< std::endl;
  }

  void SV_channel_status(const SkyTraqBin::SV_channel_status &sv_chan) {
    std::cout << "\tSV channel status, issue of data: " << (int)sv_chan.issue_of_data() << ", " << (int)sv_chan.num_svs() << " SV statuses." << std::endl;
    for (auto s : sv_chan.statuses())
      std::cout << "\t\tChannel " << (int)s.channel_id
		<< ", PRN " << (int)s.PRN
		<< ", URA " << (int)s.URA
		<< ", CN0 " << (int)s.CN0 << " dBHz"
		<< ", elevation " << s.elevation << "°"
		<< ", azimuth " << s.azimuth << "°"
		<< std::endl;
  }

  void Subframe_data(const SkyTraqBin::Subframe_data &sfd) {
    std::cout << "\tSubframe data, PRN " << (int)sfd.PRN() << ", subframe #" << (int)sfd.subframe_num() << std::endl;
  }

}; // class AppListener

int main(int argc, char* argv[]) {
  auto l = std::make_shared<AppListener>();
  SkyTraq::Reader r(std::cin, l);

  while (1) {
    r.read();
  }

  return 0;
}
