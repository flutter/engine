// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <AppKit/AppKit.h>

@interface NSCursor (FlutterIgnoreCursorChange)

/// When set to YES prevents [NSCursor set] from changing current cursor.
@property(readwrite, class) BOOL flutterIgnoreCursorChange;

@end
