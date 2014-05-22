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
#include "Surface-draw.hh"

namespace GPSstatus {

  void draw_circle(SDL_Surface *surface, double cx, double cy, double radius, SDL_Colour colour) {
    double i = 0, j = radius;
    double e = 1 - radius;
    unsigned int col = SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a);
    while (1) {
      int x1 = floor(cx - i);
      int x2 = floor(cx + i);
      int y1 = floor(cy - j);
      int y2 = floor(cy + j);

      draw_pixel(surface, x1, y1, col);
      draw_pixel(surface, x2, y1, col);
      draw_pixel(surface, x1, y2, col);
      draw_pixel(surface, x2, y2, col);

      int y3 = floor(cy - i);
      if (y1 == y3)
	break;
      int y4 = floor(cy + i);
      if (y2 == y4)
	break;
      int x3 = floor(cx - j);
      int x4 = floor(cx + j);

      draw_pixel(surface, x3, y3, col);
      draw_pixel(surface, x4, y3, col);
      draw_pixel(surface, x3, y4, col);
      draw_pixel(surface, x4, y4, col);

      i++;
      if (e < 0) {
	e += 2 * i + 1;
      } else {
	j--;
	e += 2 * (i - j + 1);
      }
    }
  }

  void draw_filled_circle(SDL_Surface *surface, double cx, double cy, double radius, SDL_Colour colour) {
    double i = 0, j = radius;
    double e = 1 - radius;
    unsigned int col = SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a);
    while (1) {
      int x1 = floor(cx - i);
      int x2 = floor(cx + i);
      int y1 = floor(cy - j);
      int y2 = floor(cy + j);
      draw_hline(surface, x1, x2, y1, col);
      draw_hline(surface, x1, x2, y2, col);

      int y3 = floor(cy - i);
      if (y1 == y3)
	break;
      int y4 = floor(cy + i);
      if (y2 == y4)
	break;
      int x3 = floor(cx - j);
      int x4 = floor(cx + j);
      draw_hline(surface, x3, x4, y3, col);
      draw_hline(surface, x3, x4, y4, col);

      i++;
      if (e < 0) {
	e += 2 * i + 1;
      } else {
	j--;
	e += 2 * (i - j + 1);
      }
    }
  }

  void draw_text(SDL_Surface *surface, TTF_Font *font, std::string text, int x, int y, SDL_Colour colour, double ox, double oy) {
    SDL_Surface *text_surface = TTF_RenderUTF8_Blended(font, text.c_str(), colour);
    if (text_surface) {
      x += ox * text_surface->w;
      y += oy * text_surface->h;
      SDL_Rect destrect = { x, y, text_surface->w, text_surface->h };
      SDL_BlitSurface(text_surface, NULL, surface, &destrect);
      SDL_FreeSurface(text_surface);
    }
  }


}; // namespace GPSstatus
