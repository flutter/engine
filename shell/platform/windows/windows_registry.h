// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOWS_REGISTRY_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOWS_REGISTRY_H_

#include <Windows.h>

#include "flutter/fml/macros.h"

namespace flutter {

class WindowsRegistry {
 public:
  explicit WindowsRegistry();
  virtual ~WindowsRegistry();

  virtual LSTATUS GetRegistryValue(HKEY hkey,
                                   LPCWSTR key,
                                   LPCWSTR value,
                                   DWORD flags,
                                   LPDWORD type,
                                   PVOID data,
                                   LPDWORD sizeData) const;

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(WindowsRegistry);
};

}  // namespace flutter

#endif
