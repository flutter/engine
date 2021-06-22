// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_UI_TESTING_VIEWS_TEST_VIEW_H_
#define SRC_UI_TESTING_VIEWS_TEST_VIEW_H_

#include <lib/ui/scenic/cpp/session.h>

namespace scenic {

// Represents a view that allows a callback to be set for its |Present|.
class TestView {
 public:
  virtual ~TestView() = default;
  virtual void set_present_callback(Session::PresentCallback present_callback) = 0;
};

}  // namespace scenic
#endif  // SRC_UI_TESTING_VIEWS_TEST_VIEW_H_
