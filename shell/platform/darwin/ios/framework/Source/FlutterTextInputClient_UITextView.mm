// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextInputClient.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextInputPlugin.h"

FLUTTER_ASSERT_ARC

@interface FlutterUITextViewInputClient
    : UITextView <FlutterTextInputClient, FlutterTextAutofillClient, UIScribbleInteractionDelegate>

// Scribble Support
@property(nonatomic, weak) id<FlutterViewResponder> viewResponder;
@property(nonatomic, strong) NSArray<FlutterTextSelectionRect*>* selectionRects;

@property(nonatomic, weak) FlutterTextInputPlugin* textInputPlugin;
@property(nonatomic, readonly) CATransform3D editableTransform;
@property(nonatomic, assign) CGRect markedRect;
@property(nonatomic, assign) BOOL isVisibleToAutofill;
@property(nonatomic, assign) BOOL enableDeltaModel;
@property(nonatomic, assign) BOOL enableInteractiveSelection;
@property(nonatomic, assign) BOOL enableSoftwareKeyboard;
@property(nonatomic, assign) BOOL isFloatingCursorActive;

- (instancetype)initWithFrame:(CGRect)frame
                textContainer:(NSTextContainer*)textContainer NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)initWithCoder:(NSCoder*)aDecoder NS_UNAVAILABLE;
+ (instancetype)textViewUsingTextLayoutManager:(BOOL)usingTextLayoutManager NS_UNAVAILABLE;
- (instancetype)initWithOwner:(FlutterTextInputPlugin*)textInputPlugin NS_DESIGNATED_INITIALIZER;
@end

@implementation FlutterUITextViewInputClient {
  FlutterTokenizer* _tokenizer;
  CGRect _realFrame;
  FlutterScribbleInteractionStatus _scribbleInteractionStatus;
}
@synthesize accessibilityEnabled = _accessibilityEnabled;
@synthesize scribbleFocusStatus = _scribbleFocusStatus;
@synthesize autofillID = _autofillID;
@synthesize clientID = _clientID;
@synthesize backingTextInputAccessibilityObject = _backingTextInputAccessibilityObject;
@synthesize smartQuotesType = _smartQuotesType;
@synthesize smartDashesType = _smartDashesType;

#pragma mark - FlutterTextInputClient Conformance
#pragma mark Initializer

- (instancetype)initWithOwner:(FlutterTextInputPlugin*)textInputPlugin {
  self = [super initWithFrame:CGRectZero textContainer:nil];
  if (self) {
    _enableDeltaModel = NO;
    _enableInteractiveSelection = YES;
    _accessibilityEnabled = NO;
    _smartQuotesType = UITextSmartQuotesTypeYes;
    _smartDashesType = UITextSmartDashesTypeYes;
    _selectionRects = [[NSArray alloc] init];
    _realFrame = self.frame;
    _scribbleInteractionStatus = FlutterScribbleInteractionStatusNone;
    _editableTransform = CATransform3D();
    _isFloatingCursorActive = false;
    if (@available(iOS 14.0, *)) {
      UIScribbleInteraction* interaction = [[UIScribbleInteraction alloc] initWithDelegate:self];
      [self addInteraction:interaction];
    }
  }
  return self;
}
- (void)setTextInputState:(NSDictionary*)state {
}

- (void)setEditableSize:(CGSize)size transform:(NSArray*)matrix {
  CATransform3D* transform = &_editableTransform;

  transform->m11 = [matrix[0] doubleValue];
  transform->m12 = [matrix[1] doubleValue];
  transform->m13 = [matrix[2] doubleValue];
  transform->m14 = [matrix[3] doubleValue];

  transform->m21 = [matrix[4] doubleValue];
  transform->m22 = [matrix[5] doubleValue];
  transform->m23 = [matrix[6] doubleValue];
  transform->m24 = [matrix[7] doubleValue];

  transform->m31 = [matrix[8] doubleValue];
  transform->m32 = [matrix[9] doubleValue];
  transform->m33 = [matrix[10] doubleValue];
  transform->m34 = [matrix[11] doubleValue];

  transform->m41 = [matrix[12] doubleValue];
  transform->m42 = [matrix[13] doubleValue];
  transform->m43 = [matrix[14] doubleValue];
  transform->m44 = [matrix[15] doubleValue];
}

