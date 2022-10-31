// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterGLCompositor.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewControllerTestUtils.h"
#import "flutter/testing/testing.h"

namespace flutter::testing {
namespace {

FlutterViewProvider* MockViewProvider() {
  id viewMock = OCMClassMock([FlutterView class]);
  FlutterViewProvider* viewProviderMock = OCMStrictClassMock([FlutterViewProvider class]);
  OCMStub([viewProviderMock getView:kFlutterDefaultViewId]).ignoringNonObjectArgs().andReturn(viewMock);
  return viewProviderMock;
}
}  // namespace

TEST(FlutterGLCompositorTest, TestPresent) {
  std::unique_ptr<flutter::FlutterGLCompositor> macos_compositor =
      std::make_unique<FlutterGLCompositor>(MockViewProvider(), nullptr);

  bool flag = false;
  macos_compositor->SetPresentCallback([f = &flag](bool has_flutter_content) {
    *f = true;
    return true;
  });

  ASSERT_TRUE(macos_compositor->Present(nil, 0));
  ASSERT_TRUE(flag);
}

}  // namespace flutter::testing
