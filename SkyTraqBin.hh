/*
        Copyright 2014 Ian Tester

        This file is part of NavSpark tools.

        Photo Finish is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        Photo Finish is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with Photo Finish.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __SKYTRAQBIN_HH__
#define __SKYTRAQBIN_HH__

#include <memory>
#include <iostream>

namespace SkyTraqBin {

  //! Base class for a binary message
  class Message {
  protected:
    unsigned char _msg_id;

  public:
    //! Constructor from a binary buffer
    Message(unsigned char* payload, std::streamsize payload_len);

    inline unsigned char message_id(void) const { return _msg_id; }

    typedef std::shared_ptr<Message> ptr;
  }; // class Message


  //! Base class for messages that come from the GPS receiver
  class Output_message : public Message {
  protected:

  public:

    typedef std::shared_ptr<Output_message> ptr;
  }; // class Output_message


  //! Base class for messages that go to the GPS receiver
  class Input_message : public Message {
  protected:
    //! The length of the body (not including message id)
    virtual unsigned short int body_length(void) const = 0;

    //! Write body fields into a pre-allocated buffer
    virtual void body_to_buf(unsigned char* start) const = 0;

  public:
    //! The total length of the message
    inline unsigned short int message_length(void) const { return 2 + 2 + 1 + body_length() + 1 + 2; }

    //! Write the message into a buffer
    /*!
      Use message_length() to know how big the buffer needs to be.
     */
    virtual void to_buf(unsigned char *buffer) const;

    typedef std::shared_ptr<Input_message> ptr;
  }; // class Input_message

  /* All input message class names shall start with a verb
     Common words shortened:
      configure => config
      download => dl
      message => msg
      navigation => nav
      position => pos
      query => q
      software => sw
      system => sys
   */


}; // SkyTraqBin

#endif // __SKYTRAQBIN_HH__
