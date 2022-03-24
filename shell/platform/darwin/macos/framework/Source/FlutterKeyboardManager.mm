// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyboardManager.h"

#include <cctype>
#include <map>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterChannelKeyResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEmbedderKeyResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyPrimaryResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/KeyCodeMap_Internal.h"

// #define DEBUG_PRINT_LAYOUT
#ifdef DEBUG_PRINT_LAYOUT
NSString* debugFormatLayoutData(NSString* debugLayoutData, uint16_t keyCode, LayoutClue clue1, LayoutClue clue2) {
    return [NSString
        stringWithFormat:@"    %@%@0x%d%04x, 0x%d%04x,", debugLayoutData,
                         keyCode % 4 == 0 ? [NSString stringWithFormat:@"\n/* 0x%02x */ ", keyCode]
                                          : @" ",
                         clue1.second, clue1.first, clue2.second, clue2.first];
}
#endif

namespace {

// Someohow this pointer type must be defined as a single type for the compiler
// to compile the function pointer type (due to _Nullable).
typedef NSResponder* _NSResponderPtr;
typedef _Nullable _NSResponderPtr (^NextResponderProvider)();

bool isEascii(const LayoutClue& clue) {
  return clue.first < 256 && !clue.second;
}

}

@interface FlutterKeyboardManager ()

/**
 * The text input plugin set by initialization.
 */
@property(nonatomic) id<FlutterKeyboardViewDelegate> viewDelegate;

/**
 * The primary responders added by addPrimaryResponder.
 */
@property(nonatomic) NSMutableArray<id<FlutterKeyPrimaryResponder>>* primaryResponders;

@property(nonatomic) NSMutableArray<NSEvent*>* pendingTextEvents;

@property(nonatomic) BOOL processingEvent;

@property(nonatomic) NSMutableDictionary<NSNumber*, NSNumber*>* layoutMap;

/**
 * Add a primary responder, which asynchronously decides whether to handle an
 * event.
 */
- (void)addPrimaryResponder:(nonnull id<FlutterKeyPrimaryResponder>)responder;

- (void)dispatchToSecondaryResponders:(NSEvent*)event;

- (void)buildLayout;

@end

@implementation FlutterKeyboardManager {
  NextResponderProvider _getNextResponder;
}

- (nonnull instancetype)initWithViewDelegate:(nonnull id<FlutterKeyboardViewDelegate>)viewDelegate {
  self = [super init];
  if (self != nil) {
    _viewDelegate = viewDelegate;

    _primaryResponders = [[NSMutableArray alloc] init];
    [self addPrimaryResponder:[[FlutterEmbedderKeyResponder alloc]
                                  initWithSendEvent:^(const FlutterKeyEvent& event,
                                                      FlutterKeyEventCallback callback,
                                                      void* userData) {
                                    [_viewDelegate sendKeyEvent:event
                                                       callback:callback
                                                       userData:userData];
                                  }]];
    [self
        addPrimaryResponder:[[FlutterChannelKeyResponder alloc]
                                initWithChannel:[FlutterBasicMessageChannel
                                                    messageChannelWithName:@"flutter/keyevent"
                                                           binaryMessenger:[_viewDelegate
                                                                               getBinaryMessenger]
                                                                     codec:[FlutterJSONMessageCodec
                                                                               sharedInstance]]]];
    _pendingTextEvents = [[NSMutableArray alloc] init];
    _layoutMap = [NSMutableDictionary<NSNumber*, NSNumber*> dictionary];
    [self buildLayout];
    for (id<FlutterKeyPrimaryResponder> responder in _primaryResponders) {
      responder.layoutMap = _layoutMap;
    }

    __weak __typeof__(self) weakSelf = self;
    [_viewDelegate subscribeToKeyboardLayoutChange:^() {
      [weakSelf buildLayout];
    }];
  }
  return self;
}

- (void)addPrimaryResponder:(nonnull id<FlutterKeyPrimaryResponder>)responder {
  [_primaryResponders addObject:responder];
}

- (void)handleEvent:(nonnull NSEvent*)event {
  // Be sure to add a handling method in propagateKeyEvent when allowing more
  // event types here.
  if (event.type != NSEventTypeKeyDown && event.type != NSEventTypeKeyUp &&
      event.type != NSEventTypeFlagsChanged) {
    return;
  }

  if (_viewDelegate.isComposing) {
    [self dispatchToSecondaryResponders:event];
    return;
  }

  // Having no primary responders require extra logic, but Flutter hard-codes
  // all primary responders, so this is a situation that Flutter will never
  // encounter.
  NSAssert([_primaryResponders count] >= 0, @"At least one primary responder must be added.");

  __weak __typeof__(self) weakSelf = self;
  __block int unreplied = [_primaryResponders count];
  __block BOOL anyHandled = false;
  FlutterAsyncKeyCallback replyCallback = ^(BOOL handled) {
    unreplied -= 1;
    NSAssert(unreplied >= 0, @"More primary responders replied than possible.");
    anyHandled = anyHandled || handled;
    if (unreplied == 0 && !anyHandled) {
      [weakSelf dispatchToSecondaryResponders:event];
    }
  };

  for (id<FlutterKeyPrimaryResponder> responder in _primaryResponders) {
    [responder handleEvent:event callback:replyCallback];
  }
}

