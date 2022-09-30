// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/windows_registry.h"

namespace flutter {

LSTATUS WindowsRegistry::GetRegistryValue(HKEY hkey, LPCSTR key, LPCSTR value, DWORD flags, LPDWORD type, PVOID data, LPDWORD dataSize) const {
  return RegGetValueA(hkey, key, value, flags, type, data, dataSize);
}

WindowsRegistry::~WindowsRegistry() {}

}
