// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/windows_registry.h"

namespace flutter {

WindowsRegistry::WindowsRegistry() {}

LSTATUS WindowsRegistry::GetRegistryValue(HKEY hkey,
                                          LPCWSTR key,
                                          LPCWSTR value,
                                          DWORD flags,
                                          LPDWORD type,
                                          PVOID data,
                                          LPDWORD dataSize) const {
  return RegGetValue(hkey, key, value, flags, type, data, dataSize);
}

WindowsRegistry::~WindowsRegistry() {}

}  // namespace flutter