- (BOOL)isVisibleToAutofill {
  return self.frame.size.width > 0 && self.frame.size.height > 0;
}

// An input view is generally ignored by password autofill attempts, if it's
// not the first responder and is zero-sized. For input fields that are in the
// autofill context but do not belong to the current autofill group, setting
// their frames to CGRectZero prevents ios autofill from taking them into
// account.
- (void)setIsVisibleToAutofill:(BOOL)isVisibleToAutofill {
  if (isVisibleToAutofill != self.isVisibleToAutofill) {
    self.frame = isVisibleToAutofill ? _realFrame : CGRectZero;
  }
}

#pragma mark - Scribble Implementation

#pragma mark UIScribbleInteractionDelegate

- (void)scribbleInteractionWillBeginWriting:(UIScribbleInteraction*)interaction
    API_AVAILABLE(ios(14.0)) {
  _scribbleInteractionStatus = FlutterScribbleInteractionStatusStarted;
  [self.textInputPlugin.textInputDelegate flutterTextInputViewScribbleInteractionBegan:self];
}

- (void)scribbleInteractionDidFinishWriting:(UIScribbleInteraction*)interaction
    API_AVAILABLE(ios(14.0)) {
  _scribbleInteractionStatus = FlutterScribbleInteractionStatusEnding;
  [self.textInputPlugin.textInputDelegate flutterTextInputViewScribbleInteractionFinished:self];
}

- (BOOL)scribbleInteraction:(UIScribbleInteraction*)interaction
      shouldBeginAtLocation:(CGPoint)location API_AVAILABLE(ios(14.0)) {
  return YES;
}

- (BOOL)scribbleInteractionShouldDelayFocus:(UIScribbleInteraction*)interaction
    API_AVAILABLE(ios(14.0)) {
  return NO;
}

- (void)resetScribbleInteractionStatusIfEnding {
  if (_scribbleInteractionStatus == FlutterScribbleInteractionStatusEnding) {
    _scribbleInteractionStatus = FlutterScribbleInteractionStatusNone;
  }
}
// Prevent UIKit from showing selection handles or highlights. This is needed
// because Scribble interactions require the view to have it's actual frame on
// the screen.
- (UIColor*)insertionPointColor {
  return [UIColor clearColor];
}

- (UIColor*)selectionBarColor {
  return [UIColor clearColor];
}

- (UIColor*)selectionHighlightColor {
  return [UIColor clearColor];
}

#pragma mark - Accessibility

- (BOOL)accessibilityElementsHidden {
  return !_accessibilityEnabled;
}

- (void)accessibilityElementDidBecomeFocused {
  if ([self accessibilityElementIsFocused]) {
    // For most of the cases, this flutter text input view should never
    // receive the focus. If we do receive the focus, we make the best effort
    // to send the focus back to the real text field.
    FML_DCHECK(_backingTextInputAccessibilityObject);
    UIAccessibilityPostNotification(UIAccessibilityScreenChangedNotification,
                                    _backingTextInputAccessibilityObject);
  }
}

#pragma mark - UIResponder

- (UIInputViewController*)inputViewController {
  return _enableSoftwareKeyboard ? super.inputViewController : nil;
}

- (BOOL)canBecomeFirstResponder {
  // Only the currently focused input field can
  // become the first responder. This prevents iOS
  // from changing focus by itself (the framework
  // focus will be out of sync if that happens).
  return self.clientID != 0;
}

- (BOOL)resignFirstResponder {
  BOOL success = [super resignFirstResponder];
  if (success) {
    [self.textInputPlugin.textInputDelegate flutterTextInputViewDidResignFirstResponder:self];
  }
  return success;
}

- (BOOL)canPerformAction:(SEL)action withSender:(id)sender {
  // When scribble is available, the FlutterTextInputView will display the native toolbar unless
  // these text editing actions are disabled.
  if (FlutterTextInputPlugin.isScribbleAvailable && sender == NULL) {
    return NO;
  }
  if (action == @selector(paste:)) {
    // Forbid pasting images, memojis, or other non-string content.
    return [UIPasteboard generalPasteboard].string != nil;
  }
  return [super canPerformAction:action withSender:sender];
}

#pragma mark Pointer Events

