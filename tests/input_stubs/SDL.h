#pragma once

#include <cstdint>
#include <queue>
#include <vector>

using SDL_Scancode = int;
using Uint8 = uint8_t;
using Uint32 = uint32_t;

enum {
  SDL_SCANCODE_UNKNOWN = 0,
  SDL_SCANCODE_ESCAPE,
  SDL_SCANCODE_RETURN,
  SDL_SCANCODE_LEFT,
  SDL_SCANCODE_RIGHT,
  SDL_SCANCODE_UP,
  SDL_SCANCODE_DOWN,
  SDL_SCANCODE_P,
  SDL_SCANCODE_S,
  SDL_SCANCODE_H,
};

enum {
  SDL_INIT_EVENTS = 0x00004000,
  SDL_INIT_TIMER = 0x00000001,
  SDL_INIT_VIDEO = 0x00000020,
};

enum {
  SDL_QUIT = 0x100,
  SDL_KEYDOWN = 0x300,
  SDL_KEYUP = 0x301,
  SDL_MOUSEBUTTONDOWN = 0x401,
  SDL_MOUSEBUTTONUP = 0x402,
  SDL_MOUSEMOTION = 0x400,
};

enum { SDL_BUTTON_LEFT = 1 };

struct SDL_Keysym {
  SDL_Scancode scancode;
};

struct SDL_Event {
  Uint32 type = 0;
  struct {
    int repeat = 0;
    SDL_Keysym keysym{};
  } key;
  struct {
    Uint32 windowID = 0;
    Uint8 button = 0;
    int x = 0;
    int y = 0;
  } button;
  struct {
    int x = 0;
    int y = 0;
  } motion;
};

inline std::queue<SDL_Event> &sdlEventQueue() {
  static std::queue<SDL_Event> queue;
  return queue;
}

inline unsigned long &sdlTicks() {
  static unsigned long ticks = 0;
  return ticks;
}

inline void sdlClearEventQueue() {
  while (!sdlEventQueue().empty()) {
    sdlEventQueue().pop();
  }
}

inline void sdlAdvanceMs(unsigned long delta) { sdlTicks() += delta; }

inline int SDL_Init(Uint32) { return 0; }

inline void SDL_Quit() {}

inline unsigned long SDL_GetTicks() { return sdlTicks(); }

inline void SDL_Delay(Uint32 ms) { sdlTicks() += ms; }

inline int SDL_PushEvent(SDL_Event *event) {
  if (!event) {
    return -1;
  }
  sdlEventQueue().push(*event);
  return 1;
}

inline int SDL_PollEvent(SDL_Event *event) {
  if (sdlEventQueue().empty()) {
    return 0;
  }
  *event = sdlEventQueue().front();
  sdlEventQueue().pop();
  return 1;
}

inline const Uint8 *SDL_GetKeyboardState(int *) {
  static Uint8 state[512] = {};
  return state;
}
