// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/key_event_regulator.h"

#include <set>
#include "flutter/fml/logging.h"

namespace flutter {

namespace {
typedef KeyEventRegulator::Phase Phase;
typedef KeyEventRegulator::Type EventType;

bool IsSynonymKey(uint64_t key) {
  constexpr uint64_t kPlaneMask = 0xFF00000000;
  return (key & kPlaneMask) == KeyEventRegulator::kSynonymPlane;
}

template <class T>
bool NoneIsWithin(const std::vector<uint64_t>& none_of,
                  const std::map<uint64_t, T>& is_within) {
  for (uint64_t testee : none_of) {
    if (is_within.find(testee) != is_within.end()) {
      return false;
    }
  }
  return true;
}

KeyEventRegulator::Phase StateToPhase(bool pressed, bool toggled) {
  return ((pressed ? 0 : 1) + (toggled ? 1 : 3)) % KeyEventRegulator::kMaxPhase;
}

bool OfCompatibleTypes(EventType input_type, EventType event_type) {
  switch (input_type) {
    case EventType::kDown:
    case EventType::kRepeat:
      return event_type == EventType::kDown || event_type == EventType::kRepeat;
    case EventType::kUp:
      return event_type == EventType::kUp;
    case EventType::kUnknown:
      return event_type != EventType::kRepeat;
  }
}

} // namespace

KeyEventRegulator::KeyEventRegulator(Config config)
    : on_duplicate_down(config.on_duplicate_down),
      on_abrupt_up(config.on_abrupt_up) {
  for (uint64_t toggling_key : config.toggling_keys) {
    FML_CHECK(!IsSynonymKey(toggling_key));
    key_states[toggling_key] = KeyState{.is_toggling = true};
  }
  for (const SynonymKey& synonym_key : config.synonym_keys) {
    FML_CHECK(synonym_key.concrete_keys.size() > 1);
    // Ensure that no concrete key is toggling, since only toggling keys are
    // initialized at this moment.
    FML_CHECK(NoneIsWithin(synonym_key.concrete_keys, key_states));
    synonym_keys[synonym_key.id] = synonym_key.concrete_keys;
  }
}

std::vector<KeyEventRegulator::OutputEvent> KeyEventRegulator::SettleByEvent(InputEvent input) {

  // Resolve pending state of synonym keys' into concrete keys'.
  for (const auto& pending_item : pending_synonym_pressing) {
    // `pending_item.first` is guaranteed by ReportPressed a valid toggling key.
    const auto& concrete_keys = synonym_keys[pending_item.first];
    bool should_be_pressed = pending_item.second;
    if (should_be_pressed) {
      // The key should be pressed. If any concrete key is pressed, no changes
      // are needed. Otherwise, set the first concrete key as pressed.
      bool any_pressed = false;
      for (uint64_t concrete_key : concrete_keys) {
        auto found = key_states.find(concrete_key);
        bool overall_pressed;
        if (found == key_states.end()) {
          overall_pressed = false;
        } else {
          const auto& key_state = found->second;
          FML_CHECK(!key_state.is_toggling);
          overall_pressed =
              key_state.dirty ? key_state.pending.pressed : key_state.pressed;
        }
        if (overall_pressed) {
          any_pressed = true;
          break;
        }
      }
      if (!any_pressed) {
        ReportPressed(concrete_keys[0], true);
      }
    } else {
      // The key should not be pressed, so set all concrete states to false.
      for (uint64_t concrete_key : concrete_keys) {
        ReportPressed(concrete_key, false);
      }
    }
  }
  pending_synonym_pressing.clear();

  // Always mark the input key as dirty.
  MarkAsDirty(key_states.try_emplace(input.logical_key, KeyState{}).first);

  std::vector<OutputEvent> synthesized_outputs;
  std::vector<OutputEvent> main_outputs;

  // Synthesize events for each key.
  for (const auto& pending_item : dirty_keys) {
    uint64_t key = pending_item.first;
    KeyState& key_state = *pending_item.second;
    bool is_input_key = key == input.logical_key;

    std::vector<Type> following_events;
    bool no_main_event = false;

    if (is_input_key) {
      key_state.last_seen_physical_key = input.physical_key;

      // Ensure that the state matches the type of the input event.
      switch(input.type) {
        case kRepeat:
        case kDown:
          if (key_state.is_toggling) {
            ReportPhase(key, {kPressedEnabled, kPressedDisabled});
          } else {
            ReportPressed(key, true);
            // Resolve duplicate down.
            if (key_state.pressed && input.type == kDown) {
              switch(on_duplicate_down) {
                case DuplicateDownBehavior::kSynthesizeRepeat:
                  following_events.push_back(kRepeat);
                  break;
                case DuplicateDownBehavior::kSynthesizeUp:
                  break;
                case DuplicateDownBehavior::kIgnore:
                  no_main_event = true;
                  break;
              }
            }
          }
          break;
        case kUp:
          if (key_state.is_toggling) {
            ReportPhase(key, {kReleasedEnabled, kReleasedDisabled});
          } else {
            ReportPressed(key, false);
            // Resolve abrupt up.
            if (!key_state.pressed) {
              switch(on_abrupt_up) {
                case AbruptUpBehavior::kIgnore:
                  no_main_event = true;
                  break;
                case AbruptUpBehavior::kSynthesizeDown:
                  break;
              }
            }
          }
          break;
        case kUnknown:
          no_main_event = true;
          break;
      }
    }

    // Synthesize events that lead from the current to the target state.
    if (key_state.is_toggling) {
      TransitToggling(following_events, key_state, key_state.pending.toggled);
    } else {
      TransitPressing(following_events, key_state, key_state.pending.pressed);
    }

    // Ensure that the input key always has some events by pushing a full cycle.
    if (is_input_key && !no_main_event && following_events.size() == 0) {
      Type first_type = key_state.pressed ? kUp : kDown;
      Type second_type = key_state.pressed == kUp ? kDown : kUp;
      following_events.push_back(first_type);
      following_events.push_back(second_type);
      if (key_state.is_toggling) {
        following_events.push_back(first_type);
        following_events.push_back(second_type);
      }
    }

    FML_CHECK(StateTransitionIsRegular(key_state, following_events));

    // Allow the only down event to be a repeat.
    if (following_events.size() == 1 && input.type == kRepeat) {
      following_events[0] = kRepeat;
    }

    uint64_t physical_key = key_state.pressed
                                ? key_state.pressed_physical_key
                                : key_state.last_seen_physical_key;
     // Generate events.
    for (Type type : following_events) {
      synthesized_outputs.push_back(OutputEvent{
        .type = type,
        .physical_key = physical_key,
        .logical_key = key,
        .synthesized = true,
      });
      physical_key = key_state.last_seen_physical_key;
      key_state.ForwardPhase();
    }
    key_state.dirty = false;
    key_state.pressed_physical_key = physical_key;
    // Possibly change the last event to be a main event.
    if (is_input_key && !no_main_event) {
      main_outputs.push_back(synthesized_outputs.back());
      synthesized_outputs.pop_back();
      main_outputs.back().synthesized = false;
      FML_CHECK(OfCompatibleTypes(main_outputs.back().type, input.type));
    }
  }

  synthesized_outputs.insert(synthesized_outputs.end(), main_outputs.begin(), main_outputs.end());
  dirty_keys.clear();

  return synthesized_outputs;
}

void KeyEventRegulator::ReportPressed(uint64_t key, bool pressed) {
  if (IsSynonymKey(key)) {
    pending_synonym_pressing[key] = pressed;
  } else {
    auto found = key_states.try_emplace(key, KeyState{}).first;
    FML_CHECK(!found->second.is_toggling);
    found->second.pending.pressed = pressed;
    MarkAsDirty(found);
  }
}

void KeyEventRegulator::ReportPhase(uint64_t key, std::vector<Phase> phases) {
  FML_CHECK(!IsSynonymKey(key));
  auto found = key_states.find(key);
  FML_CHECK(found != key_states.end());
  FML_CHECK(found->second.is_toggling);
  found->second.pending.toggled = {false, false, false, false};
  for (Phase phase : phases) {
    found->second.pending.toggled[phase] = true;
  }
  MarkAsDirty(found);
}

void KeyEventRegulator::MarkAsDirty(std::map<uint64_t, KeyState>::iterator key_state_iter) {
  if (!key_state_iter->second.dirty) {
    key_state_iter->second.dirty = true;
    dirty_keys[key_state_iter->first] = &key_state_iter->second;
  }
}

void KeyEventRegulator::KeyState::ForwardPhase() {
  if (is_toggling && !pressed) {
    toggled = !toggled;
  }
  pressed = !pressed;
}

void KeyEventRegulator::TransitToggling(
    std::vector<Type>& result,
    const KeyState& key_state,
    const std::array<bool, kMaxPhase>& allowed_phases) {
  size_t from_phase = StateToPhase(key_state.pressed, key_state.toggled);
  for (size_t elapsed = 0; elapsed < kMaxPhase; elapsed += 1) {
    size_t current_phase = (from_phase + elapsed) % kMaxPhase;
    if (allowed_phases[current_phase]) {
      break;
    }
    result.push_back((current_phase == kReleasedDisabled ||
                      current_phase == kReleasedEnabled)
                         ? kDown
                         : kUp);
  }
  FML_CHECK(result.size() < kMaxPhase);
}

void KeyEventRegulator::TransitPressing(
    std::vector<Type>& result,
    const KeyState& key_state,
    bool pending_pressed) {
  if (key_state.pressed && !pending_pressed) {
    result.push_back(kUp);
  } else if (!key_state.pressed && pending_pressed) {
    result.push_back(kDown);
  }
}

bool KeyEventRegulator::StateTransitionIsRegular(
    const KeyState& state,
    const std::vector<Type>& following_events) {
  bool pressed = state.pressed;
  bool toggled = state.toggled;
  for (Type type : following_events) {
    switch (type) {
      case kUp:
        if (!pressed) {
          return false;
        }
        pressed = !pressed;
        break;
      case kDown:
        if (pressed) {
          return false;
        }
        toggled = !toggled;
        pressed = !pressed;
        break;
      case kRepeat:
        if (!pressed) {
          return false;
        }
        break;
      default:
        return false;
    }
  }
  if (state.is_toggling) {
    return state.pending.toggled[StateToPhase(pressed, toggled)];
  } else {
    return state.pending.pressed == pressed;
  }
}


}  // namespace flutter
