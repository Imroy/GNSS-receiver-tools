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
#include <unistd.h>
#include "SkyTraqBin.hh"

int main(int argc, char* argv[]) {
  unsigned char buffer[16];
  while (1) {
    std::cin.read((char*)buffer, 16);
    try {
      std::vector<SkyTraqBin::Output_message::ptr> messages = SkyTraqBin::parse_messages(buffer, std::cin.gcount());

      for (auto msg : messages) {
	if (msg->isa<SkyTraqBin::Measurement_time>()) {
	  SkyTraqBin::Measurement_time *mt = msg->cast_as<SkyTraqBin::Measurement_time>();
	  if (mt != NULL) {
	    std::cout << "\tMeasurement time, issue of data: " << (int)mt->issue_of_data()
		      << ", week " << mt->week_number()
		      << ", " << mt->time_in_week() << " ms in week"
		      << ", measurement period " << mt->period() << " ms" << std::endl;
	  }

	} else if (msg->isa<SkyTraqBin::Raw_measurements>()) {
	  SkyTraqBin::Raw_measurements *rm = msg->cast_as<SkyTraqBin::Raw_measurements>();
	  if (rm != NULL) {
	    std::cout << "\tRaw measurements, issue of data: " << (int)rm->issue_of_data() << ", " << (int)rm->num_measurements() << " raw measurements." << std::endl;
	    for (auto m : rm->measurements())
	      std::cout << "\t\tPRN " << (int)m.PRN
			<< ", CN0 " << (int)m.CN0 << " dBHz"
			<< ", pseudo-range " << m.pseudorange << " m"
			<< ", carrier phase " << m.carrier_phase << " cycles"
			<< ", doppler " << m.doppler_freq << " Hz"
			<< std::endl;
	  }

	} else if (msg->isa<SkyTraqBin::SV_channel_status>()) {
	  SkyTraqBin::SV_channel_status *sv = msg->cast_as<SkyTraqBin::SV_channel_status>();
	  if (sv != NULL) {
	    std::cout << "\tSV channel status, issue of data: " << (int)sv->issue_of_data() << ", " << (int)sv->num_svs() << " SV statuses." << std::endl;
	  }


	}
      }
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
  }

  return 0;
}
