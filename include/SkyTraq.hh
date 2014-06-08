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
#ifndef __SKYTRAQ_HH__
#define __SKYTRAQ_HH__

#include <memory>
#include <typeinfo>
#include <vector>

namespace SkyTraq {

  //! Abstract base class for both NMEA-0183 and binary messages
  class Message {
  public:
    //! Virtual destructor to force polymorphism
    inline virtual ~Message() {}

    //! Check the type of an object
    template <typename T>
    inline bool isa(void) const { return typeid(*this) == typeid(T); }

    //! Recast this object to another type
    template <typename T>
    inline T* cast_as(void) {
      T *a = dynamic_cast<T*>(this);
      if (a == nullptr)
	throw std::bad_cast();
      return a;
    }

    typedef std::shared_ptr<Message> ptr;
  }; // class Message


  //! Unified parser class
  class Parser {
  private:
    unsigned char *_parse_buffer;
    std::streamsize _parse_buflen;

  public:
    //! Empty constructor
    Parser();

    //! Destructor
    ~Parser();

    //! Clear the parse buffer
    void reset_buffer(void);

    //! Add bytes to the end of the parse buffer
    void add_bytes(unsigned char* buffer, std::streamsize buffer_len);

    //! Parse contents of the parse buffer into messages
    std::vector<Message::ptr> parse_messages(void);

  }; // class Parser


}; // namespace SkyTraq

#endif // __SKYTRAQ_HH__
