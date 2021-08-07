// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/testing/test_keyboard.h"

#include <windowsx.h>

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

namespace {
std::string _print_character(const char* s) {
  if (s == nullptr) {
    return "nullptr";
  }
  return std::string("\"") + s + "\"";
}
} // namespace

#define _RETURN_IF_NOT_EQUALS(val1, val2)                                     \
  if ((val1) != (val2)) {                                                     \
    return ::testing::AssertionFailure()                                      \
           << "Expected equality of these values:\n  " #val1 "\n    To be:  " \
           << val2 << "\n    Actual: \n    " << val1;                         \
  }

::testing::AssertionResult _EventEquals(const char* expr_event,
                                        const char* expr_expected,
                                        const FlutterKeyEvent& event,
                                        const FlutterKeyEvent& expected) {
  _RETURN_IF_NOT_EQUALS(event.struct_size, sizeof(FlutterKeyEvent));
  _RETURN_IF_NOT_EQUALS(event.type, expected.type);
  _RETURN_IF_NOT_EQUALS(event.physical, expected.physical);
  _RETURN_IF_NOT_EQUALS(event.logical, expected.logical);
  if ((event.character == nullptr) != (expected.character == nullptr) ||
      strcmp(event.character, expected.character) != 0) {
    return ::testing::AssertionFailure()
           << "Expected equality of these values:\n  expected.character\n    "
           << _print_character(expected.character) << "\n  Actual: \n    "
           << _print_character(event.character);
  }
  _RETURN_IF_NOT_EQUALS(event.synthesized, expected.synthesized);
  return ::testing::AssertionSuccess();
}


}  // namespace testing
}  // namespace flutter
