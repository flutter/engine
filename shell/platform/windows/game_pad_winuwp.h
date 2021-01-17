// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_GAME_PAD_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_GAME_PAD_H_

#include <winrt/Windows.Gaming.Input.h>
#include <functional>
#include <vector>

namespace flutter {

using SingleAxisCallback = std::function<void(double)>;
using DualAxisCallback = std::function<void(double, double)>;
using ButtonCallback =
    std::function<void(winrt::Windows::Gaming::Input::GamepadButtons)>;
using GamepadAddedRemovedCallback = std::function<void()>;

class GamePadWinUWP {
 public:
  GamePadWinUWP(DualAxisCallback,
               DualAxisCallback,
               SingleAxisCallback,
               SingleAxisCallback,
               ButtonCallback,
               ButtonCallback,
               GamepadAddedRemovedCallback);

  void Initialize();

  void Process();

  bool HasController();

 private:
  const winrt::Windows::Gaming::Input::Gamepad* GetLastGamepad();

  void OnGamepadAdded(winrt::Windows::Foundation::IInspectable const& sender,
                      winrt::Windows::Gaming::Input::Gamepad const& args);
  void OnGamepadRemoved(winrt::Windows::Foundation::IInspectable const& sender,
                        winrt::Windows::Gaming::Input::Gamepad const& args);
  void RefreshCachedGamepads();

  void GamePadButtonPressedInternal(
      winrt::Windows::Gaming::Input::GamepadButtons b);

  void RaiseLeftStickMoved(double x, double y);
  void RaiseRightStickMoved(double x, double y);
  void RaiseLeftTriggerMoved(double value);
  void RaiseRightTriggerMoved(double value);

  void RaiseGameGamePadButtonPressed(
      winrt::Windows::Gaming::Input::GamepadButtons);

  void RaiseGameGamePadButtonReleased(
      winrt::Windows::Gaming::Input::GamepadButtons);

  std::vector<winrt::Windows::Gaming::Input::Gamepad> enumerated_game_pads_;
  winrt::Windows::Gaming::Input::GamepadReading last_reading_;
  const winrt::Windows::Gaming::Input::Gamepad* current_game_pad_;

  bool current_gamepad_needs_refresh_;
  double left_trigger_value_;
  double right_trigger_value_;
  double left_stick_x_value_;
  double left_stick_y_value_;
  double right_stick_x_value_;
  double right_stick_y_value_;

  DualAxisCallback left_stick_callback_;
  DualAxisCallback right_stick_callback_;
  SingleAxisCallback left_trigger_callback_;
  SingleAxisCallback right_trigger_callback_;

  ButtonCallback button_pressed_callback_;
  ButtonCallback button_released_callback_;
  GamepadAddedRemovedCallback arrival_departure_callback_;

  winrt::Windows::Gaming::Input::GamepadButtons pressed_buttons_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_GAME_PAD_H_
