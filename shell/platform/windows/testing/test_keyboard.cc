// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/testing/test_keyboard.h"

namespace flutter {
namespace testing {

char* clone_string(const char* string) {
  if (string == nullptr) {
    return nullptr;
  }
  size_t len = strlen(string);
  char* result = new char[len + 1];
  strcpy(result, string);
  return result;
}

}  // namespace testing
}  // namespace flutter
