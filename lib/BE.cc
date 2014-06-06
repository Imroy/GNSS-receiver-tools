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
#include "BE.hh"

// Write a boolean as a single byte
template <>
void append_be<bool>(unsigned char* &buffer, bool val) {
  buffer[0] = (unsigned char)val;
  buffer++;
}

template <>
void append_be<char>(unsigned char* &buffer, char val) {
  buffer[0] = val;
  buffer++;
}

template <>
void append_be<unsigned char>(unsigned char* &buffer, unsigned char val) {
  buffer[0] = val;
  buffer++;
}

template <>
void append_be<short int>(unsigned char* &buffer, short int val) {
  buffer[0] = val >> 8;
  buffer[1] = val & 0xff;
  buffer += 2;
}

template <>
void append_be<unsigned short int>(unsigned char* &buffer, unsigned short int val) {
  buffer[0] = val >> 8;
  buffer[1] = val & 0xff;
  buffer += 2;
}

template <>
void append_be<int>(unsigned char* &buffer, int val) {
  buffer[0] = val >> 24;
  buffer[1] = (val >> 16) & 0xff;
  buffer[2] = (val >> 8) & 0xff;
  buffer[2] = val & 0xff;
  buffer += 4;
}

template <>
void append_be<unsigned int>(unsigned char* &buffer, unsigned int val) {
  buffer[0] = val >> 24;
  buffer[1] = (val >> 16) & 0xff;
  buffer[2] = (val >> 8) & 0xff;
  buffer[2] = val & 0xff;
  buffer += 4;
}

template <>
void append_be<float>(unsigned char* &buffer, float val) {
  unsigned char *mem = (unsigned char*)&val;
#if __BYTE_ORDER == __LITTLE_ENDIAN
  buffer[0] = mem[3];
  buffer[1] = mem[2];
  buffer[2] = mem[1];
  buffer[3] = mem[0];
#elif __BYTE_ORDER == __BIG_ENDIAN
  buffer[0] = mem[0];
  buffer[1] = mem[1];
  buffer[2] = mem[2];
  buffer[3] = mem[3];
#endif
  buffer += 4;
}

template <>
void append_be<double>(unsigned char* &buffer, double val) {
  unsigned char *mem = (unsigned char*)&val;
#if __BYTE_ORDER == __LITTLE_ENDIAN
  buffer[0] = mem[7];
  buffer[1] = mem[6];
  buffer[2] = mem[5];
  buffer[3] = mem[4];
  buffer[4] = mem[3];
  buffer[5] = mem[2];
  buffer[6] = mem[1];
  buffer[7] = mem[0];
#elif __BYTE_ORDER == __BIG_ENDIAN
  buffer[0] = mem[0];
  buffer[1] = mem[1];
  buffer[2] = mem[2];
  buffer[3] = mem[3];
  buffer[4] = mem[4];
  buffer[5] = mem[5];
  buffer[6] = mem[6];
  buffer[7] = mem[7];
#endif
  buffer += 8;
}

template <>
char extract_be<char>(unsigned char* buffer, unsigned int offset) {
  return (char)buffer[offset];
}

template <>
unsigned char extract_be<unsigned char>(unsigned char* buffer, unsigned int offset) {
  return buffer[offset];
}

template <>
short int extract_be<short int>(unsigned char* buffer, unsigned int offset) {
  return ((short int)buffer[offset] << 8) | buffer[offset + 1];
}

template <>
unsigned short int extract_be<unsigned short int>(unsigned char* buffer, unsigned int offset) {
  return ((unsigned short int)buffer[offset] << 8) | buffer[offset + 1];
}

template <>
int extract_be<int>(unsigned char* buffer, unsigned int offset) {
  return ((int)(buffer[offset] << 24)
	  | ((int)buffer[offset + 1] << 16)
	  | ((int)buffer[offset + 2] << 8)
	  | (int)buffer[offset + 3]);
}

template <>
unsigned int extract_be<unsigned int>(unsigned char* buffer, unsigned int offset) {
  return ((unsigned int)buffer[offset] << 24)
    | ((unsigned int)buffer[offset + 1] << 16)
    | ((unsigned int)buffer[offset + 2] << 8)
    | (unsigned int)buffer[offset + 3];
}

template <>
float extract_be<float>(unsigned char* buffer, unsigned int offset) {
  float val;
  unsigned char *mem = (unsigned char*)&val;
#if __BYTE_ORDER == __LITTLE_ENDIAN
  mem[3] = buffer[offset];
  mem[2] = buffer[offset + 1];
  mem[1] = buffer[offset + 2];
  mem[0] = buffer[offset + 3];
#elif __BYTE_ORDER == __BIG_ENDIAN
  mem[0] = buffer[offset];
  mem[1] = buffer[offset + 1];
  mem[2] = buffer[offset + 2];
  mem[3] = buffer[offset + 3];
#endif
  return val;
}

template <>
double extract_be<double>(unsigned char* buffer, unsigned int offset) {
  double val;
  unsigned char *mem = (unsigned char*)&val;
#if __BYTE_ORDER == __LITTLE_ENDIAN
  mem[7] = buffer[offset];
  mem[6] = buffer[offset + 1];
  mem[5] = buffer[offset + 2];
  mem[4] = buffer[offset + 3];
  mem[3] = buffer[offset + 4];
  mem[2] = buffer[offset + 5];
  mem[1] = buffer[offset + 6];
  mem[0] = buffer[offset + 7];
#elif __BYTE_ORDER == __BIG_ENDIAN
  mem[0] = buffer[offset];
  mem[1] = buffer[offset + 1];
  mem[2] = buffer[offset + 2];
  mem[3] = buffer[offset + 3];
  mem[4] = buffer[offset + 4];
  mem[5] = buffer[offset + 5];
  mem[6] = buffer[offset + 6];
  mem[7] = buffer[offset + 7];
#endif
  return val;
}


