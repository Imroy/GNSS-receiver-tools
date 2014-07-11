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
#include <unistd.h>
#include <string>
#include <cstdio>
#include <string.h>
#include "SkyTraq.hh"
#include "NMEA-0183.hh"
#include "SkyTraqBin.hh"
#include "Parser.hh"

class AppListener : public SkyTraq::Listener {
public:
  void GGA(SkyTraq::Interface* iface, const NMEA0183::GGA &gga) {
    std::cout << "\tTime " << gga.UTC_time()
	      << ", longitude " << gga.longitude() << "°, lattitude " << gga.lattitude() << "°"
	      << ", \"" << gga.fix_quality() << "\""
	      << ", " << gga.num_sats_used() << " satellites used"
	      << ", HDOP " << gga.HDOP()
	      << ", altitude " << gga.altitude() << " m"
	      << std::endl;
  }

  void GLL(SkyTraq::Interface* iface, const NMEA0183::GLL &gll) {
    std::cout << "\tLongitude " << gll.longitude() << "°, lattitude " << gll.lattitude() << "°"
	      << ", time " << gll.UTC_time()
	      << ", " << gll.receiver_mode()
	      << std::endl;
  }

  void GSA(SkyTraq::Interface* iface, const NMEA0183::GSA &gsa) {
    std::cout << "\t" << gsa.mode()
	      << ", fix: " << gsa.fix_type()
	      << ", PDOP " << gsa.PDOP() << ", HDOP " << gsa.HDOP() << ", VDOP " << gsa.VDOP()
	      << std::endl;
    std::cout << "\tSatellites:";
    for (auto sat : gsa.satellite_ids())
      std::cout << " " << sat;
    std::cout << std::endl;
  }

  void GSV(SkyTraq::Interface* iface, const NMEA0183::GSV &gsv) {
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

  void RMC(SkyTraq::Interface* iface, const NMEA0183::RMC &rmc) {
    std::cout << "\tTime " << rmc.UTC_datetime()
	      << ", " << (rmc.status() ? "data valid" : "warning")
	      << ", longitude " << rmc.longitude() << "°, lattitude " << rmc.lattitude() << "°"
	      << ", speed " << rmc.speed() << ", course " << rmc.course()
	      << ", " << rmc.receiver_mode()
	      << std::endl;
  }

  void VTG(SkyTraq::Interface* iface, const NMEA0183::VTG &vtg) {
    std::cout << "\tCourse " << vtg.true_course() << "°"
	      << ", speed " << vtg.speed() << " km/h"
	      << ", " << vtg.receiver_mode()
	      << std::endl;
  }

  void ZDA(SkyTraq::Interface* iface, const NMEA0183::ZDA &zda) {
    std::cout << "\tTime " << zda.UTC_datetime()
	      << ", TZ " << zda.TZ_hours() << ":" << zda.TZ_minutes()
	      << std::endl;
  }

  void STI(SkyTraq::Interface* iface, const NMEA0183::STI &sti) {
    std::cout << "\tProprietary " << sti.proprietary()
	      << ", PPS mode \"" << sti.PPS_mode() << "\""
	      << std::endl;
  }

  void Ack(SkyTraq::Interface* iface, const SkyTraqBin::Ack &ack) {
    std::cout << "\tAcknowledge message id 0x" << std::hex << (int)ack.ack_id();
    if (ack.has_subid())
      std::cout << ", sub id 0x" << (int)ack.ack_subid();
    std::cout << std::dec << std::endl;
  }

  void Nack(SkyTraq::Interface* iface, const SkyTraqBin::Nack &nack) {
    std::cout << "\tNegative acknowledge message id 0x" << std::hex << (int)nack.nack_id();
    if (nack.has_subid())
      std::cout << ", sub id 0x" << (int)nack.nack_subid();
    std::cout << std::dec << std::endl;
  }

  void Measurement_time(SkyTraq::Interface* iface, const SkyTraqBin::Measurement_time &mt) {
    std::cout << "\tMeasurement time, issue of data: " << (int)mt.issue_of_data()
	      << ", week " << mt.week_number()
	      << ", " << mt.time_in_week() << " ms in week"
	      << ", measurement period " << mt.period() << " ms" << std::endl;
  }

  void Raw_measurements(SkyTraq::Interface* iface, const SkyTraqBin::Raw_measurements &rm) {
    std::cout << "\tRaw measurements, issue of data: " << (int)rm.issue_of_data() << ", " << (int)rm.num_measurements() << " raw measurements." << std::endl;
    for (auto m : rm.measurements()) {
      std::cout << "\t\tPRN " << (int)m.PRN
		<< ", CN0 " << (int)m.CN0 << " dBHz";
      if (m.has_pseudo_range)
	std::cout << ", pseudo-range " << m.pseudo_range << " m";
      if (m.has_carrier_phase)
	std::cout << ", carrier phase " << m.carrier_phase << " cycles";
      if (m.has_doppler_freq)
	std::cout << ", doppler " << m.doppler_freq << " Hz";
      std::cout << std::endl;
    }
  }

  void SV_channel_status(SkyTraq::Interface* iface, const SkyTraqBin::SV_channel_status &sv_chan) {
    std::cout << "\tSV channel status, issue of data: " << (int)sv_chan.issue_of_data() << ", " << (int)sv_chan.num_svs() << " SV statuses." << std::endl;
    for (auto s : sv_chan.statuses()) {
      std::cout << "\t\tChannel " << (int)s.channel_id
		<< ", PRN " << (int)s.PRN
		<< ", URA " << (int)s.URA
		<< ", CN0 " << (int)s.CN0 << " dBHz"
		<< ", elevation " << s.elevation << "°"
		<< ", azimuth " << s.azimuth << "°"
		<< std::endl;
  }

  void Subframe_data(SkyTraq::Interface* iface, const SkyTraqBin::Subframe_data &sfd) {
    std::cout << "\tSubframe data, PRN " << (int)sfd.PRN() << ", subframe #" << (int)sfd.subframe_num() << std::endl;
  }

}; // class AppListener

int main(int argc, char* argv[]) {
  std::string filename = "/dev/ttyUSB0";
  SkyTraqBin::UpdateType ut = SkyTraqBin::UpdateType::SRAM;
  SkyTraqBin::MessageType mt;
  int rate;
  bool change_mt = false, change_rate = false;

  if (argc > 1) {
    int opt;
    while ((opt = getopt(argc, argv, "fntbr:")) != -1) {
      switch (opt) {
      case 'f':
	ut = SkyTraqBin::UpdateType::SRAM_and_flash;
	break;
      case 'n':
	mt = SkyTraqBin::MessageType::None;
	change_mt = true;
	break;
      case 't':
	mt = SkyTraqBin::MessageType::NMEA0183;
	change_mt = true;
	break;
      case 'b':
	mt = SkyTraqBin::MessageType::Binary;
	change_mt = true;
	break;
      case 'r':
	rate = atoi(optarg);
	change_rate = true;
	break;
      default:
	break;
      }
    }

    if (optind < argc)
      filename = argv[optind];
  }

  std::FILE *file = fopen(filename.c_str(), "a+");
  if (file == NULL) {
    std::cerr << "input is not open: " << strerror(errno) << std::endl;
    exit(1);
  }

  auto l = std::make_shared<AppListener>();
  SkyTraq::Interface iface(file, l);

  if (change_mt) {
    std::cout << "Switching to " << mt << " message type..." << std::endl;
    iface.send(std::make_shared<SkyTraqBin::Config_msg_type>(mt, ut),
	       [](bool ack, SkyTraqBin::Output_message* msg) {
		 std::cout << (ack ? "A" : "Not a") << "cknowledged message type switch." << std::endl;
	       });
  }
  if (change_rate) {
    std::cout << "Changing output rate to " << rate << " Hz" << std::endl;
    iface.send(std::make_shared<SkyTraqBin::Config_sys_pos_rate>(rate, ut),
	       [](bool ack, SkyTraqBin::Output_message* msg) {
		 std::cout << (ack ? "A" : "Not a") << "cknowledged output rate change." << std::endl;
	       });
  }

  while (1) {
    try {
      iface.read();
    } catch (std::invalid_argument &e) {
      std::cerr << e.what() << std::endl;
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
  }

  return 0;
}
