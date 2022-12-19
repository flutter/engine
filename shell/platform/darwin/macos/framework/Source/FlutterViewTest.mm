// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"

#import <Metal/Metal.h>
#import <OCMock/OCMock.h>

#import "flutter/testing/testing.h"

@interface TestReshapeListener : NSObject <FlutterViewReshapeListener>

@end

@implementation TestReshapeListener

- (void)viewDidReshape:(nonnull NSView*)view {
}

@end

TEST(FlutterView, BackingPropertiesChangeUpdatesLayerScale) {
  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  id<MTLCommandQueue> queue = [device newCommandQueue];
  TestReshapeListener* listener = [[TestReshapeListener alloc] init];
  FlutterView* view = [[FlutterView alloc] initWithMTLDevice:device
                                                commandQueue:queue
                                             reshapeListener:listener];

  view.layer.contentsScale = 1.0;

  FlutterView* mockView = OCMPartialMock(view);

  NSWindow* mockWindow = OCMClassMock([NSWindow class]);
  OCMStub([mockWindow backingScaleFactor]).andReturn(3.0);

  OCMStub([mockView window]).andReturn(mockWindow);

  EXPECT_EQ(view.layer.contentsScale, 1.0);

  [mockView viewDidChangeBackingProperties];

  EXPECT_EQ(view.layer.contentsScale, 3.0);
}
