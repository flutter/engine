// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERTEXTINPUTPLUGIN_H_
#define SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERTEXTINPUTPLUGIN_H_

#import <UIKit/UIKit.h>

#import "flutter/shell/platform/common/text_editing_delta.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterChannels.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterIndirectScribbleDelegate.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterKeySecondaryResponder.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextInputDelegate.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterViewResponder.h"

@interface FlutterTextInputPlugin
    : NSObject <FlutterKeySecondaryResponder, UIIndirectScribbleInteractionDelegate>

@property(nonatomic, weak) UIViewController* viewController;
@property(nonatomic, readonly) UIView* hostView;
@property(nonatomic, weak) id<FlutterIndirectScribbleDelegate> indirectScribbleDelegate;
@property(nonatomic, strong)
    NSMutableDictionary<UIScribbleElementIdentifier, NSValue*>* scribbleElements;
@property(nonatomic, readonly, weak) id<FlutterTextInputDelegate> textInputDelegate;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithDelegate:(id<FlutterTextInputDelegate>)textInputDelegate
    NS_DESIGNATED_INITIALIZER;

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result;

/**
 * The `UITextInput` implementation used to control text entry.
 *
 * This is used by `AccessibilityBridge` to forward interactions with iOS'
 * accessibility system.
 */
- (UIView<UITextInput>*)textInputView;

+ (BOOL)isScribbleAvailable;
/**
 * These are used by the UIIndirectScribbleInteractionDelegate methods to handle focusing on the
 * correct element.
 */
- (void)setupIndirectScribbleInteraction:(id<FlutterViewResponder>)viewResponder;
- (void)resetViewResponder;

@end

/** A tokenizer used by `FlutterTextInputView` to customize string parsing. */
@interface FlutterTokenizer : UITextInputStringTokenizer
@end

@interface FlutterTextSelectionRect : UITextSelectionRect

@property(nonatomic, assign) CGRect rect;
@property(nonatomic) NSUInteger position;
@property(nonatomic, assign) NSWritingDirection writingDirection;
@property(nonatomic) BOOL containsStart;
@property(nonatomic) BOOL containsEnd;
@property(nonatomic) BOOL isVertical;

+ (instancetype)selectionRectWithRectAndInfo:(CGRect)rect
                                    position:(NSUInteger)position
                            writingDirection:(NSWritingDirection)writingDirection
                               containsStart:(BOOL)containsStart
                                 containsEnd:(BOOL)containsEnd
                                  isVertical:(BOOL)isVertical;

+ (instancetype)selectionRectWithRect:(CGRect)rect position:(NSUInteger)position;

- (instancetype)initWithRectAndInfo:(CGRect)rect
                           position:(NSUInteger)position
                   writingDirection:(NSWritingDirection)writingDirection
                      containsStart:(BOOL)containsStart
                        containsEnd:(BOOL)containsEnd
                         isVertical:(BOOL)isVertical;

- (instancetype)init NS_UNAVAILABLE;
@end

API_AVAILABLE(ios(13.0)) @interface FlutterTextPlaceholder : UITextPlaceholder
@end

#endif  // SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERTEXTINPUTPLUGIN_H_
