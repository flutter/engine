// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/display_helper_winuwp.h"

namespace flutter {

DisplayHelperWinUWP::DisplayHelperWinUWP() {
  current_display_info_ = winrt::Windows::Graphics::Display::
      DisplayInformation::GetForCurrentView();

  ConfigureXboxSpecific();
}

bool DisplayHelperWinUWP::IsRunningOnXbox() {
  return running_on_xbox_;
}

float DisplayHelperWinUWP::GetXboxOverscanXOffset() {
  return xbox_overscan_x_offset_;
}

float DisplayHelperWinUWP::GetXboxOverscanYOffset() {
  return xbox_overscan_y_offset_;
}

WindowBoundsWinUWP DisplayHelperWinUWP::GetBounds(bool physical) {
  return GetBounds(current_display_info_, physical);
}

WindowBoundsWinUWP DisplayHelperWinUWP::GetBounds(
    winrt::Windows::Graphics::Display::DisplayInformation const& disp,
    bool physical) {
  winrt::Windows::UI::ViewManagement::ApplicationView app_view =
      winrt::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
  winrt::Windows::Foundation::Rect bounds = app_view.VisibleBounds();
  if (running_on_xbox_) {
    return {bounds.Width + (bounds.X), bounds.Height + (bounds.Y)};
  }

  if (physical) {
    // Return the height in physical pixels
    return {bounds.Width * static_cast<float>(disp.RawPixelsPerViewPixel()),
            bounds.Height * static_cast<float>(disp.RawPixelsPerViewPixel())};
  }

  return {bounds.Width, bounds.Height};
}

void DisplayHelperWinUWP::ConfigureXboxSpecific() {
  running_on_xbox_ =
      winrt::Windows::System::Profile::AnalyticsInfo::VersionInfo()
          .DeviceFamily() == L"Windows.Xbox";

  if (running_on_xbox_) {
    bool result =
        winrt::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()
            .SetDesiredBoundsMode(winrt::Windows::UI::ViewManagement::
                                      ApplicationViewBoundsMode::UseCoreWindow);
    if (!result) {
      OutputDebugString(L"Couldn't set bounds mode.");
    }

    winrt::Windows::UI::ViewManagement::ApplicationView app_view = winrt::
        Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
    winrt::Windows::Foundation::Rect bounds = app_view.VisibleBounds();

    // the offset /2 represents how much off-screan the core window is
    // positioned unclear why disabling overscan doesn't correct this
    xbox_overscan_x_offset_ = bounds.X / 2;
    xbox_overscan_y_offset_ = bounds.Y / 2;
  }
}

float DisplayHelperWinUWP::GetDpiScale() {
  double raw_per_view = current_display_info_.RawPixelsPerViewPixel();

  // TODO(clarkezone): ensure DPI handling is correct:
  // because XBOX has display scaling off, logicalDpi retuns 96 which is
  // incorrect check if raw_per_view is more acurate.
  // Also confirm if it is necessary to use this workaround on 10X
  // https://github.com/flutter/flutter/issues/70198

  if (running_on_xbox_) {
    return 1.5;
  }
  return static_cast<float>(raw_per_view);
}

}  // namespace flutter
