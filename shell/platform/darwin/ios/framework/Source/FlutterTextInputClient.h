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

/** An object that represents a framework text editing widget and interacts with the iOS text input
 * system on behalf of that widget.
 * A FlutterTextInputClient can receive editing state updates from the setTextInputState: method,
 * and it should typically relay editing state changes made by the iOS text input system to the
 * framework, via the textInputPlugin.textInputDelegate method. */
@protocol FlutterTextInputClient <NSObject, UITextInput>
/** The framework issued id of this client. */
@property(nonatomic, assign) int clientID;
@property(nonatomic, assign) BOOL accessibilityEnabled;
@property(nonatomic, assign) FlutterScribbleFocusStatus scribbleFocusStatus;
@property(nonatomic, weak) UIAccessibilityElement* backingTextInputAccessibilityObject;

- (instancetype)initWithOwner:(FlutterTextInputPlugin*)textInputPlugin;
/** Updates the rect that describes the bounding box of the framework blinking cursor, in the
 * framework widget's coordinates.
 * See the setEditableSize:transform: method. */
- (void)setMarkedRect:(CGRect)rect;
- (void)setViewResponder:(id<FlutterViewResponder>)viewResponder;
/** Updates the visible glyph boxes in the framework, in the framework widget's coordinates.
 * See the setEditableSize:transform: method. */
- (void)setSelectionRects:(NSArray*)rects;
/** Called by the framework to update the editing state  (text, selection, composing region). */
- (void)setTextInputState:(NSDictionary*)state;
/** Updates the transform and the size of the framework text editing widget's text editing region.
 * The information describes the paint transform and paint bounds of the framework widget. */
- (void)setEditableSize:(CGSize)size transform:(NSArray*)matrix;
- (void)setEnableDeltaModel:(BOOL)enableDeltaModel;
- (void)setEnableSoftwareKeyboard:(BOOL)enabled;
- (void)setEnableInteractiveSelection:(BOOL)enabled;
@end

@protocol FlutterTextAutofillClient <NSObject>

/** A framework issued id used to uniquely identify an autofill client. The ID is guaranteed to be
 * unique at any given time. */
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
