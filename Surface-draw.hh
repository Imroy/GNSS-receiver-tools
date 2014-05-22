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
#ifndef __SURFACE_DRAW_HH__
#define __SURFACE_DRAW_HH__

#include <string>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>

namespace GPSstatus {

  inline void draw_pixel(SDL_Surface *surface, int x, int y, unsigned colour) {
    unsigned char *pixels = (unsigned char*)surface->pixels + (y * surface->pitch) + (x * surface->format->BytesPerPixel);
    SDL_memset4(pixels, colour, 1);
  }

  inline void draw_pixel(SDL_Surface *surface, int x, int y, SDL_Colour colour) {
    draw_pixel(surface, x, y, SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a));
  }

  inline void draw_hline(SDL_Surface *surface, int x1, int x2, int y, unsigned int colour) {
    unsigned char *pixels = (unsigned char*)surface->pixels + (y * surface->pitch) + (x1 * surface->format->BytesPerPixel);
    SDL_memset4(pixels, colour, x2 - x1 + 1);
  }

  inline void draw_hline(SDL_Surface *surface, int x1, int x2, int y, SDL_Colour colour) {
    draw_hline(surface, x1, x2, y, SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a));
  }

  inline void draw_vline(SDL_Surface *surface, int x, int y1, int y2, unsigned int colour) {
    unsigned char *pixels = (unsigned char*)surface->pixels + (y1 * surface->pitch) + (x * surface->format->BytesPerPixel);
    for (int y = y1; y < y2; y++, pixels += surface->pitch)
      SDL_memset4(pixels, colour, 1);
  }

  inline void draw_vline(SDL_Surface *surface, int x, int y1, int y2, SDL_Colour colour) {
    draw_vline(surface, x, y1, y2, SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a));
  }

  void draw_circle(SDL_Surface *surface, double cx, double cy, double radius, SDL_Colour colour);

  void draw_filled_circle(SDL_Surface *surface, double cx, double cy, double radius, SDL_Colour colour);

  void draw_text(SDL_Surface *surface, TTF_Font *font, std::string text, int x, int y, SDL_Colour colour, double ox=0.0, double oy=0.0);

}; // namespace GPSstatus

#endif // __SURFACE_DRAW_HH__
