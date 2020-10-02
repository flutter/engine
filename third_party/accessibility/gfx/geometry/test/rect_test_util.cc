// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/geometry/test/rect_test_util.h"

namespace gfx {
namespace test {

testing::AssertionResult RectContains(const gfx::Rect& outer_rect,
                                      const gfx::Rect& inner_rect) {
  if (outer_rect.Contains(inner_rect)) {
    return testing::AssertionSuccess()
           << "outer_rect (" << outer_rect.ToString()
           << ") does contain inner_rect (" << inner_rect.ToString() << ")";
  }
  return testing::AssertionFailure() << "outer_rect (" << outer_rect.ToString()
                                     << ") does not contain inner_rect ("
                                     << inner_rect.ToString() << ")";
}

}  // namespace test
}  // namespace gfx
