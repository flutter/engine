// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_KEY_EVENT_REGULARIZER_H_
#define COMMON_KEY_EVENT_REGULARIZER_H_

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

#include "flutter/fml/macros.h"

namespace flutter {

class KeyEventRegulator {
 public:
  // For two-phase keys, 0 is released, 1 is pressed.
  // For four-phase keys, 0 and 4 are disabled, 1 and 2 are enabled.
  typedef uint8_t KeyPhase;

  enum class Type {
    kUnknown = 0,
    kUp,
    kDown,
    kRepeat,
  };

  static constexpr uint64_t kSynonymPlane = 0x0F00000000;

  struct SynonymKey {
    // Should has kSynonymPlane as the high 32 bits.
    uint64_t id;
    std::vector<uint64_t> logical_keys;
    // Must be of the same length as `logical_keys`.
    std::vector<uint64_t> default_physical_keys;
  };

  struct InputEvent {
    Type type;
    uint64_t physical_key;
    uint64_t logical_key;
    std::string characters;
  };

  struct OutputEvent {
    Type type;
    uint64_t physical_key;
    uint64_t logical_key;
    std::string characters;
    bool synthesized;
  };

  enum class DuplicateDownBehavior {
    kIgnore = 0,
    kSynthesizeRepeat,
    kSynthesizeUp,
  };

  enum class AbruptUpBehavior {
    kIgnore = 0,
    kSynthesizeDown,
  };

  struct Config {
    DuplicateDownBehavior on_duplicate_down;

    AbruptUpBehavior on_abrupt_up;

    // Declare that multiple logical keys can be recognized as the same
    // goal.
    std::vector<SynonymKey> synonym_keys;

    // Declare that a goal, whether a logical key or a synonym key, has a
    // toggling state, and therefore has 4 phases.
    std::vector<uint64_t> toggling_keys;
  };

  explicit KeyEventRegulator(Config config);

  void ReportState(std::vector<std::pair<uint64_t, KeyPhase>> goal_states);
  std::vector<OutputEvent> SettleByEvent(InputEvent event);
  // std::vector<OutputEvent> Settle();

 private:
  std::map<uint64_t, uint64_t> recorded_physical_keys;
  std::map<uint64_t, KeyPhase> key_states;
  std::map<uint64_t, KeyPhase> unsettled_states;

  FML_DISALLOW_COPY_AND_ASSIGN(KeyEventRegulator);
};

}  // namespace flutter

#endif  // COMMON_KEY_EVENT_REGULARIZER_H_
