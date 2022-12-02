// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERTEXTINPUTDELEGATE_H_
#define SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERTEXTINPUTDELEGATE_H_

#import <Foundation/Foundation.h>

@class FlutterTextInputPlugin;
@protocol FlutterTextInputClient;

typedef NS_ENUM(NSInteger, FlutterTextInputAction) {
  FlutterTextInputActionUnspecified,
  FlutterTextInputActionDone,
  FlutterTextInputActionGo,
  FlutterTextInputActionSend,
  FlutterTextInputActionSearch,
  FlutterTextInputActionNext,
  FlutterTextInputActionContinue,
  FlutterTextInputActionJoin,
  FlutterTextInputActionRoute,
  FlutterTextInputActionEmergencyCall,
  FlutterTextInputActionNewline,
};

typedef NS_ENUM(NSInteger, FlutterFloatingCursorDragState) {
  FlutterFloatingCursorDragStateStart,
  FlutterFloatingCursorDragStateUpdate,
  FlutterFloatingCursorDragStateEnd,
};

@protocol FlutterTextInputDelegate <NSObject>
- (void)flutterTextInputView:(UIView<FlutterTextInputClient>*)textInputClient
         updateEditingClient:(int)client
                   withState:(NSDictionary*)state;
- (void)flutterTextInputView:(UIView<FlutterTextInputClient>*)textInputClient
         updateEditingClient:(int)client
                   withState:(NSDictionary*)state
                     withTag:(NSString*)tag;
- (void)flutterTextInputView:(UIView<FlutterTextInputClient>*)textInputClient
         updateEditingClient:(int)client
                   withDelta:(NSDictionary*)state;
- (void)flutterTextInputView:(UIView<FlutterTextInputClient>*)textInputClient
               performAction:(FlutterTextInputAction)action
                  withClient:(int)client;
- (void)flutterTextInputView:(UIView<FlutterTextInputClient>*)textInputView
        updateFloatingCursor:(FlutterFloatingCursorDragState)state
                  withClient:(int)client
                withPosition:(NSDictionary*)point;
- (void)flutterTextInputView:(UIView<FlutterTextInputClient>*)textInputClient
    showAutocorrectionPromptRectForStart:(NSUInteger)start
                                     end:(NSUInteger)end
                              withClient:(int)client;
- (void)flutterTextInputView:(UIView<FlutterTextInputClient>*)textInputClient
                 showToolbar:(int)client;
- (void)flutterTextInputViewScribbleInteractionBegan:(UIView<FlutterTextInputClient>*)textInputView;
- (void)flutterTextInputViewScribbleInteractionFinished:
    (UIView<FlutterTextInputClient>*)textInputView;
- (void)flutterTextInputView:(UIView<FlutterTextInputClient>*)textInputClient
    insertTextPlaceholderWithSize:(CGSize)size
                       withClient:(int)client;
- (void)flutterTextInputView:(UIView<FlutterTextInputClient>*)textInputClient
       removeTextPlaceholder:(int)client;
- (void)flutterTextInputViewDidResignFirstResponder:
    (UIView<FlutterTextInputClient>*)textInputClient;

@end

#endif  // SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERTEXTINPUTDELEGATE_H_
