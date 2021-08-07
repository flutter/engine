// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_TESTING_TEST_KEYBOARD_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_TESTING_TEST_KEYBOARD_H_

#include <windowsx.h>

#include <string>

#include "flutter/shell/platform/embedder/embedder.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

// Clone string onto the heap.
//
// If #string is nullptr, returns nullptr. Otherwise, the returned pointer must
// be freed with delete[].
char* clone_string(const char* string);

::testing::AssertionResult _EventEquals(const char* expr_event,
                                        const char* expr_expected,
                                        const FlutterKeyEvent& event,
                                        const FlutterKeyEvent& expected);

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
