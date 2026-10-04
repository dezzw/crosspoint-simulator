#include "FunctionButtonGesture.h"

namespace {

uint8_t faceIndex(FunctionButtonGesture::FaceButton button) {
  return static_cast<uint8_t>(button);
}

} // namespace

void FunctionButtonGesture::reset() {
  for (bool &held : physicalHeld_) {
    held = false;
  }
  beginLogicalFrame();
  functionLastReleaseMs_ = 0;
  pendingConfirmAtMs_ = 0;
  bootFunctionChordArmed_ = false;
}

void FunctionButtonGesture::beginLogicalFrame() {
  for (bool &pressed : logicalPressed_) {
    pressed = false;
  }
  for (bool &released : logicalReleased_) {
    released = false;
  }
}

bool FunctionButtonGesture::isPhysicalHeld(FaceButton button) const {
  return physicalHeld_[faceIndex(button)];
}

bool FunctionButtonGesture::logicalPressed(uint8_t logicalIndex) const {
  return logicalIndex < 7 && logicalPressed_[logicalIndex];
}

bool FunctionButtonGesture::logicalReleased(uint8_t logicalIndex) const {
  return logicalIndex < 7 && logicalReleased_[logicalIndex];
}

void FunctionButtonGesture::latchLogicalEdge(uint8_t logicalIndex) {
  if (logicalIndex >= 7) {
    return;
  }
  logicalPressed_[logicalIndex] = true;
  logicalReleased_[logicalIndex] = true;
}

void FunctionButtonGesture::emitConfirmTap() {
  latchLogicalEdge(HalGPIO::BTN_CONFIRM);
}

void FunctionButtonGesture::emitBackTap() {
  latchLogicalEdge(HalGPIO::BTN_BACK);
}

void FunctionButtonGesture::cancelPendingConfirm() {
  pendingConfirmAtMs_ = 0;
  functionLastReleaseMs_ = 0;
}

void FunctionButtonGesture::onPhysicalPressed(FaceButton button,
                                                unsigned long nowMs) {
  (void)nowMs;
  physicalHeld_[faceIndex(button)] = true;

  if (button == FaceButton::Boot && isPhysicalHeld(FaceButton::Function)) {
    bootFunctionChordArmed_ = true;
  } else if (button == FaceButton::Function &&
             isPhysicalHeld(FaceButton::Boot)) {
    bootFunctionChordArmed_ = true;
  }

  if (button == FaceButton::Left && !isPhysicalHeld(FaceButton::Boot)) {
    logicalPressed_[HalGPIO::BTN_LEFT] = true;
  } else if (button == FaceButton::Right && !isPhysicalHeld(FaceButton::Boot)) {
    logicalPressed_[HalGPIO::BTN_RIGHT] = true;
  }
}

void FunctionButtonGesture::onPhysicalReleased(FaceButton button,
                                                 unsigned long nowMs) {
  const bool wasHeld = physicalHeld_[faceIndex(button)];
  if (!wasHeld) {
    return;
  }
  physicalHeld_[faceIndex(button)] = false;

  switch (button) {
  case FaceButton::Boot:
    if (isPhysicalHeld(FaceButton::Function) && bootFunctionChordArmed_) {
      emitBackTap();
      cancelPendingConfirm();
      bootFunctionChordArmed_ = false;
    }
    break;
  case FaceButton::Function:
    if (isPhysicalHeld(FaceButton::Boot) && bootFunctionChordArmed_) {
      emitBackTap();
      cancelPendingConfirm();
      bootFunctionChordArmed_ = false;
      break;
    }
    if (functionLastReleaseMs_ != 0 &&
        nowMs - functionLastReleaseMs_ <= kDoubleClickWindowMs) {
      emitBackTap();
      cancelPendingConfirm();
      break;
    }
    functionLastReleaseMs_ = nowMs;
    pendingConfirmAtMs_ = nowMs + kDoubleClickWindowMs;
    break;
  case FaceButton::Left:
    if (isPhysicalHeld(FaceButton::Boot)) {
      latchLogicalEdge(HalGPIO::BTN_UP);
    } else {
      logicalReleased_[HalGPIO::BTN_LEFT] = true;
    }
    break;
  case FaceButton::Right:
    if (isPhysicalHeld(FaceButton::Boot)) {
      latchLogicalEdge(HalGPIO::BTN_DOWN);
    } else {
      logicalReleased_[HalGPIO::BTN_RIGHT] = true;
    }
    break;
  }

  if (!isPhysicalHeld(FaceButton::Boot) && !isPhysicalHeld(FaceButton::Function)) {
    bootFunctionChordArmed_ = false;
  }
}

void FunctionButtonGesture::advance(unsigned long nowMs) {
  if (pendingConfirmAtMs_ == 0 || nowMs < pendingConfirmAtMs_) {
    return;
  }
  pendingConfirmAtMs_ = 0;
  functionLastReleaseMs_ = 0;
  emitConfirmTap();
}
