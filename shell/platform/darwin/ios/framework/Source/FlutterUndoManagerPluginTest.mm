// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterUndoManagerPlugin.h"

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextInputPlugin.h"

FLUTTER_ASSERT_ARC

@interface FlutterUndoManagerDelegateForTest : NSObject <FlutterUndoManagerDelegate>
@property(nonatomic, weak) UIResponder* viewController;
@property(nonatomic) FlutterTextInputPlugin* textInputPlugin;
@end

@implementation FlutterUndoManagerDelegateForTest

- (void)flutterUndoManagerPlugin:(FlutterUndoManagerPlugin*)undoManagerPlugin
         handleUndoWithDirection:(FlutterUndoRedoDirection)direction {
}
@end

@interface FlutterUndoManagerPluginTest : XCTestCase
@property(nonatomic) id undoManagerDelegate;
@property(nonatomic) FlutterUndoManagerPlugin* undoManagerPlugin;
@property(nonatomic) UIResponder* viewController;
@property(nonatomic) NSUndoManager* undoManager;
@end

@implementation FlutterUndoManagerPluginTest

- (void)setUp {
  [super setUp];
  self.undoManagerDelegate = OCMClassMock([FlutterUndoManagerDelegateForTest class]);

  self.undoManagerPlugin =
      [[FlutterUndoManagerPlugin alloc] initWithDelegate:self.undoManagerDelegate];

  self.undoManager = OCMClassMock([NSUndoManager class]);

  self.viewController = OCMClassMock([UIResponder class]);
  OCMStub([self.viewController undoManager]).andReturn(self.undoManager);
  OCMStub([self.undoManagerDelegate viewController]).andReturn(self.viewController);
}

- (void)tearDown {
  [self.undoManager removeAllActionsWithTarget:self.undoManagerPlugin];
  self.undoManagerDelegate = nil;
  self.viewController = nil;
  self.undoManager = nil;
  [super tearDown];
}

- (void)testSetUndoState {
  __block int registerUndoCount = 0;
  __block void (^undoHandler)(id target);
  OCMStub([self.undoManager registerUndoWithTarget:self.undoManagerPlugin handler:[OCMArg any]])
      .andDo(^(NSInvocation* invocation) {
        registerUndoCount++;
        __weak void (^handler)(id target);
        [invocation retainArguments];
        [invocation getArgument:&handler atIndex:3];
        undoHandler = handler;
      });
  __block int removeAllActionsCount = 0;
  OCMStub([self.undoManager removeAllActionsWithTarget:self.undoManagerPlugin])
      .andDo(^(NSInvocation* invocation) {
        removeAllActionsCount++;
      });
  __block int delegateUndoCount = 0;
  OCMStub([self.undoManagerDelegate flutterUndoManagerPlugin:[OCMArg any]
                                     handleUndoWithDirection:FlutterUndoRedoDirectionUndo])
      .andDo(^(NSInvocation* invocation) {
        delegateUndoCount++;
      });
  __block int delegateRedoCount = 0;
  OCMStub([self.undoManagerDelegate flutterUndoManagerPlugin:[OCMArg any]
                                     handleUndoWithDirection:FlutterUndoRedoDirectionRedo])
      .andDo(^(NSInvocation* invocation) {
        delegateRedoCount++;
      });
  __block int undoCount = 0;
  OCMStub([self.undoManager undo]).andDo(^(NSInvocation* invocation) {
    undoCount++;
    undoHandler(self.undoManagerPlugin);
  });

  // If canUndo and canRedo are false, only removeAllActionsWithTarget is called.
  FlutterMethodCall* setUndoStateCall =
      [FlutterMethodCall methodCallWithMethodName:@"UndoManager.setUndoState"
                                        arguments:@{@"canUndo" : @NO, @"canRedo" : @NO}];
  [self.undoManagerPlugin handleMethodCall:setUndoStateCall
                                    result:^(id _Nullable result){
                                    }];
  XCTAssertEqual(1, removeAllActionsCount);
  XCTAssertEqual(0, registerUndoCount);

  // If canUndo is true, an undo will be registered.
  setUndoStateCall =
      [FlutterMethodCall methodCallWithMethodName:@"UndoManager.setUndoState"
                                        arguments:@{@"canUndo" : @YES, @"canRedo" : @NO}];
  [self.undoManagerPlugin handleMethodCall:setUndoStateCall
                                    result:^(id _Nullable result){
                                    }];
  XCTAssertEqual(2, removeAllActionsCount);
  XCTAssertEqual(1, registerUndoCount);

  // Invoking the undo handler will invoke the handleUndo delegate method with "undo".
  undoHandler(self.undoManagerPlugin);
  XCTAssertEqual(1, delegateUndoCount);
  XCTAssertEqual(0, delegateRedoCount);
  XCTAssertEqual(2, registerUndoCount);

  // Invoking the redo handler will invoke the handleUndo delegate method with "redo".
  undoHandler(self.undoManagerPlugin);
  XCTAssertEqual(1, delegateUndoCount);
  XCTAssertEqual(1, delegateRedoCount);
  XCTAssertEqual(3, registerUndoCount);

  // If canRedo is true, an undo will be registered and undo will be called.
  setUndoStateCall =
      [FlutterMethodCall methodCallWithMethodName:@"UndoManager.setUndoState"
                                        arguments:@{@"canUndo" : @NO, @"canRedo" : @YES}];
  [self.undoManagerPlugin handleMethodCall:setUndoStateCall
                                    result:^(id _Nullable result){
                                    }];
  XCTAssertEqual(3, removeAllActionsCount);
  XCTAssertEqual(5, registerUndoCount);
  XCTAssertEqual(1, undoCount);

  // Invoking the redo handler will invoke the handleUndo delegate method with "redo".
  undoHandler(self.undoManagerPlugin);
  XCTAssertEqual(1, delegateUndoCount);
  XCTAssertEqual(2, delegateRedoCount);
}

- (void)testSetUndoStateDoesInteractWithInputDelegate {
  // Regression test for https://github.com/flutter/flutter/issues/133424
  FlutterTextInputPlugin* textInputPlugin = OCMClassMock([FlutterTextInputPlugin class]);
  FlutterTextInputView* textInputView = OCMClassMock([FlutterTextInputView class]);

  OCMStub([self.undoManagerDelegate textInputPlugin]).andReturn(textInputPlugin);
  OCMStub([textInputPlugin textInputView]).andReturn(textInputView);

  FlutterMethodCall* setUndoStateCall =
      [FlutterMethodCall methodCallWithMethodName:@"UndoManager.setUndoState"
                                        arguments:@{@"canUndo" : @NO, @"canRedo" : @NO}];
  [self.undoManagerPlugin handleMethodCall:setUndoStateCall
                                    result:^(id _Nullable result){
                                    }];

  OCMVerify(never(), [textInputView inputDelegate]);
}

@end