// Forward touches to the viewResponder to allow tapping inside the UITextField as normal.
- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
  _scribbleFocusStatus = FlutterScribbleFocusStatusUnfocused;
  [self resetScribbleInteractionStatusIfEnding];
  [self.viewResponder touchesBegan:touches withEvent:event];
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
  [self.viewResponder touchesMoved:touches withEvent:event];
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
  [self.viewResponder touchesEnded:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
  [self.viewResponder touchesCancelled:touches withEvent:event];
}

- (void)touchesEstimatedPropertiesUpdated:(NSSet*)touches {
  [self.viewResponder touchesEstimatedPropertiesUpdated:touches];
}

#pragma mark Key Events

- (void)pressesBegan:(NSSet<UIPress*>*)presses
           withEvent:(UIPressesEvent*)event API_AVAILABLE(ios(9.0)) {
  [_textInputPlugin.viewController pressesBegan:presses withEvent:event];
}

- (void)pressesChanged:(NSSet<UIPress*>*)presses
             withEvent:(UIPressesEvent*)event API_AVAILABLE(ios(9.0)) {
  [_textInputPlugin.viewController pressesChanged:presses withEvent:event];
}

- (void)pressesEnded:(NSSet<UIPress*>*)presses
           withEvent:(UIPressesEvent*)event API_AVAILABLE(ios(9.0)) {
  [_textInputPlugin.viewController pressesEnded:presses withEvent:event];
}

- (void)pressesCancelled:(NSSet<UIPress*>*)presses
               withEvent:(UIPressesEvent*)event API_AVAILABLE(ios(9.0)) {
  [_textInputPlugin.viewController pressesCancelled:presses withEvent:event];
}

#pragma mark - UITextInput Conformance

- (id<UITextInputTokenizer>)tokenizer {
  if (_tokenizer == nil) {
    _tokenizer = [[FlutterTokenizer alloc] initWithTextInput:self];
  }
  return _tokenizer;
}

#pragma mark Floating Cursor

- (void)beginFloatingCursorAtPoint:(CGPoint)point {
  // For "beginFloatingCursorAtPoint" and "updateFloatingCursorAtPoint", "point" is roughly:
  //
  // CGPoint(
  //   width >= 0 ? point.x.clamp(boundingBox.left, boundingBox.right) : point.x,
  //   height >= 0 ? point.y.clamp(boundingBox.top, boundingBox.bottom) : point.y,
  // )
  //   where
  //     point = keyboardPanGestureRecognizer.translationInView(textInputView) +
  //     caretRectForPosition boundingBox = self.convertRect(bounds, fromView:textInputView)
  //     bounds = self._selectionClipRect ?? self.bounds
  //
  // It's tricky to provide accurate "bounds" and "caretRectForPosition" so it's preferred to
  // bypass the clamping and implement the same clamping logic in the framework where we have easy
  // access to the bounding box of the input field and the caret location.
  //
  // The current implementation returns kSpacePanBounds for "bounds" when
  // "_isFloatingCursorActive" is true. kSpacePanBounds centers "caretRectForPosition" so the
  // floating cursor has enough clearance in all directions to move around.
  //
  // It seems impossible to use a negative "width" or "height", as the "convertRect"
  // call always turns a CGRect's negative dimensions into non-negative values, e.g.,
  // (1, 2, -3, -4) would become (-2, -2, 3, 4).
  _isFloatingCursorActive = true;
  [self.textInputPlugin.textInputDelegate
      flutterTextInputView:self
      updateFloatingCursor:FlutterFloatingCursorDragStateStart
                withClient:self.clientID
              withPosition:@{@"X" : @(point.x), @"Y" : @(point.y)}];
}

- (void)updateFloatingCursorAtPoint:(CGPoint)point {
  _isFloatingCursorActive = true;
  [self.textInputPlugin.textInputDelegate
      flutterTextInputView:self
      updateFloatingCursor:FlutterFloatingCursorDragStateUpdate
                withClient:self.clientID
              withPosition:@{@"X" : @(point.x), @"Y" : @(point.y)}];
}

- (void)endFloatingCursor {
  _isFloatingCursorActive = false;
  [self.textInputPlugin.textInputDelegate flutterTextInputView:self
                                          updateFloatingCursor:FlutterFloatingCursorDragStateEnd
                                                    withClient:self.clientID
                                                  withPosition:@{@"X" : @(0), @"Y" : @(0)}];
}

@end
