/*
        Copyright 2014 Ian Tester

        This file is part of GNSS receiver tools.

        GNSS receiver tools is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        GNSS receiver tools is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with GNSS receiver tools.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

//! Append a value to a buffer, in big-endian byte order
/*!
  \param buffer Pointer to a buffer where the next bytes are to added. **NOTE** This value is incremented by append_be.
  \param val The value
 */
template <typename T>
void append_be(unsigned char* &buffer, T val);

//! Extract a value from a buffer, in big-endian byte order
/*!
  \param buffer Pointer to the start of the buffer
  \param offset Where to start reading from
 */
template <typename T>
T extract_be(unsigned char* buffer, unsigned int offset);

//! Extract a 24-bit value from a buffer, in big-endian byte order
unsigned int extract_be24(unsigned char* buffer, unsigned int offset);

