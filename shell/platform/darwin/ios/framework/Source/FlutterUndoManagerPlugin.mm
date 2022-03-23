// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterUndoManagerPlugin.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include "flutter/fml/logging.h"

#pragma mark - UndoManager channel method names.
static NSString* const kSetUndoStateMethod = @"UndoManager.setUndoState";

#pragma mark - Undo State field names
static NSString* const kCanUndo = @"canUndo";
static NSString* const kCanRedo = @"canRedo";

@implementation FlutterUndoManagerPlugin {
  id<FlutterUndoManagerDelegate> _undoManagerDelegate;
}

- (instancetype)initWithDelegate:(id<FlutterUndoManagerDelegate>)undoManagerDelegate {
  self = [super init];

  if (self) {
    // `_undoManagerDelegate` is a weak reference because it should retain FlutterUndoManagerPlugin.
    _undoManagerDelegate = undoManagerDelegate;
  }

  return self;
}

- (void)dealloc {
  [self resetUndoManager];
  [super dealloc];
}

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
  NSString* method = call.method;
  id args = call.arguments;
  if ([method isEqualToString:kSetUndoStateMethod]) {
    [self setUndoState:args];
    result(nil);
  } else {
    result(FlutterMethodNotImplemented);
  }
}

- (NSUndoManager*)undoManager {
  return _viewController.undoManager;
}

- (void)resetUndoManager API_AVAILABLE(ios(9.0)) {
  [self.undoManager removeAllActionsWithTarget:self];
}

- (void)registerUndoWithDirection:(FlutterUndoRedoDirection)direction API_AVAILABLE(ios(9.0)) {
  self.undoManager.groupsByEvent = NO;
  [self.undoManager beginUndoGrouping];
  [self.undoManager registerUndoWithTarget:self
                                   handler:^(id target) {
                                     // Register undo with opposite direction.
                                     FlutterUndoRedoDirection newDirection =
                                         (direction == FlutterUndoRedoDirectionRedo)
                                             ? FlutterUndoRedoDirectionUndo
                                             : FlutterUndoRedoDirectionRedo;
                                     [target registerUndoWithDirection:newDirection];
                                     // Invoke method on delegate.
                                     [_undoManagerDelegate flutterUndoManagerPlugin:self
                                                                         handleUndo:direction];
                                   }];
  [self.undoManager endUndoGrouping];
  self.undoManager.groupsByEvent = YES;
}

- (void)registerRedo API_AVAILABLE(ios(9.0)) {
  self.undoManager.groupsByEvent = NO;
  [self.undoManager beginUndoGrouping];
  [self.undoManager
      registerUndoWithTarget:self
                     handler:^(id target) {
                       // Register undo with opposite direction.
                       [target registerUndoWithDirection:FlutterUndoRedoDirectionRedo];
                     }];
  [self.undoManager endUndoGrouping];
  self.undoManager.groupsByEvent = YES;
  [self.undoManager undo];
}

- (void)setUndoState:(NSDictionary*)dictionary API_AVAILABLE(ios(9.0)) {
  BOOL canUndo = [dictionary[kCanUndo] boolValue];
  BOOL canRedo = [dictionary[kCanRedo] boolValue];

  [self resetUndoManager];

  if (canUndo) {
    [self registerUndoWithDirection:FlutterUndoRedoDirectionUndo];
  }
  if (canRedo) {
    [self registerRedo];
  }
}

@end
