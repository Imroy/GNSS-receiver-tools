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
#include <mongo/client/connpool.h>
#include <mongo/client/dbclient.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "SkyTraq.hh"
#include "NMEA-0183.hh"
#include "SkyTraqBin.hh"
#include "Parser.hh"

namespace greg = boost::gregorian;
namespace ptime = boost::posix_time;

ptime::ptime GPS_epoch(greg::date(1980, greg::Jan, 6), ptime::seconds(0));
ptime::ptime time_epoch(greg::date(1970, greg::Jan, 1), ptime::seconds(0));

class MongoListener : public SkyTraq::Listener {
private:
  mongo::ScopedDbConnection *_sdc;
  std::string _dbname;
  mongo::BSONObjBuilder *_current_doc;
  uint8_t _current_issue;
  uint32_t _time_in_week;
  ptime::time_duration _leap_seconds = ptime::seconds(16); // Current value since June 30, 2012

  void _check_issue(uint8_t i) {
    if (i != _current_issue) {
      if (_current_doc != nullptr) {
	std::cerr << "Inserting issue #" << (int)_current_issue << std::endl;
	_sdc->conn().insert(_dbname + ".messages",
			    _current_doc->obj());
	delete _current_doc;
	_current_doc = nullptr;
      }

      _current_issue = i;
    }

    if (_current_doc == nullptr)
      _current_doc = new mongo::BSONObjBuilder;
  }

public:
  MongoListener(mongo::ScopedDbConnection *s, std::string db) :
    _sdc(s), _dbname(db),
    _current_doc(nullptr), _current_issue(0)
  {
    _sdc->conn().ensureIndex(db + ".messages", BSON("week_number" << 1 << "time_in_week" << 1));
    _sdc->conn().ensureIndex(db + ".messages", BSON("time" << 1));
    _sdc->conn().ensureIndex(db + ".subframes", BSON("PRN" << "1" << "subframe_num" << 1));
    _sdc->conn().ensureIndex(db + ".subframes", BSON("PRN" << "1" << "subframe_num" << 1 << "page_num" << 1));
  }

  void Measurement_time(SkyTraq::Interface* iface, const SkyTraqBin::Measurement_time &mt) {
    _time_in_week = mt.time_in_week();

    _check_issue(mt.issue_of_data());

    ptime::ptime time = GPS_epoch + greg::days(mt.week_number() * 7) + ptime::milliseconds(mt.time_in_week()) - _leap_seconds;

    *_current_doc << "week_number" << mt.week_number()
		  << "time_in_week" << mt.time_in_week()
		  << "time" << mongo::Date_t((time - time_epoch).total_milliseconds())
		  << "period" << mt.period();
  }

  void Raw_measurements(SkyTraq::Interface* iface, const SkyTraqBin::Raw_measurements &rm) {
    _check_issue(rm.issue_of_data());

    mongo::BSONArrayBuilder meas;
    for (auto m : rm.measurements()) {
      mongo::BSONObjBuilder mb;
      mb << "PRN" << m.PRN
	 << "CN0" << m.CN0
	 << "will_cycle_slip" << m.will_cycle_slip
	 << "coherent_integration_time" << m.coherent_integration_time;
      if (m.has_pseudo_range)
	mb << "pseudo_range" << m.pseudo_range;
      if (m.has_doppler_freq)
	mb << "doppler_freq" << m.doppler_freq;
      if (m.has_carrier_phase)
	mb << "carrier_phase" << m.carrier_phase;
      meas.append(mb.obj());
    }
    _current_doc->append("measurements", meas.arr());
  }

  void SV_channel_status(SkyTraq::Interface* iface, const SkyTraqBin::SV_channel_status &sv_chan) {
    _check_issue(sv_chan.issue_of_data());

    mongo::BSONArrayBuilder statuses;
    for (auto s : sv_chan.statuses()) {
      mongo::BSONObjBuilder sb;
      sb << "channel_id" << s.channel_id
	 << "PRN" << s.PRN
	 << "SV_has_almanac" << s.sv_has_almanac
	 << "SV_has_ephemeris" << s.sv_has_ephemeris
	 << "SV_is_healthy" << s.sv_is_healthy
	 << "user_range_accuracy" << s.URA
	 << "CN0" << s.CN0
	 << "elevation" << s.elevation
	 << "azimuth" << s.azimuth
	 << "channel_has_pull_in" << s.chan_has_pull_in
	 << "channel_has_bit_sync" << s.chan_has_bit_sync
	 << "channel_has_frame_sync" << s.chan_has_frame_sync
	 << "channel_has_ephemeris" << s.chan_has_ephemeris
	 << "channel_used_for_normal" << s.chan_for_normal
	 << "channel_used_for_differential" << s.chan_for_differential;
      statuses.append(sb.obj());
    }
    _current_doc->append("statuses", statuses.arr());
  }

