// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

@protocol FlutterKeyboardViewDelegate

@required

@property(nonatomic, readonly, nullable) NSResponder* nextResponder;

- (BOOL)onTextInputKeyEvent:(nonnull NSEvent*)event;

@end
