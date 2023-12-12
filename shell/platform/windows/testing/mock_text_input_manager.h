// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstring>
#include <optional>

#include "flutter/fml/macros.h"
#include "flutter/shell/platform/windows/text_input_manager.h"
#include "gmock/gmock.h"

namespace flutter {
namespace testing {

/// Mock for the |Window| base class.
class MockTextInputManager : public TextInputManager {
 public:
  MockTextInputManager();
  virtual ~MockTextInputManager();

  MOCK_METHOD(std::optional<std::u16string>,
              GetComposingString,
              (),
              (const, override));
  MOCK_METHOD(std::optional<std::u16string>,
              GetResultString,
              (),
              (const, override));
  MOCK_METHOD(long, GetComposingCursorPosition, (), (const, override));

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(MockTextInputManager);
};

}  // namespace testing
}  // namespace flutter
