// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Windows.h"

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_DPI_UTILS_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_DPI_UTILS_H_

namespace flutter {

/// Returns the current DPI. Supports all DPI awareness modes, and is backward
/// compatible down to Windows Vista. If |hwnd| is nullptr, returns the DPI for
/// the nearest monitor is available. Otherwise, returns the system's DPI.
UINT GetDpiForHWND(HWND hwnd);

/// Enables scaling of non-client UI (scrolling bars, title bars, etc). Only
/// supported on Per-Monitor V1 DPI awareness mode.
BOOL EnableNonClientDpiScaling(HWND hwnd);

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_DPI_UTILS_H_
