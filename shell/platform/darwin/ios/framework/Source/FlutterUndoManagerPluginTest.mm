// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterUndoManagerPlugin.h"

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"
#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterViewController.h"

FLUTTER_ASSERT_ARC

@interface FlutterEngine ()
- (nonnull FlutterUndoManagerPlugin*)undoManagerPlugin;
@end

@interface FlutterUndoManagerPluginTest : XCTestCase
@end

@implementation FlutterUndoManagerPluginTest {
  id engine;
  FlutterUndoManagerPlugin* undoManagerPlugin;
  FlutterViewController* viewController;
  NSUndoManager* undoManager;
}

- (void)setUp {
  [super setUp];
  engine = OCMClassMock([FlutterEngine class]);

  undoManagerPlugin = [[FlutterUndoManagerPlugin alloc] initWithDelegate:engine];

  viewController = [FlutterViewController new];
  undoManagerPlugin.viewController = viewController;

  undoManager = OCMClassMock([NSUndoManager class]);
  undoManagerPlugin.undoManager = undoManager;
}

- (void)tearDown {
  [undoManager removeAllActionsWithTarget:undoManagerPlugin];
  undoManagerPlugin = nil;
  engine = nil;
  viewController = nil;
  undoManager = nil;
  [super tearDown];
}

- (void)testSetUndoState {
  __block int registerUndoCount = 0;
  __block void (^undoHandler)(id target);
  OCMStub([undoManager registerUndoWithTarget:undoManagerPlugin handler:[OCMArg any]])
      .andDo(^(NSInvocation* invocation) {
        registerUndoCount++;
        __weak void (^handler)(id target);
        [invocation retainArguments];
        [invocation getArgument:&handler atIndex:3];
        undoHandler = handler;
      });
  __block int removeAllActionsCount = 0;
  OCMStub([undoManager removeAllActionsWithTarget:undoManagerPlugin])
      .andDo(^(NSInvocation* invocation) {
        removeAllActionsCount++;
      });
  __block int delegateUndoCount = 0;
  OCMStub([engine flutterUndoManagerPlugin:[OCMArg any] handleUndo:FlutterUndoRedoDirectionUndo])
      .andDo(^(NSInvocation* invocation) {
        delegateUndoCount++;
      });
  __block int delegateRedoCount = 0;
  OCMStub([engine flutterUndoManagerPlugin:[OCMArg any] handleUndo:FlutterUndoRedoDirectionRedo])
      .andDo(^(NSInvocation* invocation) {
        delegateRedoCount++;
      });
  __block int undoCount = 0;
  OCMStub([undoManager undo]).andDo(^(NSInvocation* invocation) {
    undoCount++;
    undoHandler(undoManagerPlugin);
  });

  // If canUndo and canRedo are false, only removeAllActionsWithTarget is called.
  FlutterMethodCall* setUndoStateCall =
      [FlutterMethodCall methodCallWithMethodName:@"UndoManager.setUndoState"
                                        arguments:@{@"canUndo" : @NO, @"canRedo" : @NO}];
  [undoManagerPlugin handleMethodCall:setUndoStateCall
                               result:^(id _Nullable result){
                               }];
  XCTAssertEqual(1, removeAllActionsCount);
  XCTAssertEqual(0, registerUndoCount);

  // If canUndo is true, an undo will be registered.
  setUndoStateCall =
      [FlutterMethodCall methodCallWithMethodName:@"UndoManager.setUndoState"
                                        arguments:@{@"canUndo" : @YES, @"canRedo" : @NO}];
  [undoManagerPlugin handleMethodCall:setUndoStateCall
                               result:^(id _Nullable result){
                               }];
  XCTAssertEqual(2, removeAllActionsCount);
  XCTAssertEqual(1, registerUndoCount);

  // Invoking the undo handler will invoke the handleUndo delegate method with "undo".
  undoHandler(undoManagerPlugin);
  XCTAssertEqual(1, delegateUndoCount);
  XCTAssertEqual(0, delegateRedoCount);
  XCTAssertEqual(2, registerUndoCount);

  // Invoking the redo handler will invoke the handleUndo delegate method with "redo".
  undoHandler(undoManagerPlugin);
  XCTAssertEqual(1, delegateUndoCount);
  XCTAssertEqual(1, delegateRedoCount);
  XCTAssertEqual(3, registerUndoCount);

  // If canRedo is true, an undo will be registered and undo will be called.
  setUndoStateCall =
      [FlutterMethodCall methodCallWithMethodName:@"UndoManager.setUndoState"
                                        arguments:@{@"canUndo" : @NO, @"canRedo" : @YES}];
  [undoManagerPlugin handleMethodCall:setUndoStateCall
                               result:^(id _Nullable result){
                               }];
  XCTAssertEqual(3, removeAllActionsCount);
  XCTAssertEqual(5, registerUndoCount);
  XCTAssertEqual(1, undoCount);

  // Invoking the redo handler will invoke the handleUndo delegate method with "redo".
  undoHandler(undoManagerPlugin);
  XCTAssertEqual(1, delegateUndoCount);
  XCTAssertEqual(2, delegateRedoCount);
}

@end
