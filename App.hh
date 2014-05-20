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
#ifndef __APP_HH__
#define __APP_HH__

#include <map>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_thread.h>
#include "Parser.hh"

namespace GPSstatus {

  class App {
  private:
    bool _running;

    SDL_mutex *_redraw_lock;
    SDL_cond *_redraw_cond;

    Parser _parser;
    SDL_Thread *_parser_thread;

    SDL_Window *_window;
    SDL_Renderer *_renderer;

    void Init();

    void OnEvent(SDL_Event* Event);

    void Loop();

    void Render();

    void Cleanup();

  public:
    App();

    int Execute();
  };

}; // namespace GPSstatus

#endif // __APP_HH__