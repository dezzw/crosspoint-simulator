#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "$0")/.." && pwd)"
test_binary="$(mktemp "${TMPDIR:-/tmp}/crosspoint-function-button.XXXXXX")"
trap 'rm -f "$test_binary"' EXIT

compile=(
  -std=gnu++20
  -Wall
  -Wextra
  -Wno-unused-parameter
  -DSIMULATOR
  -DCROSSPOINT_EMULATED=1
  -DSIMULATOR_DEVICE_WAVESHARE_EPAPER_397
  "-I$repo_root/tests/display_stubs"
  "-I$repo_root/src"
)

"${CXX:-g++}" "${compile[@]}" \
  "$repo_root/tests/function_button_gesture_test.cpp" \
  "$repo_root/src/FunctionButtonGesture.cpp" \
  -o "$test_binary"
"$test_binary"

hal_compile=("${compile[@]}" "-I$repo_root/tests/input_stubs")
"${CXX:-g++}" "${hal_compile[@]}" \
  "$repo_root/tests/hal_gpio_waveshare_function_test.cpp" \
  "$repo_root/src/FunctionButtonGesture.cpp" \
  "$repo_root/src/HalGPIO.cpp" \
  "$repo_root/src/SimulatorLifecycle.cpp" \
  "$repo_root/src/ESP.cpp" \
  -pthread \
  -o "$test_binary"
"$test_binary"

printf 'Waveshare function-button tests passed\n'
