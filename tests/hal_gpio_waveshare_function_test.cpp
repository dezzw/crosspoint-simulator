#ifdef NDEBUG
#undef NDEBUG
#endif

#include <FunctionButtonGesture.h>
#include <HalGPIO.h>
#include <SDL.h>

#include <atomic>
#include <cassert>
#include <cstdlib>

#include "GfxRenderer.h"

void sdlClearEventQueue();
void sdlAdvanceMs(unsigned long delta);

std::atomic<bool> quitRequested{false};
GfxRenderer renderer;

namespace {

void pushFaceKey(SDL_Scancode scancode, bool down) {
  SDL_Event event{};
  event.type = down ? SDL_KEYDOWN : SDL_KEYUP;
  event.key.repeat = 0;
  event.key.keysym.scancode = scancode;
  assert(SDL_PushEvent(&event) == 1);
}

void pumpInput() {
  gpio.beginFrame();
  gpio.update();
}

void advanceMs(unsigned long delta) { sdlAdvanceMs(delta); }

void drainDeferredGesture(unsigned long afterMs = 0) {
  advanceMs(afterMs + FunctionButtonGesture::kDoubleClickWindowMs + 5);
  pumpInput();
}

void assertNoLogicalEdges() {
  assert(!gpio.wasPressed(HalGPIO::BTN_BACK));
  assert(!gpio.wasPressed(HalGPIO::BTN_CONFIRM));
  assert(!gpio.wasReleased(HalGPIO::BTN_BACK));
  assert(!gpio.wasReleased(HalGPIO::BTN_CONFIRM));
}

// Function physical key-up must not synthesize Confirm/Back release via HalGPIO.
void testFunctionReleaseDoesNotSpuriouslyReleaseConfirm() {
  pumpInput();
  pushFaceKey(SDL_SCANCODE_RETURN, true);
  gpio.update();
  pushFaceKey(SDL_SCANCODE_RETURN, false);
  gpio.update();
  assert(!gpio.wasReleased(HalGPIO::BTN_CONFIRM));
  assert(!gpio.wasReleased(HalGPIO::BTN_BACK));
  drainDeferredGesture();
}

// Single Function click becomes one Confirm after the double-click window.
void testSingleFunctionClickBecomesConfirm() {
  pumpInput();
  pushFaceKey(SDL_SCANCODE_RETURN, true);
  gpio.update();
  pushFaceKey(SDL_SCANCODE_RETURN, false);
  gpio.update();
  assertNoLogicalEdges();

  advanceMs(FunctionButtonGesture::kDoubleClickWindowMs + 5);
  pumpInput();
  assert(gpio.wasPressed(HalGPIO::BTN_CONFIRM));
  assert(gpio.wasReleased(HalGPIO::BTN_CONFIRM));
  assert(!gpio.wasPressed(HalGPIO::BTN_BACK));
}

// Function double-click within the window becomes Back once.
void testFunctionDoubleClickBecomesBack() {
  pumpInput();
  pushFaceKey(SDL_SCANCODE_RETURN, true);
  gpio.update();
  pushFaceKey(SDL_SCANCODE_RETURN, false);
  gpio.update();

  advanceMs(20);
  pumpInput();
  pushFaceKey(SDL_SCANCODE_RETURN, true);
  gpio.update();
  pushFaceKey(SDL_SCANCODE_RETURN, false);
  gpio.update();

  assert(gpio.wasPressed(HalGPIO::BTN_BACK));
  assert(gpio.wasReleased(HalGPIO::BTN_BACK));
  assert(!gpio.wasPressed(HalGPIO::BTN_CONFIRM));
}

// BOOT+Function chord emits Back when Function releases.
void testBootPlusFunctionBecomesBack() {
  pumpInput();
  pushFaceKey(SDL_SCANCODE_ESCAPE, true);
  gpio.update();
  advanceMs(5);
  pumpInput();
  pushFaceKey(SDL_SCANCODE_RETURN, true);
  gpio.update();
  pushFaceKey(SDL_SCANCODE_RETURN, false);
  gpio.update();

  assert(gpio.wasPressed(HalGPIO::BTN_BACK));
  assert(gpio.wasReleased(HalGPIO::BTN_BACK));
  assert(!gpio.wasPressed(HalGPIO::BTN_CONFIRM));
  pushFaceKey(SDL_SCANCODE_ESCAPE, false);
  gpio.update();
}

// Lone BOOT release stays silent at the logical layer.
void testLoneBootIsSilent() {
  pumpInput();
  pushFaceKey(SDL_SCANCODE_ESCAPE, true);
  gpio.update();
  pushFaceKey(SDL_SCANCODE_ESCAPE, false);
  gpio.update();
  assertNoLogicalEdges();
}

} // namespace

int main() {
  assert(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER) == 0);
  gpio.begin();
  gpio.beginInput();

  testFunctionReleaseDoesNotSpuriouslyReleaseConfirm();
  sdlClearEventQueue();
  testSingleFunctionClickBecomesConfirm();
  sdlClearEventQueue();
  testFunctionDoubleClickBecomesBack();
  sdlClearEventQueue();
  testBootPlusFunctionBecomesBack();
  sdlClearEventQueue();
  testLoneBootIsSilent();

  SDL_Quit();
  return 0;
}
