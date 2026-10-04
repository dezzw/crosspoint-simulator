#include "FunctionButtonGesture.h"

#include <cassert>

int main() {
  FunctionButtonGesture gesture;
  unsigned long now = 0;

  gesture.onPhysicalPressed(FunctionButtonGesture::FaceButton::Function, now);
  gesture.onPhysicalReleased(FunctionButtonGesture::FaceButton::Function, now);
  gesture.advance(now + FunctionButtonGesture::kDoubleClickWindowMs + 1);
  assert(gesture.logicalPressed(HalGPIO::BTN_CONFIRM));
  assert(!gesture.logicalPressed(HalGPIO::BTN_BACK));

  gesture.reset();
  now = 1000;
  gesture.onPhysicalPressed(FunctionButtonGesture::FaceButton::Function, now);
  gesture.onPhysicalReleased(FunctionButtonGesture::FaceButton::Function, now + 10);
  gesture.onPhysicalPressed(FunctionButtonGesture::FaceButton::Function, now + 50);
  gesture.onPhysicalReleased(FunctionButtonGesture::FaceButton::Function, now + 60);
  assert(gesture.logicalPressed(HalGPIO::BTN_BACK));

  gesture.reset();
  now = 2000;
  gesture.onPhysicalPressed(FunctionButtonGesture::FaceButton::Boot, now);
  gesture.onPhysicalPressed(FunctionButtonGesture::FaceButton::Left, now + 5);
  gesture.onPhysicalReleased(FunctionButtonGesture::FaceButton::Left, now + 20);
  assert(gesture.logicalPressed(HalGPIO::BTN_UP));

  gesture.reset();
  now = 3000;
  gesture.onPhysicalPressed(FunctionButtonGesture::FaceButton::Boot, now);
  gesture.onPhysicalPressed(FunctionButtonGesture::FaceButton::Function, now + 5);
  gesture.onPhysicalReleased(FunctionButtonGesture::FaceButton::Function, now + 20);
  assert(gesture.logicalPressed(HalGPIO::BTN_BACK));

  gesture.reset();
  now = 4000;
  gesture.onPhysicalPressed(FunctionButtonGesture::FaceButton::Boot, now);
  gesture.onPhysicalReleased(FunctionButtonGesture::FaceButton::Boot, now + 30);
  assert(!gesture.logicalPressed(HalGPIO::BTN_BACK));
  assert(!gesture.logicalPressed(HalGPIO::BTN_CONFIRM));

  return 0;
}
