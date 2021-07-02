// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/playground/playground.h"

namespace impeller {
namespace testing {

TEST_F(Playground, CanCreateEntity) {
  OpenPlaygroundHere([]() { return true; });
}

}  // namespace testing
}  // namespace impeller
