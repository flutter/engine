// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/key_event_regulator.h"

#include "flutter/fml/logging.h"

namespace flutter {

namespace {
typedef KeyEventRegulator::KeyPhase KeyPhase;

bool IsValidPhase(KeyPhase phase) {
  return phase >= KeyEventRegulator::kReleasedDisabled &&
         phase <= KeyEventRegulator::kPressed;
}

bool IsPressed(KeyPhase phase) {
  FML_CHECK(IsValidPhase(phase));
  return phase & 0x01;
}

KeyPhase OverridePhase(KeyPhase base, KeyPhase new_value) {
  FML_CHECK(IsValidPhase(base));
  FML_CHECK(IsValidPhase(new_value));
  switch (base) {
    case KeyEventRegulator::kReleasedDisabled:
    case KeyEventRegulator::kReleasedEnabled:
      if (new_value == KeyEventRegulator::kReleased) {
        return base;
      }
      break;
    case KeyEventRegulator::kPressedDisabled:
    case KeyEventRegulator::kPressedEnabled:
      if (new_value == KeyEventRegulator::kPressed) {
        return base;
      }
      break;
  }
  return new_value;
}

} // namespace

KeyEventRegulator::KeyEventRegulator(Config config) {
}

std::vector<KeyEventRegulator::OutputEvent> KeyEventRegulator::SettleByEvent(InputEvent input) {
  std::vector<OutputEvent> synthesized_outputs;
  std::vector<OutputEvent> main_outputs;

  KeyPhase input_phase = StateFor(input.logical_key);
  if (input.type == Type::kDown && IsPressed(input_phase)) {

  }
  return {};
}

KeyEventRegulator::KeyPhase KeyEventRegulator::StateFor(uint64_t key) {
  auto found = key_states.find(key);
  if (found == key_states.end()) {
    return KeyEventRegulator::kReleased;
  }
  return found->second;
}


}  // namespace flutter
