// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_TESTING_TEST_KEYBOARD_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_TESTING_TEST_KEYBOARD_H_

#include <string>

#include "flutter/shell/platform/embedder/embedder.h"

#include "gtest/gtest.h"

#define _RETURN_IF_NOT_EQUALS(val1, val2)                                     \
  if ((val1) != (val2)) {                                                     \
    return ::testing::AssertionFailure()                                      \
           << "Expected equality of these values:\n  " #val1 "\n    To be:  " \
           << val2 << "\n    Actual: \n    " << val1;                         \
  }

namespace flutter {
namespace testing {

namespace {

std::string _print_character(const char* s) {
  if (s == nullptr) {
    return "nullptr";
  }
  return std::string("\"") + s + "\"";
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

}  // namespace

// Clone string onto the heap.
//
// If #string is nullptr, returns nullptr. Otherwise, the returned pointer must
// be freed with delete[].
char* clone_string(const char* string);

}  // namespace testing
}  // namespace flutter

#define EXPECT_EVENT_EQUALS(_target, _type, _physical, _logical, _character, \
                            _synthesized)                                    \
  EXPECT_PRED_FORMAT2(_EventEquals, _target,                                 \
                      (FlutterKeyEvent{                                      \
                          .type = _type,                                     \
                          .physical = _physical,                             \
                          .logical = _logical,                               \
                          .character = _character,                           \
                          .synthesized = _synthesized,                       \
                      }));

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_TESTING_TEST_KEYBOARD_H_
