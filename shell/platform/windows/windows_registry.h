// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOWS_REGISTRY_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOWS_REGISTRY_H_

#include <Windows.h>

#include <string>
#include <vector>

namespace flutter {

class WindowsRegistry {
  public:
    virtual ~WindowsRegistry();

    virtual LSTATUS GetRegistryValue(HKEY hkey, LPCSTR key, LPCSTR value, DWORD flags, LPDWORD type, PVOID data, LPDWORD sizeData) const;
  private:
};

}

#endif
