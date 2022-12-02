// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERTEXTINPUTCLIENT_H_
#define SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERTEXTINPUTCLIENT_H_

#import <UIKit/UIKit.h>
#import "FlutterMacros.h"

#define RegularInputClient FlutterTextInputView
#define SecureInputClient FlutterSecureTextInputView

@class FlutterTextInputPlugin;
@class FlutterTextSelectionRect;

@protocol FlutterViewResponder;

typedef NS_ENUM(NSInteger, FlutterScribbleFocusStatus) {
  FlutterScribbleFocusStatusUnfocused,
  FlutterScribbleFocusStatusFocusing,
  FlutterScribbleFocusStatusFocused,
};

typedef NS_ENUM(NSInteger, FlutterScribbleInteractionStatus) {
  FlutterScribbleInteractionStatusNone,
  FlutterScribbleInteractionStatusStarted,
  FlutterScribbleInteractionStatusEnding,
};

/** An indexed position in the buffer of a Flutter text editing widget. */
@interface FlutterTextPosition : UITextPosition

@property(nonatomic, readonly) NSUInteger index;

+ (instancetype)positionWithIndex:(NSUInteger)index;
- (instancetype)initWithIndex:(NSUInteger)index;

@end

/** A range of text in the buffer of a Flutter text editing widget. */
@interface FlutterTextRange : UITextRange <NSCopying>

@property(nonatomic, readonly) NSRange range;

+ (instancetype)rangeWithNSRange:(NSRange)range;

@end

@protocol FlutterTextInputClient <NSObject, UITextInput>

@property(nonatomic, assign) int clientID;
@property(nonatomic, assign) BOOL accessibilityEnabled;
@property(nonatomic, assign) FlutterScribbleFocusStatus scribbleFocusStatus;
@property(nonatomic, weak) UIAccessibilityElement* backingTextInputAccessibilityObject;

- (instancetype)initWithOwner:(FlutterTextInputPlugin*)textInputPlugin;

- (void)setMarkedRect:(CGRect)rect;
- (void)setViewResponder:(id<FlutterViewResponder>)viewResponder;
- (void)setSelectionRects:(NSArray*)rects;
- (void)setTextInputState:(NSDictionary*)state;
- (void)setEditableSize:(CGSize)size transform:(NSArray*)matrix;
- (void)setEnableDeltaModel:(BOOL)enableDeltaModel;
- (void)setEnableSoftwareKeyboard:(BOOL)enabled;
- (void)setEnableInteractiveSelection:(BOOL)enabled;
@end

@protocol FlutterTextAutofillClient <NSObject>

@property(nonatomic, copy) NSString* autofillID;

- (void)setIsVisibleToAutofill:(BOOL)visibility;
@end

#if FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DEBUG
FLUTTER_DARWIN_EXPORT
#endif
@interface FlutterTextInputView
    : UIView <FlutterTextInputClient, FlutterTextAutofillClient, UIScribbleInteractionDelegate>

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)initWithCoder:(NSCoder*)aDecoder NS_UNAVAILABLE;
- (instancetype)initWithFrame:(CGRect)frame NS_UNAVAILABLE;
- (instancetype)initWithOwner:(FlutterTextInputPlugin*)textInputPlugin NS_DESIGNATED_INITIALIZER;
@end

// A FlutterTextInputView that masquerades as a UITextField, and forwards
// selectors it can't respond to to a shared UITextField instance.
//
// Relevant API docs claim that password autofill supports any custom view
// that adopts the UITextInput protocol, automatic strong password seems to
// currently only support UITextFields, and password saving only supports
// UITextFields and UITextViews, as of iOS 13.5.
@interface FlutterSecureTextInputView : FlutterTextInputView
@end

#endif  // SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERTEXTINPUTCLIENT_H_
