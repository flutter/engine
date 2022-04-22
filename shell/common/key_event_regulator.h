// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_KEY_EVENT_REGULARIZER_H_
#define COMMON_KEY_EVENT_REGULARIZER_H_

#include <array>
#include <cinttypes>
#include <string>
#include <vector>
#include <map>

#include "flutter/fml/macros.h"

namespace flutter {

class KeyEventRegulator {
 public:
  typedef uint8_t Phase;

  static constexpr bool kReleased = false;
  static constexpr bool kPressed = true;
  static constexpr Phase kReleasedDisabled = 0;
  static constexpr Phase kPressedEnabled = 1;
  static constexpr Phase kReleasedEnabled = 2;
  static constexpr Phase kPressedDisabled = 3;

  static constexpr size_t kMaxPhase = 4;

  enum Type : uint8_t {
    kUnknown = 0,
    kUp,
    kDown,
    kRepeat,
  };

  static constexpr uint64_t kSynonymPlane = 0x0F00000000;

  struct SynonymKey {
    // Should has kSynonymPlane as the high 32 bits.
    uint64_t id;
    std::vector<uint64_t> concrete_keys;
    // // Must be of the same length as `concrete_keys`.
    // std::vector<uint64_t> default_physical_keys;
  };

  struct InputEvent {
    Type type;
    uint64_t physical_key;
    uint64_t logical_key;
  };

  struct OutputEvent {
    Type type;
    uint64_t physical_key;
    uint64_t logical_key;
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
    //
    // The concrete keys must not be toggling keys.
    std::vector<SynonymKey> synonym_keys;

    // Declare that a goal, whether a logical key or a synonym key, has a
    // toggling state, and therefore has 4 phases.
    //
    // Must not have synonym keys.
    std::vector<uint64_t> toggling_keys;
  };

  explicit KeyEventRegulator(Config config);

  void ReportPressed(uint64_t key, bool pressed);
  void ReportPhase(uint64_t key, std::vector<Phase> phase);
  std::vector<OutputEvent> SettleByEvent(InputEvent event);
  // std::vector<OutputEvent> Settle();

 private:
  struct KeyState {
    uint64_t pressed_physical_key;
    uint64_t last_seen_physical_key;
    bool is_toggling;
    bool pressed;
    bool toggled;
    bool dirty;

    union {
      bool pressed;
      std::array<bool, kMaxPhase> toggled;
    } pending;

    void ForwardPhase();
  };

  DuplicateDownBehavior on_duplicate_down;
  AbruptUpBehavior on_abrupt_up;

  std::map<uint64_t, std::vector<uint64_t>> synonym_keys;
  std::map<uint64_t, KeyState> key_states;

  std::map<uint64_t, bool> pending_synonym_pressing;
  std::map<uint64_t, KeyState*> dirty_keys;

  void MarkAsDirty(std::map<uint64_t, KeyState>::iterator key_state_iter);

  static bool StateTransitionIsRegular(
      const KeyState& current_state,
      const std::vector<Type>& following_events);
  static void TransitToggling(
      std::vector<Type>& result,
      const KeyState& key_state,
      const std::array<bool, kMaxPhase>& allowed_phases);
  static void TransitPressing(std::vector<Type>& result,
                              const KeyState& key_state,
                              bool pending_pressed);

  FML_DISALLOW_COPY_AND_ASSIGN(KeyEventRegulator);
};

}  // namespace flutter

#endif  // COMMON_KEY_EVENT_REGULARIZER_H_
