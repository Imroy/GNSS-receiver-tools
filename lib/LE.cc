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
#include "LE.hh"

template <>
char extract_le<char>(unsigned char* buffer, unsigned int offset) {
  return (char)buffer[offset];
}

template <>
unsigned char extract_le<unsigned char>(unsigned char* buffer, unsigned int offset) {
  return buffer[offset];
}

template <>
short int extract_le<short int>(unsigned char* buffer, unsigned int offset) {
  return buffer[offset] |((short int)buffer[offset + 1] << 8);
}

template <>
unsigned short int extract_le<unsigned short int>(unsigned char* buffer, unsigned int offset) {
  return buffer[offset] | ((unsigned short int)buffer[offset + 1] << 8);
}

template <>
int extract_le<int>(unsigned char* buffer, unsigned int offset) {
  return (int)buffer[offset]
	  | ((int)buffer[offset + 1] << 8)
	  | ((int)buffer[offset + 2] << 16)
	  | ((int)buffer[offset + 3] << 24);
}

template <>
unsigned int extract_le<unsigned int>(unsigned char* buffer, unsigned int offset) {
  return (unsigned int)buffer[offset]
    | ((unsigned int)buffer[offset + 1] << 8)
    | ((unsigned int)buffer[offset + 2] << 16)
    | ((unsigned int)buffer[offset + 3] << 24);
}

template <>
float extract_le<float>(unsigned char* buffer, unsigned int offset) {
  float val;
  unsigned char *mem = (unsigned char*)&val;
#if __BYTE_ORDER == __LITTLE_ENDIAN
  mem[0] = buffer[offset];
  mem[1] = buffer[offset + 1];
  mem[2] = buffer[offset + 2];
  mem[3] = buffer[offset + 3];
#elif __BYTE_ORDER == __BIG_ENDIAN
  mem[3] = buffer[offset];
  mem[2] = buffer[offset + 1];
  mem[1] = buffer[offset + 2];
  mem[0] = buffer[offset + 3];
#endif
  return val;
}

template <>
double extract_le<double>(unsigned char* buffer, unsigned int offset) {
  double val;
  unsigned char *mem = (unsigned char*)&val;
#if __BYTE_ORDER == __LITTLE_ENDIAN
  mem[0] = buffer[offset];
  mem[1] = buffer[offset + 1];
  mem[2] = buffer[offset + 2];
  mem[3] = buffer[offset + 3];
  mem[4] = buffer[offset + 4];
  mem[5] = buffer[offset + 5];
  mem[6] = buffer[offset + 6];
  mem[7] = buffer[offset + 7];
#elif __BYTE_ORDER == __BIG_ENDIAN
  mem[7] = buffer[offset];
  mem[6] = buffer[offset + 1];
  mem[5] = buffer[offset + 2];
  mem[4] = buffer[offset + 3];
  mem[3] = buffer[offset + 4];
  mem[2] = buffer[offset + 5];
  mem[1] = buffer[offset + 6];
  mem[0] = buffer[offset + 7];
#endif
  return val;
}