#pragma mark - Private

- (void)dispatchToSecondaryResponders:(NSEvent*)event {
  if ([_viewDelegate onTextInputKeyEvent:event]) {
    return;
  }
  NSResponder* nextResponder = _viewDelegate.nextResponder;
  if (nextResponder == nil) {
    return;
  }
  switch (event.type) {
    case NSEventTypeKeyDown:
      if ([nextResponder respondsToSelector:@selector(keyDown:)]) {
        [nextResponder keyDown:event];
      }
      break;
    case NSEventTypeKeyUp:
      if ([nextResponder respondsToSelector:@selector(keyUp:)]) {
        [nextResponder keyUp:event];
      }
      break;
    case NSEventTypeFlagsChanged:
      if ([nextResponder respondsToSelector:@selector(flagsChanged:)]) {
        [nextResponder flagsChanged:event];
      }
      break;
    default:
      NSAssert(false, @"Unexpected key event type (got %lu).", event.type);
  }
}

- (void)buildLayout {
  [_layoutMap removeAllObjects];

  std::map<uint32_t, LayoutGoal> mandatoryGoalsByChar;
  std::map<uint32_t, LayoutGoal> usLayoutGoalsByKeyCode;
  for (const LayoutGoal& goal : layoutGoals) {
    if (goal.mandatory) {
      mandatoryGoalsByChar[goal.keyChar] = goal;
    } else {
      usLayoutGoalsByKeyCode[goal.keyCode] = goal;
    }
  }

  // Derive key mapping for each key code based on their layout clues.
  // Max key code is 127 for ADB keyboards.
  // https://developer.apple.com/documentation/coreservices/1390584-uckeytranslate?language=objc#parameters
  const uint16_t kMaxKeyCode = 127;
#ifdef DEBUG_PRINT_LAYOUT
  NSString* debugLayoutData = @"";
#endif
  for (uint16_t keyCode = 0; keyCode <= kMaxKeyCode; keyCode += 1) {
    std::vector<LayoutClue> thisKeyClues = {
        [_viewDelegate lookUpLayoutForKeyCode:keyCode shift:false],
        [_viewDelegate lookUpLayoutForKeyCode:keyCode shift:true]};
#ifdef DEBUG_PRINT_LAYOUT
    debugLayoutData = debugFormatLayoutData(debugLayoutData, keyCode, thisKeyClues[0], thisKeyClues[1]);
#endif
    // The logical key should be the first available clue from below:
    //
    //  - Mandatory goal, if matches any clue.
    //    This ensures that all alnum keys can be found somewhere.
    //  - US layout, if neither clue of the key is EASCII.
    //    This ensures that there are no non-latin logical keys.
    //  - Derived on the fly from keyCode & characters.
    for (const LayoutClue& clue : thisKeyClues) {
      uint32_t keyChar = clue.first;
      auto matchingGoal = mandatoryGoalsByChar.find(keyChar);
      if (matchingGoal != mandatoryGoalsByChar.end()) {
        // Found a key that produces a mandatory char. Use it.
        NSAssert(_layoutMap[@(keyCode)] == nil, @"Attempting to assign an assigned key code.");
        _layoutMap[@(keyCode)] = @(keyChar);
        mandatoryGoalsByChar.erase(matchingGoal);
        break;
      }
    }
    bool hasAnyEascii = isEascii(thisKeyClues[0]) || isEascii(thisKeyClues[1]);
    // See if any produced char meets the requirement as a logical key.
    if (_layoutMap[@(keyCode)] == nil && !hasAnyEascii) {
      _layoutMap[@(keyCode)] = @(usLayoutGoalsByKeyCode[keyCode].keyChar);
    }
  }
#ifdef DEBUG_PRINT_LAYOUT
  NSLog(@"%@", debugLayoutData);
#endif

  // Ensure all mandatory goals are assigned.
  for (auto mandatoryGoalIter : mandatoryGoalsByChar) {
    const LayoutGoal& goal = mandatoryGoalIter.second;
    _layoutMap[@(goal.keyCode)] = @(goal.keyChar);
  }
}

@end
