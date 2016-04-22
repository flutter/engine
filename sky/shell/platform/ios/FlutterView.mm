// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/weak_ptr.h"
#include "sky/shell/platform/ios/accessibility_bridge.h"
#include "sky/shell/platform/ios/FlutterView.h"

@interface FlutterView ()<UIInputViewAudioFeedback>

@end

@implementation FlutterView {
  base::WeakPtr<sky::shell::AccessibilityBridge> _accessibilityBridge;
}

- (void)setSemanticsServer:(semantics::SemanticsServerPtr)semanticsServer {
  auto bridge =
      new sky::shell::AccessibilityBridge(self, semanticsServer.Pass());
  _accessibilityBridge = bridge->AsWeakPtr();
}

- (void)layoutSubviews {
  CGFloat screenScale = [UIScreen mainScreen].scale;
  CAEAGLLayer* layer = reinterpret_cast<CAEAGLLayer*>(self.layer);

  layer.allowsGroupOpacity = YES;
  layer.opaque = YES;
  layer.contentsScale = screenScale;
  layer.rasterizationScale = screenScale;

  [super layoutSubviews];
}

+ (Class)layerClass {
  return [CAEAGLLayer class];
}

- (BOOL)enableInputClicksWhenVisible {
  return YES;
}

- (BOOL)accessibilityActivate {
  // TODO(tvolkert): Implement
  return NO;
}

- (void)accessibilityIncrement {
  // TODO(tvolkert): Implement
}

- (void)accessibilityDecrement {
  // TODO(tvolkert): Implement
}

- (BOOL)accessibilityScroll:(UIAccessibilityScrollDirection)direction {
  // TODO(tvolkert): Implement
  return NO;
}

- (BOOL)accessibilityPerformEscape {
  // TODO(tvolkert): Implement
  return NO;
}

- (BOOL)accessibilityPerformMagicTap {
  // TODO(tvolkert): Implement
  return NO;
}

- (void)dealloc {
  delete _accessibilityBridge.get();
  _accessibilityBridge.reset();

  [super dealloc];
}

@end