  void Rcv_state(SkyTraq::Interface* iface, const SkyTraqBin::Rcv_state &rcv) {
    _check_issue(rcv.issue_of_data());

    *_current_doc << "navigation_state" << std::to_string(rcv.navigation_state())
		  << "ECEF_X" << rcv.ECEF_X()
		  << "ECEF_Y" << rcv.ECEF_Y()
		  << "ECEF_Z" << rcv.ECEF_Z()
		  << "ECEF_VX" << rcv.ECEF_VX()
		  << "ECEF_VY" << rcv.ECEF_VY()
		  << "ECEF_VZ" << rcv.ECEF_VZ()
		  << "clock_bias" << rcv.clock_bias()
		  << "clock_drift" << rcv.clock_drift()
		  << "GDOP" << rcv.GDOP()
		  << "PDOP" << rcv.PDOP()
		  << "HDOP" << rcv.HDOP()
		  << "VDOP" << rcv.VDOP()
		  << "TDOP" << rcv.TDOP();
  }

  void Subframe_data(SkyTraq::Interface* iface, const SkyTraqBin::Subframe_data &sfd) {
    mongo::BSONObjBuilder doc;
    doc << "PRN" << sfd.PRN()
	<< "subframe_num" << sfd.subframe_num();
    if (sfd.subframe_num() > 3) {
      // Note: Fudge factor! I'm going to assume it takes a short amount of
      // time from the last Measurement_time message until this subframe.
      // Without this subframe 5 would appear to be page_num + 1.
      uint8_t page_num = 1 + ((_time_in_week - 6000) / 30000) % 25;
      doc << "page_num" << page_num;

      if ((sfd.subframe_num() == 4) && (page_num == 18)) {
	uint8_t dt_LS = (sfd.word(2) >> 8) & 0xff;
	if (_leap_seconds.total_seconds() > dt_LS)
	  // NOTE: Don't know why it's incorrect some times
	  std::cerr << "Decreasing number of leap seconds!" << std::endl;
	else
	  _leap_seconds = ptime::seconds(dt_LS);
      }
    }

    mongo::Query query(doc.asTempObj());

    mongo::BSONArrayBuilder words;
    for (int i = 0; i < 10; i++) {
      mongo::BSONArrayBuilder wb;
      uint32_t word = sfd.word(i);
      unsigned char bytes[3] = { (unsigned char)(word & 0xff),
				 (unsigned char)((word >> 8) & 0xff),
				 (unsigned char)((word >> 16) & 0xff) };
      wb << bytes[0] << bytes[1] << bytes[2];
      words.append(wb.arr());
    }
    doc << "words" << words.arr();

    std::cerr << "Inserting sub-frame #" << (int)sfd.subframe_num() << " of SV #" << (int)sfd.PRN() << std::endl;
    _sdc->conn().update(_dbname + ".subframes",
			query,
			doc.obj(),
			true);
  }

}; // class MongoListener

int main(int argc, char* argv[]) {
  std::string filename = "/dev/ttyUSB0";
  std::string hostname = "localhost";
  std::string username, password, dbname = "nsraw";

  if (argc > 1) {
    int opt;
    while ((opt = getopt(argc, argv, "H:u:p:d:")) != -1) {
      switch (opt) {
      case 'H':
	hostname = optarg;
	break;

      case 'u':
	username = optarg;
	break;

      case 'p':
	password = optarg;
	break;

      case 'd':
	dbname = optarg;
	break;

      default:
	break;
      }
    }

    if (optind < argc)
      filename = argv[optind];
  }

  std::cerr << "Opening \"" << filename << "\"..." << std::endl;
  std::FILE *file = fopen(filename.c_str(), "r");
  if (file == NULL) {
    std::cerr << "input is not open: " << strerror(errno) << std::endl;
    exit(1);
  }

  mongo::ScopedDbConnection *sdc;
  {
    std::cerr << "Connecting to " << hostname << "..." << std::endl;
    mongo::ConnectionString cs(hostname, mongo::ConnectionString::MASTER);
    sdc = mongo::ScopedDbConnection::getScopedDbConnection(cs);
    if (username.length() > 0) {
      std::string err = "";
      sdc->conn().auth(dbname, username, password, err, true);
    }
  }

  auto l = std::make_shared<MongoListener>(sdc, dbname);
  SkyTraq::Interface iface(file, l);

  while (1) {
    try {
      iface.read();
    } catch (SkyTraq::EndOfFile &e) {
      break;
    } catch (std::invalid_argument &e) {
      std::cerr << e.what() << std::endl;
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
  }

  return 0;
}
