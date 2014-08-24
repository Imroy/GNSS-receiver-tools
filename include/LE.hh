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
#ifndef __LE_HH__
#define __LE_HH__

//! Extract a value from a buffer, in little-endian byte order
/*!
  \param buffer Pointer to the start of the buffer
  \param offset Where to start reading from
 */
template <typename T>
T extract_le(unsigned char* buffer, unsigned int offset);

#endif // __LE_HH__
