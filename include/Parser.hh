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
#ifndef __PARSER_HH__
#define __PARSER_HH__

#include <cstdio>
#include "NMEA-0183.hh"
#include "SkyTraqBin.hh"

namespace SkyTraq {

  //! Unified parser class
  class Parser {
  private:
    unsigned char *_parse_buffer;
    std::size_t _parse_buflen;

  public:
    //! Empty constructor
    Parser();

    //! Destructor
    ~Parser();

    //! Clear the parse buffer
    void reset_buffer(void);

    //! Add bytes to the end of the parse buffer
    void add_bytes(unsigned char* buffer, std::size_t buffer_len);

    //! Parse contents of the parse buffer into messages
    std::vector<Message::ptr> parse_messages(void);

  }; // class Parser


  //! Base class that receives parsed messages
  class Listener {
  public:
    inline Listener() {}

    inline virtual ~Listener() {}

    // handler methods for NMEA-0183 sentences
    inline virtual void GGA(const NMEA0183::GGA &gga) {}
    inline virtual void GLL(const NMEA0183::GLL &gll) {}
    inline virtual void GSA(const NMEA0183::GSA &gsa) {}
    inline virtual void GSV(const NMEA0183::GSV &gsv) {}
    inline virtual void RMC(const NMEA0183::RMC &rmc) {}
    inline virtual void VTG(const NMEA0183::VTG &vtg) {}
    inline virtual void ZDA(const NMEA0183::ZDA &zda) {}
    inline virtual void STI(const NMEA0183::STI &sti) {}

    // handler methods for SkyTraq binary messages
    inline virtual void GNSS_boot_status(const SkyTraqBin::GNSS_boot_status &bs) {}
    inline virtual void Sw_ver(const SkyTraqBin::Sw_ver &ver) {}
    inline virtual void Sw_CRC(const SkyTraqBin::Sw_CRC &crc) {}
    inline virtual void Ack(const SkyTraqBin::Ack &ack) {}
    inline virtual void Nack(const SkyTraqBin::Nack &nack) {}
    inline virtual void Pos_update_rate(const SkyTraqBin::Pos_update_rate &rate) {}
    inline virtual void NMEA_talker_ID(const SkyTraqBin::NMEA_talker_ID &id) {}
    inline virtual void Nav_data_msg(const SkyTraqBin::Nav_data_msg &msg) {}
    inline virtual void GNSS_datum(const SkyTraqBin::GNSS_datum &datum) {}
    inline virtual void GNSS_DOP_mask(const SkyTraqBin::GNSS_DOP_mask &mask) {}
    inline virtual void GNSS_elevation_CNR_mask(const SkyTraqBin::GNSS_elevation_CNR_mask &mask) {}
    inline virtual void GPS_ephemeris_data(const SkyTraqBin::GPS_ephemeris_data &eph) {}
    inline virtual void GNSS_power_mode_status(const SkyTraqBin::GNSS_power_mode_status &stat) {}
    inline virtual void Measurement_time(const SkyTraqBin::Measurement_time &mt) {}
    inline virtual void Raw_measurements(const SkyTraqBin::Raw_measurements &rm) {}
    inline virtual void SV_channel_status(const SkyTraqBin::SV_channel_status &sv_chan) {}
    inline virtual void Subframe_data(const SkyTraqBin::Subframe_data &sfd) {}

    typedef std::shared_ptr<Listener> ptr;
  }; // class Listener


  //! Class for an object that reads from a stream and calls methods in a Listener object
  class Reader {
  private:
    std::FILE *_file;
    Listener::ptr _listener;
    SkyTraq::Parser _parser;

  public:
    //! Constructor
    Reader(std::FILE* f, Listener::ptr l);

    //! Read a small amount of data from the file, parse it, send messages to the listener object
    void read(void);

    //! Send a message to the device
    void write(SkyTraqBin::Input_message::ptr msg);

  }; // class Reader


}; // namespace SkyTraq


#endif //__PARSER_HH__
