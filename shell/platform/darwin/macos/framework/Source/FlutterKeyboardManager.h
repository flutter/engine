// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterTextInputPlugin.h"

namespace {
// Someohow this pointer type must be defined as a single type for the compiler
// to compile the function pointer type (due to _Nullable).
typedef NSResponder* _NSResponderPtr;
}

typedef _Nullable _NSResponderPtr (^NextResponderProvider)();

/**
 * A hub that manages how key events are dispatched to various Flutter key
 * responders, and whether the event is propagated to the next NSResponder.
 *
 * This class manages one or more primary responders, as well as zero or more
 * secondary responders.
 *
 * An event that is received by |handleEvent| is first dispatched to *all*
 * primary responders. Each primary responder responds *asynchronously* with a
 * boolean, indicating whether it handles the event.
 *
 * An event that is not handled by any primary responders is then passed to to
 * the first secondary responder (in the chronological order of addition),
 * which responds *synchronously* with a boolean, indicating whether it handles
 * the event. If not, the event is passed to the next secondary responder, and
 * so on.
 *
 * If no responders handle the event, the event is then handed over to the
 * owner's |nextResponder| if not nil, dispatching to method |keyDown|,
 * |keyUp|, or |flagsChanged| depending on the event's type. If the
 * |nextResponder| is nil, then the event will be propagated no further.
 *
 * Preventing primary responders from receiving events is not supported,
 * because in reality this class will only support 2 hardcoded ones (channel
 * and embedder), where the only purpose of supporting two is to support the
 * legacy API (channel) during the deprecation window, after which the channel
 * responder should be removed.
 */
@interface FlutterKeyboardManager : NSObject

// TODO
/**
 * Create a manager by specifying a weak pointer to the owner view controller.
 *
 * The |viewController.nextResponder| can be nil, but if it isn't, it will be where the
 * key events are propagated to if no responders handle the event.
 */
- (nonnull instancetype)initWithEngine:(nonnull FlutterEngine*)engine
                       textInputPlugin:(nonnull FlutterTextInputPlugin*)textInputPlugin
                      getNextResponder:(nonnull NextResponderProvider)getNextResponder;

/**
 * Dispatch a key event to all responders, and possibly the next |NSResponder|
 * afterwards.
 */
- (void)handleEvent:(nonnull NSEvent*)event;

@end
