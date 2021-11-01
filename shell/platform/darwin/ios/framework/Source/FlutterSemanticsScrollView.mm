// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterSemanticsScrollView.h"

#import "flutter/shell/platform/darwin/ios/framework/Source/SemanticsObject.h"

@implementation FlutterSemanticsScrollView {
  fml::WeakPtr<SemanticsObject> _semanticsObject;
}

- (instancetype)initWithSemanticsObject:(fml::WeakPtr<SemanticsObject>)semanticsObject {
  self = [super initWithFrame:CGRectZero];
  if (self) {
    _semanticsObject = semanticsObject;
  }
  return self;
}

- (UIView*)hitTest:(CGPoint)point withEvent:(UIEvent*)event {
  return nil;
}

// The following methods are explicitly forwarded to the wrapped SemanticsObject because the
// forwarding logic above doesn't apply to them since they are also implemented in the
// UIScrollView class, the base class.

- (BOOL)isAccessibilityElement {
  if (![_semanticsObject.get() isAccessibilityBridgeAlive]) {
    return NO;
  }

  if ([_semanticsObject.get() isAccessibilityElement]) {
    return YES;
  }
  if (self.contentSize.width > self.frame.size.width ||
      self.contentSize.height > self.frame.size.height) {
    // In SwitchControl or VoiceControl, the isAccessibilityElement must return YES
    // in order to use scroll actions.
    return ![_semanticsObject.get() bridge]->isVoiceOverRunning();
  } else {
    return NO;
  }
}

- (NSString*)accessibilityLabel {
  return [_semanticsObject.get() accessibilityLabel];
}

- (NSAttributedString*)accessibilityAttributedLabel {
  return [_semanticsObject.get() accessibilityAttributedLabel];
}

- (NSString*)accessibilityValue {
  return [_semanticsObject.get() accessibilityValue];
}

- (NSAttributedString*)accessibilityAttributedValue {
  return [_semanticsObject.get() accessibilityAttributedValue];
}

- (NSString*)accessibilityHint {
  return [_semanticsObject.get() accessibilityHint];
}

- (NSAttributedString*)accessibilityAttributedHint {
  return [_semanticsObject.get() accessibilityAttributedHint];
}

- (BOOL)accessibilityActivate {
  return [_semanticsObject.get() accessibilityActivate];
}

- (void)accessibilityIncrement {
  [_semanticsObject.get() accessibilityIncrement];
}

- (void)accessibilityDecrement {
  [_semanticsObject.get() accessibilityDecrement];
}

- (BOOL)accessibilityScroll:(UIAccessibilityScrollDirection)direction {
  return [_semanticsObject.get() accessibilityScroll:direction];
}

- (BOOL)accessibilityPerformEscape {
  return [_semanticsObject.get() accessibilityPerformEscape];
}

- (void)accessibilityElementDidBecomeFocused {
  [_semanticsObject.get() accessibilityElementDidBecomeFocused];
}

- (void)accessibilityElementDidLoseFocus {
  [_semanticsObject.get() accessibilityElementDidLoseFocus];
}

- (id)accessibilityContainer {
  return [_semanticsObject.get() accessibilityContainer];
}

- (NSInteger)accessibilityElementCount {
  return [[_semanticsObject.get() children] count];
}

@end
