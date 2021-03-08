// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyHandler.h"
#include "flutter/shell/platform/embedder/embedder.h"

namespace {
typedef void* _VoidPtr;
}

typedef void (^FlutterSendEmbedderKeyEvent)(const FlutterKeyEvent& /* event */,
                                            _Nullable FlutterKeyEventCallback /* callback */,
                                            _Nullable _VoidPtr /* user_data */);

@interface FlutterKeyEmbedderHandler : NSObject <FlutterKeyHandler>

- (nonnull instancetype)initWithSendEvent:(_Nonnull FlutterSendEmbedderKeyEvent)sendEvent;

@end
