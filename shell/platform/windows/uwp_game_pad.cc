// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/uwp_game_pad.h"

namespace flutter {

WinRTGamePad::WinRTGamePad(DualAxisCallback leftstick,
                           DualAxisCallback rightstick,
                           SingleAxisCallback lefttrigger,
                           SingleAxisCallback righttrigger,
                           ButtonCallback pressedcb,
                           ButtonCallback releasedcb,
                           GamepadAddedRemovedCallback changedcb)
    : pressed_buttons_(winrt::Windows::Gaming::Input::GamepadButtons::None),
      left_stick_x_value_(0),
      left_stick_y_value_(0),
      right_stick_x_value_(0),
      right_stick_y_value_(0),
      left_trigger_value_(0),
      right_trigger_value_(0) {
  left_stick_callback_ = leftstick;
  right_stick_callback_ = rightstick;

  left_trigger_callback_ = lefttrigger;
  right_trigger_callback_ = righttrigger;

  button_pressed_callback_ = pressedcb;
  button_released_callback_ = releasedcb;

  arrival_departure_callback_ = changedcb;
}

void WinRTGamePad::Initialize() {
  RefreshCachedGamepads();

  winrt::Windows::Gaming::Input::Gamepad::GamepadAdded(
      {this, &WinRTGamePad::OnGamepadAdded});
  winrt::Windows::Gaming::Input::Gamepad::GamepadRemoved(
      {this, &WinRTGamePad::OnGamepadRemoved});

  current_game_pad_ = GetLastGamepad();
  current_gamepad_needs_refresh_ = false;
}

bool WinRTGamePad::HasController() {
  return current_game_pad_ != nullptr;
}

void WinRTGamePad::OnGamepadAdded(
    winrt::Windows::Foundation::IInspectable const& th,
    winrt::Windows::Gaming::Input::Gamepad const& args) {
  enumerated_game_pads_.push_back(args);
  auto most_recent = GetLastGamepad();
  if (current_game_pad_ != most_recent) {
    current_game_pad_ = most_recent;
  }
  if (this->arrival_departure_callback_ != nullptr) {
    arrival_departure_callback_();
  }
}

void WinRTGamePad::OnGamepadRemoved(
    winrt::Windows::Foundation::IInspectable const&,
    winrt::Windows::Gaming::Input::Gamepad const& /*args*/) {
  RefreshCachedGamepads();
}

void WinRTGamePad::RefreshCachedGamepads() {
  // enumerated_game_pads_.clear();
  // auto gamepads = winrt::Windows::Gaming::Input::Gamepad::Gamepads();
  // for (auto gamepad : gamepads) {
  //  enumerated_game_pads_.push_back(gamepad);
  //}
}

const winrt::Windows::Gaming::Input::Gamepad* WinRTGamePad::GetLastGamepad() {
  winrt::Windows::Gaming::Input::Gamepad* gamepad = nullptr;

  if (enumerated_game_pads_.size() > 0) {
    gamepad = &enumerated_game_pads_.back();
  }

  return gamepad;
}

bool isValid(double value) {
  if (value > 0.1 || value < -0.1) {
    return true;
  }
  return false;
}

void WinRTGamePad::Process() {
  if (current_gamepad_needs_refresh_) {
    auto mostRecentGamepad = GetLastGamepad();
    if (current_game_pad_ != mostRecentGamepad) {
      current_game_pad_ = mostRecentGamepad;
    }
    current_gamepad_needs_refresh_ = false;
  }

  if (current_game_pad_ == nullptr) {
    return;
  }

  last_reading_ = current_game_pad_->GetCurrentReading();

  int exitComboPressed = 0;

  namespace gi = winrt::Windows::Gaming::Input;

  GamePadButtonPressedInternal(last_reading_.Buttons);

  if (last_reading_.LeftThumbstickX != left_stick_x_value_) {
    left_stick_x_value_ = last_reading_.LeftThumbstickX;
  }

  if (last_reading_.LeftThumbstickY != left_stick_y_value_) {
    left_stick_y_value_ = last_reading_.LeftThumbstickY;
  }

  if (isValid(left_stick_x_value_) || isValid(left_stick_y_value_)) {
    RaiseLeftStickMoved(left_stick_x_value_, left_stick_y_value_);
  }

  if (last_reading_.RightThumbstickX != right_stick_x_value_) {
    right_stick_x_value_ = last_reading_.RightThumbstickX;
  }

  if (last_reading_.RightThumbstickY != right_stick_y_value_) {
    right_stick_y_value_ = last_reading_.RightThumbstickY;
  }

  if (isValid(right_stick_x_value_) || isValid(right_stick_y_value_)) {
    RaiseRightStickMoved(right_stick_x_value_, right_stick_y_value_);
  }

  if (last_reading_.LeftTrigger != 0 &&
      last_reading_.LeftTrigger != left_trigger_value_) {
    left_trigger_value_ = last_reading_.LeftTrigger;
    RaiseLeftTriggerMoved(left_trigger_value_);
  }

  if (last_reading_.RightTrigger != 0 &&
      last_reading_.RightTrigger != right_trigger_value_) {
    right_trigger_value_ = last_reading_.RightTrigger;
    RaiseRightTriggerMoved(right_trigger_value_);
  }
}

void WinRTGamePad::GamePadButtonPressedInternal(
    winrt::Windows::Gaming::Input::GamepadButtons state) {
  namespace wgi = winrt::Windows::Gaming::Input;

  static const wgi::GamepadButtons AllButtons[] = {
      wgi::GamepadButtons::A,         wgi::GamepadButtons::B,
      wgi::GamepadButtons::DPadDown,  wgi::GamepadButtons::DPadLeft,
      wgi::GamepadButtons::DPadRight, wgi::GamepadButtons::DPadUp};

  for (const auto e : AllButtons) {
    // if button is pressed
    if ((e & state) == e) {
      // if we have not sent a pressed already
      if ((pressed_buttons_ & e) != e) {
        // send pressed callback
        if (button_pressed_callback_ != nullptr) {
          button_pressed_callback_(e);
        }

        // set the bit
        pressed_buttons_ |= e;
      }
    }

    // if button is not pressed
    if ((e & state) != e) {
      // if we have sent a pressed already
      if ((pressed_buttons_ & e) == e) {
        // send callback
        if (button_released_callback_ != nullptr) {
          button_released_callback_(e);
        }

        // set the pressed bit
        pressed_buttons_ &= ~e;
      }
    }
  }
}

void WinRTGamePad::RaiseGameGamePadButtonPressed(
    winrt::Windows::Gaming::Input::GamepadButtons b) {}

void WinRTGamePad::RaiseGameGamePadButtonReleased(
    winrt::Windows::Gaming::Input::GamepadButtons b) {}

void WinRTGamePad::RaiseLeftStickMoved(double x, double y) {
  if (left_stick_callback_ != nullptr) {
    left_stick_callback_(x, y);
  }
}

void WinRTGamePad::RaiseRightStickMoved(double x, double y) {
  if (right_stick_callback_ != nullptr) {
    right_stick_callback_(x, y);
  }
}

void WinRTGamePad::RaiseLeftTriggerMoved(double value) {
  if (left_trigger_callback_ != nullptr) {
    left_trigger_callback_(value);
  }
}

void WinRTGamePad::RaiseRightTriggerMoved(double value) {
  if (right_trigger_callback_ != nullptr) {
    right_trigger_callback_(value);
  }
}

}  // namespace flutter
