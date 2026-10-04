#pragma once

#include "HalGPIO.h"

// Decodes Waveshare ESP32-S3 ePaper 3.97 face-button chords into logical
// HalGPIO button indices. Matches CrossMux FunctionButtonGesture semantics:
//   BOOT+Left/Right (release) → Up/Down
//   Function click (after double-click window) → Confirm
//   Function double-click (~300 ms) → Back
//   BOOT+Function (either release while the other is held) → Back
//   Lone BOOT → silent
class FunctionButtonGesture {
public:
  enum class FaceButton : uint8_t { Boot, Function, Left, Right };

  static constexpr unsigned long kDoubleClickWindowMs = 300;

  void reset();

  // Call on SDL/simulated physical edges before advance().
  void onPhysicalPressed(FaceButton button, unsigned long nowMs);
  void onPhysicalReleased(FaceButton button, unsigned long nowMs);

  // Expires deferred single-click Confirm after the double-click window.
  void advance(unsigned long nowMs);

  bool isPhysicalHeld(FaceButton button) const;

  bool logicalPressed(uint8_t logicalIndex) const;
  bool logicalReleased(uint8_t logicalIndex) const;

  void beginLogicalFrame();

private:
  bool physicalHeld_[4] = {};
  bool logicalPressed_[7] = {};
  bool logicalReleased_[7] = {};

  unsigned long functionLastReleaseMs_ = 0;
  unsigned long pendingConfirmAtMs_ = 0;
  bool bootFunctionChordArmed_ = false;

  void latchLogicalEdge(uint8_t logicalIndex);
  void emitConfirmTap();
  void emitBackTap();
  void cancelPendingConfirm();
};
