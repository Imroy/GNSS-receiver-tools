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
#pragma once

#include <functional>
#include <cstdio>
#include <map>
#include <queue>
#include "NMEA-0183.hh"
#include "SkyTraqBin.hh"

namespace GNSS {

  class EndOfFile : public std::exception {
  private:

  public:
    EndOfFile() {}

    const char* what() const throw() {
      return "End of file";
    }
  }; // class EndOfFile


  class NotSendable : public std::exception {
  private:

  public:
    NotSendable() {}

    const char* what() const throw() {
      return "Is not sendable";
    }
  }; // class NotSendable


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


  class Interface;

  //! Base class that receives parsed messages
  class Listener {
  public:
    inline Listener() {}

    inline virtual ~Listener() {}

    // handler methods for NMEA-0183 sentences
    inline virtual void GGA(Interface* iface, const NMEA0183::GGA &gga) {}
    inline virtual void GLL(Interface* iface, const NMEA0183::GLL &gll) {}
    inline virtual void GSA(Interface* iface, const NMEA0183::GSA &gsa) {}
    inline virtual void GSV(Interface* iface, const NMEA0183::GSV &gsv) {}
    inline virtual void RMC(Interface* iface, const NMEA0183::RMC &rmc) {}
    inline virtual void VTG(Interface* iface, const NMEA0183::VTG &vtg) {}
    inline virtual void ZDA(Interface* iface, const NMEA0183::ZDA &zda) {}

    typedef std::shared_ptr<Listener> ptr;
  }; // class Listener

  //! Base class that receives parsed messages, including SkyTraq-specific NMEA and binary messages
  class Listener_SkyTraq : public Listener {
  public:
    // handler methods for proprietary SkyTraq sentences
    inline virtual void STI_PPS(Interface* iface, const NMEA0183::STI_PPS &sti_pps) {}
    inline virtual void STI_sensors(Interface* iface, const NMEA0183::STI_sensors &sti_sensors) {}

    // handler methods for periodic SkyTraq binary messages
    inline virtual void Navigation_data(Interface* iface, const SkyTraqBin::Nav_data_msg &nav) {}
    inline virtual void Sensor_data(Interface* iface, const SkyTraqBin::Sensor_data &sd) {}

    // handler methods for periodic NS-RAW binary messages
    inline virtual void Measurement_time(Interface* iface, const SkyTraqBin::Measurement_time &mt) {}
    inline virtual void Raw_measurements(Interface* iface, const SkyTraqBin::Raw_measurements &rm) {}
    inline virtual void SV_channel_status(Interface* iface, const SkyTraqBin::SV_channel_status &sv_chan) {}
    inline virtual void Rcv_state(Interface* iface, const SkyTraqBin::Rcv_state &rcv_state) {}
    inline virtual void GPS_subframe_data(Interface* iface, const SkyTraqBin::GPS_subframe_data &sfd) {}

  }; // class Listener_SkyTraq


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
    bool _is_chrdev;
    Listener::ptr _listener;
    GNSS::Parser _parser;
    std::queue<std::pair<unsigned char*, SkyTraqBin::Payload_length> > _output_queue;
    bool _response_pending;
    std::map<uint16_t, ResponseHandler> _response_handlers;

    void _send_from_queue(void);

  public:
    //! Constructor
    /*!
      \param f Open file handle
      \param l Listener object that will receive messages
     */
    Interface(std::FILE* f, Listener::ptr l);

    //! Read a small amount of data from the file, parse it, send messages to the listener object
    void read(void);

    // Can this interface send messages?
    /*!
      Only currently sendable if the file is a character device.
     */
    bool is_sendable(void) const { return _is_chrdev; }

    //! Send a message to the device
    void send(SkyTraqBin::Input_message::ptr msg);

    //! Send a message to the device, call lambda when response is received
    void send(SkyTraqBin::Input_message::ptr msg, ResponseHandler rh);

  }; // class Interface


}; // namespace SkyTraq
