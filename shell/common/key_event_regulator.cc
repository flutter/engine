// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/key_event_regulator.h"

#include "flutter/fml/logging.h"

namespace flutter {

namespace {

bool IsPressed(KeyEventRegulator::KeyPhase phase) {
  FML_CHECK(phase >= 0 && phase < 4);
  return phase == 1 || phase == 3;
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
    return 0;
  }
  return found->second;
}


}  // namespace flutter
