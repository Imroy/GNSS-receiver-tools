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
#include <iostream>
#include <map>
#include <memory>
#include <math.h>
#include "Surface-draw.hh"

namespace GPSstatus {

  struct Intersections {
    int start, end;
    std::map<int, std::pair<double, double> > intersections;

    inline Intersections(int s, int e) :
      start(s), end(e)
    {}

    inline std::pair<double, double>& operator [](int i) { return intersections[i]; }
    inline bool has(int i) { return intersections.count(i) > 0; }

    typedef std::shared_ptr<Intersections> ptr;
  }; // struct Intersections

  template <typename T>
  inline T sqr(T x) { return x * x; }

  Intersections::ptr circle_intersections(double cx, double cy, double radius) {
    int miny = floor(1 + cy - radius);
    int maxy = floor(cy + radius);
    auto ins = std::make_shared<Intersections>(miny, maxy);

    double rsq = radius * radius;
    for (int y = miny; y <= maxy; y++) {
      double d = rsq - sqr(y - cy);
      if (d >= 0.0) {
	double dsqrt = sqrt(d);
	(*ins)[y] = std::make_pair(cx - dsqrt, cx + dsqrt);
      }
    }

    return ins;
  }

  void draw_circle(SDL_Surface *surface, double cx, double cy, double radius, SDL_Colour colour) {
    unsigned int col = SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a);

    auto outer = circle_intersections(cx, cy, radius + 0.5);
    auto inner = circle_intersections(cx, cy, radius - 0.5);

    int miny = outer->start;
    if (miny < 0)
      miny = 0;
    int maxy = outer->end;
    if (maxy >= surface->h)
      maxy = surface->h - 1;

    for (int y = miny; y <= maxy; y++) {
      int x1 = floor((*outer)[y].first);
      int x4 = floor(0.5 + (*outer)[y].second);

      if (inner->has(y)) {
	int x2 = floor(0.5 + (*inner)[y].first);
	int x3 = floor((*inner)[y].second);
	if ((x2 >= 0) && (x1 < surface->w)){
	  if (x1 < 0)
	    x1 = 0;
	  if (x2 >= surface->w)
	    x2 = surface->w - 1;
	  draw_hline(surface, x1, x2, y, col);
	}

	if ((x4 >= 0) && (x3 < surface->w)){
	  if (x3 < 0)
	    x3 = 0;
	  if (x4 >= surface->w)
	    x4 = surface->w - 1;
	  draw_hline(surface, x3, x4, y, col);
	}
      } else {
	if ((x4 >= 0) && (x1 < surface->w)){
	  if (x1 < 0)
	    x1 = 0;
	  if (x4 >= surface->w)
	    x4 = surface->w - 1;
	  draw_hline(surface, x1, x4, y, col);
	}
      }

    }

  }

  void draw_filled_circle(SDL_Surface *surface, double cx, double cy, double radius, SDL_Colour colour) {
    unsigned int col = SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a);
    auto outline = circle_intersections(cx, cy, radius);

    int miny = outline->start;
    if (miny < 0)
      miny = 0;
    int maxy = outline->end;
    if (maxy >= surface->h)
      maxy = surface->h - 1;

    for (int y = miny; y <= maxy; y++) {
      int x1 = floor((*outline)[y].first);
      int x2 = floor((*outline)[y].second - 0.5);
      if ((x2 >= 0) && (x1 < surface->w)){
	if (x1 < 0)
	  x1 = 0;
	if (x2 >= surface->w)
	  x2 = surface->w - 1;
	draw_hline(surface, x1, x2, y, col);
      }
    }
  }

  void draw_box(SDL_Surface *surface, int x1, int y1, int x2, int y2, SDL_Colour colour) {
    unsigned int col = SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a);
    draw_hline(surface, x1, x2, y1, col);
    draw_vline(surface, x1, y1, y2, col);
    draw_vline(surface, x2, y1, y2, col);
    draw_hline(surface, x1, x2, y2, col);
  }

  void draw_filled_box(SDL_Surface *surface, int x1, int y1, int x2, int y2, SDL_Colour colour) {
    unsigned int col = SDL_MapRGBA(surface->format, colour.r, colour.g, colour.b, colour.a);
    unsigned char *pixels = (unsigned char*)surface->pixels + (y1 * surface->pitch) + (x1 * surface->format->BytesPerPixel);
    for (int y = y1; y <= y2; y++, pixels += surface->pitch)
      SDL_memset4(pixels, col, x2 - x1 + 1);
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
