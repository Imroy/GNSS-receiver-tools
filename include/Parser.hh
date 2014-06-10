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

#include <functional>
#include <cstdio>
#include <map>
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
    inline virtual void Measurement_time(const SkyTraqBin::Measurement_time &mt) {}
    inline virtual void Raw_measurements(const SkyTraqBin::Raw_measurements &rm) {}
    inline virtual void SV_channel_status(const SkyTraqBin::SV_channel_status &sv_chan) {}
    inline virtual void Subframe_data(const SkyTraqBin::Subframe_data &sfd) {}

    typedef std::shared_ptr<Listener> ptr;
  }; // class Listener


  //! Class for an object that reads from a stream and calls methods in a Listener object
  class Interface {
  public:
    //! Type for a function called when a query/get input message gets a response
    /*!
      \param ack Did the input message receive an ack (true), or a nack (false)?
      \param msg The output message that was received

      There are three ways this lambda will be called:
      -# (false, nullptr) - Nack was received
      -# (true, nullptr) - Ack was received
      -# (true, msg) - Response message received

      The third variant only happens with response-type messages.
     */
    typedef std::function<void(bool ack, SkyTraqBin::Output_message* msg)> ResponseHandler;

  private:
    std::FILE *_file;
    Listener::ptr _listener;
    SkyTraq::Parser _parser;
    std::map<uint16_t, ResponseHandler> _response_handlers;

  public:
    //! Constructor
    /*!
      \param f Open file handle
      \param l Listener object that will receive messages
     */
    Interface(std::FILE* f, Listener::ptr l);

    //! Read a small amount of data from the file, parse it, send messages to the listener object
    void read(void);

    //! Send a message to the device
    void send(SkyTraqBin::Input_message::ptr msg);

    //! Send a message to the device, call lambda when response is received
    void send(SkyTraqBin::Input_message::ptr msg, ResponseHandler rh);

  }; // class Interface


}; // namespace SkyTraq


#endif //__PARSER_HH__
