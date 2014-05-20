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
#include <SDL2/SDL.h>
#include "App.hh"

namespace GPSstatus {

  App::App() :
    _running(true),
    _parser("/dev/ttyUSB0"),
    _window(NULL),
    _renderer(NULL)
  {}

  int App::Execute() {
    Init();

    while (_running) {
      Render();

      SDL_Event Event;
      while (SDL_PollEvent(&Event)) {
	OnEvent(&Event);
      }

      Loop();
      SDL_LockMutex(_redraw_lock);
      SDL_CondWaitTimeout(_redraw_cond, _redraw_lock, 16);
      SDL_UnlockMutex(_redraw_lock);
    }
    _parser.stop_running();

    SDL_WaitThread(_parser_thread, NULL);

    Cleanup();

    return 0;
  }

  void App::Init() {
    _redraw_lock = SDL_CreateMutex();
    _redraw_cond = SDL_CreateCond();

    _parser.init(_redraw_lock, _redraw_cond);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      std::cerr << "Could not initialise SDL." << std::endl;
      exit(1);
    }

    if ((_window = SDL_CreateWindow("GPSstatus", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_OPENGL)) == NULL) {
      std::cerr << "Could not create SDL window." << std::endl;
      exit(1);
    }

    if ((_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
      std::cerr << "Could not create SDL renderer." << std::endl;
      exit(1);
    }
    SDL_RenderClear(_renderer);
    SDL_RenderPresent(_renderer);

    _parser_thread = SDL_CreateThread(Parser_runner, "Parser", (void*)&_parser);
    if (_parser_thread == NULL) {
      std::cerr << "Could not create Parser thread." << std::endl;
      exit(1);
    }
  }

  void App::OnEvent(SDL_Event* Event) {
    switch (Event->type) {
    case SDL_QUIT:
      _running = false;
      break;

    case SDL_KEYDOWN:
    case SDL_KEYUP:
      if (Event->key.keysym.scancode == SDL_SCANCODE_Q)
	_running = false;
      break;

    default:
      break;
    }
  }

  void App::Loop() {
  }

  void App::Render() {

    SDL_RenderClear(_renderer);
    SDL_RenderPresent(_renderer);
  }

  void App::Cleanup() {
    if (_renderer)
      SDL_DestroyRenderer(_renderer);
    if (_window)
      SDL_DestroyWindow(_window);

    _parser.cleanup();

    SDL_Quit();
  }

}; // namespace GPSstatus

int main(int argc, char* argv[]) {
  GPSstatus::App app;

  return app.Execute();
}
