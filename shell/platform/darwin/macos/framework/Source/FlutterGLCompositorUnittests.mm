// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterGLCompositor.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewControllerTestUtils.h"
#import "flutter/testing/testing.h"

namespace flutter::testing {

TEST(FlutterGLCompositorTest, TestPresent) {
  id mock_view = OCMClassMock([FlutterView class]);
  flutter::FlutterCompositor::ViewProvider get_view_callback = [&mock_view](uint64_t view_id) {
    return mock_view;
  };

  std::unique_ptr<flutter::FlutterGLCompositor> macos_compositor =
      std::make_unique<FlutterGLCompositor>(get_view_callback, nullptr);

  bool flag = false;
  macos_compositor->SetPresentCallback([f = &flag](bool has_flutter_content) {
    *f = true;
    return true;
  });

  ASSERT_TRUE(macos_compositor->Present(0, nil, 0));
  ASSERT_TRUE(flag);
}

}  // namespace flutter::testing
