// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_UI_TESTING_VIEWS_COORDINATE_TEST_VIEW_H_
#define SRC_UI_TESTING_VIEWS_COORDINATE_TEST_VIEW_H_

#include "src/ui/testing/views/background_view.h"
#include "src/ui/testing/views/color.h"

namespace scenic {

// Draws the following coordinate test pattern in a view:
//
// ___________________________________
// |                |                |
// |     BLACK      |        RED     |
// |           _____|_____           |
// |___________|  GREEN  |___________|
// |           |_________|           |
// |                |                |
// |      BLUE      |     MAGENTA    |
// |________________|________________|
//
class CoordinateTestView : public BackgroundView {
 public:
  static constexpr scenic::Color kUpperLeft = {0, 0, 0, 255}, kUpperRight = {0, 0, 255, 255},
                                 kLowerLeft = {255, 0, 0, 255}, kLowerRight = {255, 0, 255, 255},
                                 kCenter = {0, 255, 0, 255};

  CoordinateTestView(ViewContext context, const std::string& debug_name = "CoordinateTestView");

 private:
  // |BackgroundView|
  void Draw(float cx, float cy, float sx, float sy) override;
};

}  // namespace scenic
#endif  // SRC_UI_TESTING_VIEWS_COORDINATE_TEST_VIEW_H_
