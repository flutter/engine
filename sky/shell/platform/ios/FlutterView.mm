// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/platform/ios/FlutterView.h"

@interface FlutterView ()<UIInputViewAudioFeedback>

@end

@implementation FlutterView {
  NSArray* _accessibleElements;
}

- (void)updateAccessibleElements:(NSArray*)accessibleElements {
  if (_accessibleElements != nil) {
    [_accessibleElements release];
  }
  _accessibleElements = [accessibleElements retain];
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

/* The container itself is not accessible but contains accessible elements. */
- (BOOL)isAccessibilityElement {
  return NO;
}

- (NSArray*)accessibleElements {
  return _accessibleElements;
}

- (NSInteger)accessibilityElementCount {
  return [[self accessibleElements] count];
}

- (id)accessibilityElementAtIndex:(NSInteger)index {
  return [[self accessibleElements] objectAtIndex:index];
}

- (NSInteger)indexOfAccessibilityElement:(id)element {
  return [[self accessibleElements] indexOfObject:element];
}

@end
