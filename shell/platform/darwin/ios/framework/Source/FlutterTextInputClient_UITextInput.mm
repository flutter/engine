// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextInputClient.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextInputPlugin.h"

#include "flutter/fml/platform/darwin/string_range_sanitization.h"
#include "unicode/uchar.h"

FLUTTER_ASSERT_ARC

static const char kTextAffinityDownstream[] = "TextAffinity.downstream";
static const char kTextAffinityUpstream[] = "TextAffinity.upstream";
// The `bounds` value a FlutterTextInputView returns when the floating cursor
// is activated in that view.
//
// DO NOT use extremely large values (such as CGFloat_MAX) in this rect, for that
// will significantly reduce the precision of the floating cursor's coordinates.
//
// It is recommended for this CGRect to be roughly centered at caretRectForPosition
// (which currently always return CGRectZero), so the initial floating cursor will
// be placed at (0, 0).
// See the comments in beginFloatingCursorAtPoint and caretRectForPosition.
const CGRect kSpacePanBounds = {{-2500, -2500}, {5000, 5000}};

// The "canonical" invalid CGRect, similar to CGRectNull, used to
// indicate a CGRect involved in firstRectForRange calculation is
// invalid. The specific value is chosen so that if firstRectForRange
// returns kInvalidFirstRect, iOS will not show the IME candidates view.
const CGRect kInvalidFirstRect = {{-1, -1}, {9999, 9999}};

#pragma mark - Static Functions

// Determine if the character at `range` of `text` is an emoji.
static BOOL IsEmoji(NSString* text, NSRange charRange) {
  UChar32 codePoint;
  BOOL gotCodePoint = [text getBytes:&codePoint
                           maxLength:sizeof(codePoint)
                          usedLength:NULL
                            encoding:NSUTF32StringEncoding
                             options:kNilOptions
                               range:charRange
                      remainingRange:NULL];
  return gotCodePoint && u_hasBinaryProperty(codePoint, UCHAR_EMOJI);
}

static BOOL IsApproximatelyEqual(float x, float y, float delta) {
  return fabsf(x - y) <= delta;
}
// Checks whether point should be considered closer to selectionRect compared to
// otherSelectionRect.
//
// If checkRightBoundary is set, the right-center point on selectionRect and
// otherSelectionRect will be used instead of the left-center point.
//
// This uses special (empirically determined using a 1st gen iPad pro, 9.7" model running
// iOS 14.7.1) logic for determining the closer rect, rather than a simple distance calculation.
// First, the closer vertical distance is determined. Within the closest y distance, if the point is
// above the bottom of the closest rect, the x distance will be minimized; however, if the point is
// below the bottom of the rect, the x value will be maximized.
static BOOL IsSelectionRectCloserToPoint(CGPoint point,
                                         CGRect selectionRect,
                                         CGRect otherSelectionRect,
                                         BOOL checkRightBoundary) {
  CGPoint pointForSelectionRect =
      CGPointMake(selectionRect.origin.x + (checkRightBoundary ? selectionRect.size.width : 0),
                  selectionRect.origin.y + selectionRect.size.height * 0.5);
  float yDist = fabs(pointForSelectionRect.y - point.y);
  float xDist = fabs(pointForSelectionRect.x - point.x);

  CGPoint pointForOtherSelectionRect =
      CGPointMake(otherSelectionRect.origin.x + (checkRightBoundary ? selectionRect.size.width : 0),
                  otherSelectionRect.origin.y + otherSelectionRect.size.height * 0.5);
  float yDistOther = fabs(pointForOtherSelectionRect.y - point.y);
  float xDistOther = fabs(pointForOtherSelectionRect.x - point.x);

  // This serves a similar purpose to IsApproximatelyEqual, allowing a little buffer before
  // declaring something closer vertically to account for the small variations in size and position
  // of SelectionRects, especially when dealing with emoji.
  BOOL isCloserVertically = yDist < yDistOther - 1;
  BOOL isEqualVertically = IsApproximatelyEqual(yDist, yDistOther, 1);
  BOOL isAboveBottomOfLine = point.y <= selectionRect.origin.y + selectionRect.size.height;
  BOOL isCloserHorizontally = xDist <= xDistOther;
  BOOL isBelowBottomOfLine = point.y > selectionRect.origin.y + selectionRect.size.height;
  BOOL isFartherToRight =
      selectionRect.origin.x + (checkRightBoundary ? selectionRect.size.width : 0) >
      otherSelectionRect.origin.x;
  return (isCloserVertically ||
          (isEqualVertically && ((isAboveBottomOfLine && isCloserHorizontally) ||
                                 (isBelowBottomOfLine && isFartherToRight))));
}

@interface FlutterTextInputView ()
// UITextInput
@property(nonatomic, readonly) NSMutableString* markedText;
@property(nonatomic, strong) FlutterTextRange* markedTextRange;

// Other Configurations
@property(nonatomic, readonly) NSMutableString* text;
@property(nonatomic, getter=isEnableDeltaModel) BOOL enableDeltaModel;

// Scribble Support
@property(nonatomic, weak) id<FlutterViewResponder> viewResponder;
@property(nonatomic, strong) NSArray<FlutterTextSelectionRect*>* selectionRects;

@property(nonatomic, weak) FlutterTextInputPlugin* textInputPlugin;
@property(nonatomic, readonly) CATransform3D editableTransform;
@property(nonatomic, assign) CGRect markedRect;
@property(nonatomic) BOOL isVisibleToAutofill;
// The composed character that is temporarily removed by the keyboard API.
// This is cleared at the start of each keyboard interaction. (Enter a character, delete a character
// etc)
@property(nonatomic, copy) NSString* temporarilyDeletedComposedCharacter;

@end

@implementation FlutterTextInputView {
  int _clientID;
  const char* _selectionAffinity;
  FlutterTextRange* _selectedTextRange;
  UIInputViewController* _inputViewController;
  CGRect _cachedFirstRect;
  FlutterScribbleInteractionStatus _scribbleInteractionStatus;
  BOOL _hasPlaceholder;
  // Whether to show the system keyboard when this view
  // becomes the first responder. Typically set to false
  // when the app shows its own in-flutter keyboard.
  bool _isSystemKeyboardEnabled;
  bool _isFloatingCursorActive;
  bool _enableInteractiveSelection;
  UITextInteraction* _textInteraction API_AVAILABLE(ios(13.0));
}

@synthesize tokenizer = _tokenizer;
@synthesize markedTextStyle = _markedTextStyle;
@synthesize inputDelegate = _inputDelegate;
@synthesize autocorrectionType = _autocorrectionType;
@synthesize autocapitalizationType = _autocapitalizationType;
@synthesize spellCheckingType = _spellCheckingType;
@synthesize keyboardAppearance = _keyboardAppearance;
@synthesize keyboardType = _keyboardType;
@synthesize returnKeyType = _returnKeyType;
@synthesize enablesReturnKeyAutomatically = _enablesReturnKeyAutomatically;
@synthesize secureTextEntry = _secureTextEntry;
@synthesize smartQuotesType = _smartQuotesType;
@synthesize smartDashesType = _smartDashesType;
@synthesize textContentType = _textContentType;
@synthesize accessibilityEnabled = _accessibilityEnabled;
@synthesize scribbleFocusStatus = _scribbleFocusStatus;
@synthesize autofillID = _autofillID;
@synthesize backingTextInputAccessibilityObject = _backingTextInputAccessibilityObject;

- (instancetype)initWithOwner:(FlutterTextInputPlugin*)textInputPlugin {
  self = [super initWithFrame:CGRectZero];
  if (self) {
    _textInputPlugin = textInputPlugin;
    _clientID = 0;
    _selectionAffinity = kTextAffinityUpstream;

    // UITextInput
    _text = [[NSMutableString alloc] init];
    _markedText = [[NSMutableString alloc] init];
    _selectedTextRange = [FlutterTextRange rangeWithNSRange:NSMakeRange(0, 0)];
    _markedRect = kInvalidFirstRect;
    _cachedFirstRect = kInvalidFirstRect;
    _scribbleInteractionStatus = FlutterScribbleInteractionStatusNone;
    // Initialize with the zero matrix which is not
    // an affine transform.
    _editableTransform = CATransform3D();
    _isFloatingCursorActive = false;

    // UITextInputTraits
    _autocapitalizationType = UITextAutocapitalizationTypeSentences;
    _autocorrectionType = UITextAutocorrectionTypeDefault;
    _spellCheckingType = UITextSpellCheckingTypeDefault;
    _enablesReturnKeyAutomatically = NO;
    _keyboardAppearance = UIKeyboardAppearanceDefault;
    _keyboardType = UIKeyboardTypeDefault;
    _returnKeyType = UIReturnKeyDone;
    _secureTextEntry = NO;
    _enableDeltaModel = NO;
    _enableInteractiveSelection = YES;
    _accessibilityEnabled = NO;
    _smartQuotesType = UITextSmartQuotesTypeYes;
    _smartDashesType = UITextSmartDashesTypeYes;
    _selectionRects = [[NSArray alloc] init];

    if (@available(iOS 14.0, *)) {
      UIScribbleInteraction* interaction = [[UIScribbleInteraction alloc] initWithDelegate:self];
      [self addInteraction:interaction];
    }
  }

  return self;
}

- (UITextContentType)textContentType {
  return _textContentType;
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

- (void)setEnableSoftwareKeyboard:(BOOL)enableSoftwareKeyboard {
  _isSystemKeyboardEnabled = enableSoftwareKeyboard;
}
- (UIInputViewController*)inputViewController {
  if (_isSystemKeyboardEnabled) {
    return nil;
  }

  if (!_inputViewController) {
    _inputViewController = [[UIInputViewController alloc] init];
  }
  return _inputViewController;
}

- (id<FlutterTextInputDelegate>)textInputDelegate {
  return _textInputPlugin.textInputDelegate;
}

- (int)clientID {
  return _clientID;
}
- (void)setClientID:(int)client {
  _clientID = client;
  _hasPlaceholder = NO;
}

- (UITextInteraction*)textInteraction API_AVAILABLE(ios(13.0)) {
  if (!_textInteraction) {
    _textInteraction = [UITextInteraction textInteractionForMode:UITextInteractionModeEditable];
    _textInteraction.textInput = self;
  }
  return _textInteraction;
}

- (void)setTextInputState:(NSDictionary*)state {
  if (@available(iOS 13.0, *)) {
    // [UITextInteraction willMoveToView:] sometimes sets the textInput's inputDelegate
    // to nil. This is likely a bug in UIKit. In order to inform the keyboard of text
    // and selection changes when that happens, add a dummy UITextInteraction to this
    // view so it sets a valid inputDelegate that we can call textWillChange et al. on.
    // See https://github.com/flutter/engine/pull/32881.
    if (!self.inputDelegate && self.isFirstResponder) {
      [self addInteraction:self.textInteraction];
    }
  }

  NSString* newText = state[@"text"];
  BOOL textChanged = ![self.text isEqualToString:newText];
  if (textChanged) {
    [self.inputDelegate textWillChange:self];
    [self.text setString:newText];
  }
  NSInteger composingBase = [state[@"composingBase"] intValue];
  NSInteger composingExtent = [state[@"composingExtent"] intValue];
  NSRange composingRange = [self clampSelection:NSMakeRange(MIN(composingBase, composingExtent),
                                                            ABS(composingBase - composingExtent))
                                        forText:self.text];

  self.markedTextRange =
      composingRange.length > 0 ? [FlutterTextRange rangeWithNSRange:composingRange] : nil;

  NSRange selectedRange = [self clampSelectionFromBase:[state[@"selectionBase"] intValue]
                                                extent:[state[@"selectionExtent"] intValue]
                                               forText:self.text];

  NSRange oldSelectedRange = [(FlutterTextRange*)self.selectedTextRange range];
  if (!NSEqualRanges(selectedRange, oldSelectedRange)) {
    [self.inputDelegate selectionWillChange:self];

    [self setSelectedTextRangeLocal:[FlutterTextRange rangeWithNSRange:selectedRange]];

    _selectionAffinity = kTextAffinityDownstream;
    if ([state[@"selectionAffinity"] isEqualToString:@(kTextAffinityUpstream)]) {
      _selectionAffinity = kTextAffinityUpstream;
    }
    [self.inputDelegate selectionDidChange:self];
  }

  if (textChanged) {
    [self.inputDelegate textDidChange:self];
  }

  if (@available(iOS 13.0, *)) {
    if (_textInteraction) {
      [self removeInteraction:_textInteraction];
    }
  }
}

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

// Extracts the selection information from the editing state dictionary.
//
// The state may contain an invalid selection, such as when no selection was
// explicitly set in the framework. This is handled here by setting the
// selection to (0,0). In contrast, Android handles this situation by
// clearing the selection, but the result in both cases is that the cursor
// is placed at the beginning of the field.
- (NSRange)clampSelectionFromBase:(int)selectionBase
                           extent:(int)selectionExtent
                          forText:(NSString*)text {
  int loc = MIN(selectionBase, selectionExtent);
  int len = ABS(selectionExtent - selectionBase);
  return loc < 0 ? NSMakeRange(0, 0)
                 : [self clampSelection:NSMakeRange(loc, len) forText:self.text];
}

- (NSRange)clampSelection:(NSRange)range forText:(NSString*)text {
  NSUInteger start = MIN(MAX(range.location, 0), text.length);
  NSUInteger length = MIN(range.length, text.length - start);
  return NSMakeRange(start, length);
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
  // This probably needs to change (think it is getting overwritten by the updateSizeAndTransform
  // stuff for now).
  self.frame = isVisibleToAutofill ? CGRectMake(0, 0, 1, 1) : CGRectZero;
}

#pragma mark UIScribbleInteractionDelegate

- (void)scribbleInteractionWillBeginWriting:(UIScribbleInteraction*)interaction
    API_AVAILABLE(ios(14.0)) {
  _scribbleInteractionStatus = FlutterScribbleInteractionStatusStarted;
  [self.textInputDelegate flutterTextInputViewScribbleInteractionBegan:self];
}

- (void)scribbleInteractionDidFinishWriting:(UIScribbleInteraction*)interaction
    API_AVAILABLE(ios(14.0)) {
  _scribbleInteractionStatus = FlutterScribbleInteractionStatusEnding;
  [self.textInputDelegate flutterTextInputViewScribbleInteractionFinished:self];
}

- (BOOL)scribbleInteraction:(UIScribbleInteraction*)interaction
      shouldBeginAtLocation:(CGPoint)location API_AVAILABLE(ios(14.0)) {
  return YES;
}

- (BOOL)scribbleInteractionShouldDelayFocus:(UIScribbleInteraction*)interaction
    API_AVAILABLE(ios(14.0)) {
  return NO;
}

#pragma mark - UIResponder Overrides

- (BOOL)canBecomeFirstResponder {
  // Only the currently focused input field can
  // become the first responder. This prevents iOS
  // from changing focus by itself (the framework
  // focus will be out of sync if that happens).
  return _clientID != 0;
}

- (BOOL)resignFirstResponder {
  BOOL success = [super resignFirstResponder];
  if (success) {
    [self.textInputDelegate flutterTextInputViewDidResignFirstResponder:self];
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

#pragma mark - UIResponderStandardEditActions Overrides

- (void)cut:(id)sender {
  [UIPasteboard generalPasteboard].string = [self textInRange:_selectedTextRange];
  [self replaceRange:_selectedTextRange withText:@""];
}

- (void)copy:(id)sender {
  [UIPasteboard generalPasteboard].string = [self textInRange:_selectedTextRange];
}

- (void)paste:(id)sender {
  NSString* pasteboardString = [UIPasteboard generalPasteboard].string;
  if (pasteboardString != nil) {
    [self insertText:pasteboardString];
  }
}

- (void)delete:(id)sender {
  [self replaceRange:_selectedTextRange withText:@""];
}

- (void)selectAll:(id)sender {
  [self setSelectedTextRange:[self textRangeFromPosition:[self beginningOfDocument]
                                              toPosition:[self endOfDocument]]];
}

#pragma mark - UITextInput Overrides

- (id<UITextInputTokenizer>)tokenizer {
  if (_tokenizer == nil) {
    _tokenizer = [[FlutterTokenizer alloc] initWithTextInput:self];
  }
  return _tokenizer;
}

- (UITextRange*)selectedTextRange {
  return [_selectedTextRange copy];
}

// Change the range of selected text, without notifying the framework.
- (void)setSelectedTextRangeLocal:(UITextRange*)selectedTextRange {
  if (_selectedTextRange == selectedTextRange) {
    return;
  }
  if (self.hasText) {
    FlutterTextRange* flutterTextRange = (FlutterTextRange*)selectedTextRange;
    _selectedTextRange = [[FlutterTextRange
        rangeWithNSRange:fml::RangeForCharactersInRange(self.text, flutterTextRange.range)] copy];
  } else {
    _selectedTextRange = [selectedTextRange copy];
  }
}
- (void)setEnableInteractiveSelection:(BOOL)enableInteractiveSelection {
  _enableInteractiveSelection = enableInteractiveSelection;
}
- (void)setSelectedTextRange:(UITextRange*)selectedTextRange {
  if (!_enableInteractiveSelection) {
    return;
  }

  [self setSelectedTextRangeLocal:selectedTextRange];

  if (_enableDeltaModel) {
    [self updateEditingStateWithDelta:flutter::TextEditingDelta([self.text UTF8String])];
  } else {
    [self updateEditingState];
  }

  if (_scribbleInteractionStatus != FlutterScribbleInteractionStatusNone ||
      _scribbleFocusStatus == FlutterScribbleFocusStatusFocused) {
    NSAssert([selectedTextRange isKindOfClass:[FlutterTextRange class]],
             @"Expected a FlutterTextRange for range (got %@).", [selectedTextRange class]);
    FlutterTextRange* flutterTextRange = (FlutterTextRange*)selectedTextRange;
    if (flutterTextRange.range.length > 0) {
      [self.textInputDelegate flutterTextInputView:self showToolbar:_clientID];
    }
  }

  [self resetScribbleInteractionStatusIfEnding];
}

- (id)insertDictationResultPlaceholder {
  return @"";
}

- (void)removeDictationResultPlaceholder:(id)placeholder willInsertResult:(BOOL)willInsertResult {
}

- (NSString*)textInRange:(UITextRange*)range {
  if (!range) {
    return nil;
  }
  NSAssert([range isKindOfClass:[FlutterTextRange class]],
           @"Expected a FlutterTextRange for range (got %@).", [range class]);
  NSRange textRange = ((FlutterTextRange*)range).range;
  NSAssert(textRange.location != NSNotFound, @"Expected a valid text range.");
  // Sanitize the range to prevent going out of bounds.
  NSUInteger location = MIN(textRange.location, self.text.length);
  NSUInteger length = MIN(self.text.length - location, textRange.length);
  NSRange safeRange = NSMakeRange(location, length);
  return [self.text substringWithRange:safeRange];
}

// Replace the text within the specified range with the given text,
// without notifying the framework.
- (void)replaceRangeLocal:(NSRange)range withText:(NSString*)text {
  NSRange selectedRange = _selectedTextRange.range;

  // Adjust the text selection:
  // * reduce the length by the intersection length
  // * adjust the location by newLength - oldLength + intersectionLength
  NSRange intersectionRange = NSIntersectionRange(range, selectedRange);
  if (range.location <= selectedRange.location) {
    selectedRange.location += text.length - range.length;
  }
  if (intersectionRange.location != NSNotFound) {
    selectedRange.location += intersectionRange.length;
    selectedRange.length -= intersectionRange.length;
  }

  [self.text replaceCharactersInRange:[self clampSelection:range forText:self.text]
                           withString:text];
  [self setSelectedTextRangeLocal:[FlutterTextRange
                                      rangeWithNSRange:[self clampSelection:selectedRange
                                                                    forText:self.text]]];
}

- (void)replaceRange:(UITextRange*)range withText:(NSString*)text {
  NSString* textBeforeChange = [self.text copy];
  NSRange replaceRange = ((FlutterTextRange*)range).range;
  [self replaceRangeLocal:replaceRange withText:text];
  if (_enableDeltaModel) {
    NSRange nextReplaceRange = [self clampSelection:replaceRange forText:textBeforeChange];
    [self updateEditingStateWithDelta:flutter::TextEditingDelta(
                                          [textBeforeChange UTF8String],
                                          flutter::TextRange(
                                              nextReplaceRange.location,
                                              nextReplaceRange.location + nextReplaceRange.length),
                                          [text UTF8String])];
  } else {
    [self updateEditingState];
  }
}

- (BOOL)shouldChangeTextInRange:(UITextRange*)range replacementText:(NSString*)text {
  // `temporarilyDeletedComposedCharacter` should only be used during a single text change session.
  // So it needs to be cleared at the start of each text editting session.
  self.temporarilyDeletedComposedCharacter = nil;

  if (self.returnKeyType == UIReturnKeyDefault && [text isEqualToString:@"\n"]) {
    [self.textInputDelegate flutterTextInputView:self
                                   performAction:FlutterTextInputActionNewline
                                      withClient:_clientID];
    return YES;
  }

  if ([text isEqualToString:@"\n"]) {
    FlutterTextInputAction action;
    switch (self.returnKeyType) {
      case UIReturnKeyDefault:
        action = FlutterTextInputActionUnspecified;
        break;
      case UIReturnKeyDone:
        action = FlutterTextInputActionDone;
        break;
      case UIReturnKeyGo:
        action = FlutterTextInputActionGo;
        break;
      case UIReturnKeySend:
        action = FlutterTextInputActionSend;
        break;
      case UIReturnKeySearch:
      case UIReturnKeyGoogle:
      case UIReturnKeyYahoo:
        action = FlutterTextInputActionSearch;
        break;
      case UIReturnKeyNext:
        action = FlutterTextInputActionNext;
        break;
      case UIReturnKeyContinue:
        action = FlutterTextInputActionContinue;
        break;
      case UIReturnKeyJoin:
        action = FlutterTextInputActionJoin;
        break;
      case UIReturnKeyRoute:
        action = FlutterTextInputActionRoute;
        break;
      case UIReturnKeyEmergencyCall:
        action = FlutterTextInputActionEmergencyCall;
        break;
    }

    [self.textInputDelegate flutterTextInputView:self performAction:action withClient:_clientID];
    return NO;
  }

  return YES;
}

- (void)setMarkedText:(NSString*)markedText selectedRange:(NSRange)markedSelectedRange {
  NSString* textBeforeChange = [self.text copy];
  NSRange selectedRange = _selectedTextRange.range;
  NSRange markedTextRange = ((FlutterTextRange*)self.markedTextRange).range;
  NSRange actualReplacedRange;

  if (_scribbleInteractionStatus != FlutterScribbleInteractionStatusNone ||
      _scribbleFocusStatus != FlutterScribbleFocusStatusUnfocused) {
    return;
  }

  if (markedText == nil) {
    markedText = @"";
  }

  if (markedTextRange.length > 0) {
    // Replace text in the marked range with the new text.
    [self replaceRangeLocal:markedTextRange withText:markedText];
    actualReplacedRange = markedTextRange;
    markedTextRange.length = markedText.length;
  } else {
    // Replace text in the selected range with the new text.
    actualReplacedRange = selectedRange;
    [self replaceRangeLocal:selectedRange withText:markedText];
    markedTextRange = NSMakeRange(selectedRange.location, markedText.length);
  }

  self.markedTextRange =
      markedTextRange.length > 0 ? [FlutterTextRange rangeWithNSRange:markedTextRange] : nil;

  NSUInteger selectionLocation = markedSelectedRange.location + markedTextRange.location;
  selectedRange = NSMakeRange(selectionLocation, markedSelectedRange.length);
  [self setSelectedTextRangeLocal:[FlutterTextRange
                                      rangeWithNSRange:[self clampSelection:selectedRange
                                                                    forText:self.text]]];
  if (_enableDeltaModel) {
    NSRange nextReplaceRange = [self clampSelection:actualReplacedRange forText:textBeforeChange];
    [self updateEditingStateWithDelta:flutter::TextEditingDelta(
                                          [textBeforeChange UTF8String],
                                          flutter::TextRange(
                                              nextReplaceRange.location,
                                              nextReplaceRange.location + nextReplaceRange.length),
                                          [markedText UTF8String])];
  } else {
    [self updateEditingState];
  }
}

- (void)unmarkText {
  if (!self.markedTextRange) {
    return;
  }
  self.markedTextRange = nil;
  if (_enableDeltaModel) {
    [self updateEditingStateWithDelta:flutter::TextEditingDelta([self.text UTF8String])];
  } else {
    [self updateEditingState];
  }
}

- (UITextRange*)textRangeFromPosition:(UITextPosition*)fromPosition
                           toPosition:(UITextPosition*)toPosition {
  NSUInteger fromIndex = ((FlutterTextPosition*)fromPosition).index;
  NSUInteger toIndex = ((FlutterTextPosition*)toPosition).index;
  if (toIndex >= fromIndex) {
    return [FlutterTextRange rangeWithNSRange:NSMakeRange(fromIndex, toIndex - fromIndex)];
  } else {
    // toIndex can be smaller than fromIndex, because
    // UITextInputStringTokenizer does not handle CJK characters
    // well in some cases. See:
    // https://github.com/flutter/flutter/issues/58750#issuecomment-644469521
    // Swap fromPosition and toPosition to match the behavior of native
    // UITextViews.
    return [FlutterTextRange rangeWithNSRange:NSMakeRange(toIndex, fromIndex - toIndex)];
  }
}

- (NSUInteger)decrementOffsetPosition:(NSUInteger)position {
  return fml::RangeForCharacterAtIndex(self.text, MAX(0, position - 1)).location;
}

- (NSUInteger)incrementOffsetPosition:(NSUInteger)position {
  NSRange charRange = fml::RangeForCharacterAtIndex(self.text, position);
  return MIN(position + charRange.length, self.text.length);
}

- (UITextPosition*)positionFromPosition:(UITextPosition*)position offset:(NSInteger)offset {
  NSUInteger offsetPosition = ((FlutterTextPosition*)position).index;

  NSInteger newLocation = (NSInteger)offsetPosition + offset;
  if (newLocation < 0 || newLocation > (NSInteger)self.text.length) {
    return nil;
  }

  if (_scribbleInteractionStatus != FlutterScribbleInteractionStatusNone) {
    return [FlutterTextPosition positionWithIndex:newLocation];
  }

  if (offset >= 0) {
    for (NSInteger i = 0; i < offset && offsetPosition < self.text.length; ++i) {
      offsetPosition = [self incrementOffsetPosition:offsetPosition];
    }
  } else {
    for (NSInteger i = 0; i < ABS(offset) && offsetPosition > 0; ++i) {
      offsetPosition = [self decrementOffsetPosition:offsetPosition];
    }
  }
  return [FlutterTextPosition positionWithIndex:offsetPosition];
}

- (UITextPosition*)positionFromPosition:(UITextPosition*)position
                            inDirection:(UITextLayoutDirection)direction
                                 offset:(NSInteger)offset {
  // TODO(cbracken) Add RTL handling.
  switch (direction) {
    case UITextLayoutDirectionLeft:
    case UITextLayoutDirectionUp:
      return [self positionFromPosition:position offset:offset * -1];
    case UITextLayoutDirectionRight:
    case UITextLayoutDirectionDown:
      return [self positionFromPosition:position offset:1];
  }
}

- (UITextPosition*)beginningOfDocument {
  return [FlutterTextPosition positionWithIndex:0];
}

- (UITextPosition*)endOfDocument {
  return [FlutterTextPosition positionWithIndex:self.text.length];
}

- (NSComparisonResult)comparePosition:(UITextPosition*)position toPosition:(UITextPosition*)other {
  NSUInteger positionIndex = ((FlutterTextPosition*)position).index;
  NSUInteger otherIndex = ((FlutterTextPosition*)other).index;
  if (positionIndex < otherIndex) {
    return NSOrderedAscending;
  }
  if (positionIndex > otherIndex) {
    return NSOrderedDescending;
  }
  return NSOrderedSame;
}

- (NSInteger)offsetFromPosition:(UITextPosition*)from toPosition:(UITextPosition*)toPosition {
  return ((FlutterTextPosition*)toPosition).index - ((FlutterTextPosition*)from).index;
}

- (UITextPosition*)positionWithinRange:(UITextRange*)range
                   farthestInDirection:(UITextLayoutDirection)direction {
  NSUInteger index;
  switch (direction) {
    case UITextLayoutDirectionLeft:
    case UITextLayoutDirectionUp:
      index = ((FlutterTextPosition*)range.start).index;
      break;
    case UITextLayoutDirectionRight:
    case UITextLayoutDirectionDown:
      index = ((FlutterTextPosition*)range.end).index;
      break;
  }
  return [FlutterTextPosition positionWithIndex:index];
}

- (UITextRange*)characterRangeByExtendingPosition:(UITextPosition*)position
                                      inDirection:(UITextLayoutDirection)direction {
  NSUInteger positionIndex = ((FlutterTextPosition*)position).index;
  NSUInteger startIndex;
  NSUInteger endIndex;
  switch (direction) {
    case UITextLayoutDirectionLeft:
    case UITextLayoutDirectionUp:
      startIndex = [self decrementOffsetPosition:positionIndex];
      endIndex = positionIndex;
      break;
    case UITextLayoutDirectionRight:
    case UITextLayoutDirectionDown:
      startIndex = positionIndex;
      endIndex = [self incrementOffsetPosition:positionIndex];
      break;
  }
  return [FlutterTextRange rangeWithNSRange:NSMakeRange(startIndex, endIndex - startIndex)];
}

#pragma mark - UITextInput text direction handling

- (UITextWritingDirection)baseWritingDirectionForPosition:(UITextPosition*)position
                                              inDirection:(UITextStorageDirection)direction {
  // TODO(cbracken) Add RTL handling.
  return UITextWritingDirectionNatural;
}

- (void)setBaseWritingDirection:(UITextWritingDirection)writingDirection
                       forRange:(UITextRange*)range {
  // TODO(cbracken) Add RTL handling.
}

#pragma mark - UITextInput cursor, selection rect handling

- (void)setMarkedRect:(CGRect)markedRect {
  _markedRect = markedRect;
  // Invalidate the cache.
  _cachedFirstRect = kInvalidFirstRect;
}

// This method expects a 4x4 perspective matrix
// stored in a NSArray in column-major order.
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

  // Invalidate the cache.
  _cachedFirstRect = kInvalidFirstRect;
}

// Returns the bounding CGRect of the transformed incomingRect, in the view's
// coordinates.
- (CGRect)localRectFromFrameworkTransform:(CGRect)incomingRect {
  CGPoint points[] = {
      incomingRect.origin,
      CGPointMake(incomingRect.origin.x, incomingRect.origin.y + incomingRect.size.height),
      CGPointMake(incomingRect.origin.x + incomingRect.size.width, incomingRect.origin.y),
      CGPointMake(incomingRect.origin.x + incomingRect.size.width,
                  incomingRect.origin.y + incomingRect.size.height)};

  CGPoint origin = CGPointMake(CGFLOAT_MAX, CGFLOAT_MAX);
  CGPoint farthest = CGPointMake(-CGFLOAT_MAX, -CGFLOAT_MAX);

  for (int i = 0; i < 4; i++) {
    const CGPoint point = points[i];

    CGFloat x = _editableTransform.m11 * point.x + _editableTransform.m21 * point.y +
                _editableTransform.m41;
    CGFloat y = _editableTransform.m12 * point.x + _editableTransform.m22 * point.y +
                _editableTransform.m42;

    const CGFloat w = _editableTransform.m14 * point.x + _editableTransform.m24 * point.y +
                      _editableTransform.m44;

    if (w == 0.0) {
      return kInvalidFirstRect;
    } else if (w != 1.0) {
      x /= w;
      y /= w;
    }

    origin.x = MIN(origin.x, x);
    origin.y = MIN(origin.y, y);
    farthest.x = MAX(farthest.x, x);
    farthest.y = MAX(farthest.y, y);
  }
  return CGRectMake(origin.x, origin.y, farthest.x - origin.x, farthest.y - origin.y);
}

// The following methods are required to support force-touch cursor positioning
// and to position the
// candidates view for multi-stage input methods (e.g., Japanese) when using a
// physical keyboard.
- (CGRect)firstRectForRange:(UITextRange*)range {
  NSAssert([range.start isKindOfClass:[FlutterTextPosition class]],
           @"Expected a FlutterTextPosition for range.start (got %@).", [range.start class]);
  NSAssert([range.end isKindOfClass:[FlutterTextPosition class]],
           @"Expected a FlutterTextPosition for range.end (got %@).", [range.end class]);
  NSUInteger start = ((FlutterTextPosition*)range.start).index;
  NSUInteger end = ((FlutterTextPosition*)range.end).index;
  if (_markedTextRange != nil) {
    // The candidates view can't be shown if the framework has not sent the
    // first caret rect.
    if (CGRectEqualToRect(kInvalidFirstRect, _markedRect)) {
      return kInvalidFirstRect;
    }

    if (CGRectEqualToRect(_cachedFirstRect, kInvalidFirstRect)) {
      // If the width returned is too small, that means the framework sent us
      // the caret rect instead of the marked text rect. Expand it to 0.2 so
      // the IME candidates view would show up.
      CGRect rect = _markedRect;
      if (CGRectIsEmpty(rect)) {
        rect = CGRectInset(rect, -0.1, 0);
      }
      _cachedFirstRect = [self localRectFromFrameworkTransform:rect];
    }

    UIView* hostView = _textInputPlugin.hostView;
    NSAssert(hostView == nil || [self isDescendantOfView:hostView], @"%@ is not a descendant of %@",
             self, hostView);
    return hostView ? [hostView convertRect:_cachedFirstRect toView:self] : _cachedFirstRect;
  }

  if (_scribbleInteractionStatus == FlutterScribbleInteractionStatusNone &&
      _scribbleFocusStatus == FlutterScribbleFocusStatusUnfocused) {
    [self.textInputDelegate flutterTextInputView:self
            showAutocorrectionPromptRectForStart:start
                                             end:end
                                      withClient:_clientID];
  }

  NSUInteger first = start;
  if (end < start) {
    first = end;
  }
  FlutterTextRange* textRange = [FlutterTextRange
      rangeWithNSRange:fml::RangeForCharactersInRange(self.text, NSMakeRange(0, self.text.length))];
  for (NSUInteger i = 0; i < [_selectionRects count]; i++) {
    BOOL startsOnOrBeforeStartOfRange = _selectionRects[i].position <= first;
    BOOL isLastSelectionRect = i + 1 == [_selectionRects count];
    BOOL endOfTextIsAfterStartOfRange = isLastSelectionRect && textRange.range.length > first;
    BOOL nextSelectionRectIsAfterStartOfRange =
        !isLastSelectionRect && _selectionRects[i + 1].position > first;
    if (startsOnOrBeforeStartOfRange &&
        (endOfTextIsAfterStartOfRange || nextSelectionRectIsAfterStartOfRange)) {
      return _selectionRects[i].rect;
    }
  }

  return CGRectZero;
}

- (CGRect)caretRectForPosition:(UITextPosition*)position {
  // TODO(cbracken) Implement.

  // As of iOS 14.4, this call is used by iOS's
  // _UIKeyboardTextSelectionController to determine the position
  // of the floating cursor when the user force touches the space
  // bar to initiate floating cursor.
  //
  // It is recommended to return a value that's roughly the
  // center of kSpacePanBounds to make sure the floating cursor
  // has ample space in all directions and does not hit kSpacePanBounds.
  // See the comments in beginFloatingCursorAtPoint.
  return CGRectZero;
}

- (CGRect)bounds {
  return _isFloatingCursorActive ? kSpacePanBounds : super.bounds;
}

- (UITextPosition*)closestPositionToPoint:(CGPoint)point {
  if ([_selectionRects count] == 0) {
    NSAssert([_selectedTextRange.start isKindOfClass:[FlutterTextPosition class]],
             @"Expected a FlutterTextPosition for position (got %@).",
             [_selectedTextRange.start class]);
    NSUInteger currentIndex = ((FlutterTextPosition*)_selectedTextRange.start).index;
    return [FlutterTextPosition positionWithIndex:currentIndex];
  }

  FlutterTextRange* range = [FlutterTextRange
      rangeWithNSRange:fml::RangeForCharactersInRange(self.text, NSMakeRange(0, self.text.length))];
  return [self closestPositionToPoint:point withinRange:range];
}

- (NSArray*)selectionRectsForRange:(UITextRange*)range {
  // At least in the simulator, swapping to the Japanese keyboard crashes the app as this method
  // is called immediately with a UITextRange with a UITextPosition rather than FlutterTextPosition
  // for the start and end.
  if (![range.start isKindOfClass:[FlutterTextPosition class]]) {
    return @[];
  }
  NSAssert([range.start isKindOfClass:[FlutterTextPosition class]],
           @"Expected a FlutterTextPosition for range.start (got %@).", [range.start class]);
  NSAssert([range.end isKindOfClass:[FlutterTextPosition class]],
           @"Expected a FlutterTextPosition for range.end (got %@).", [range.end class]);
  NSUInteger start = ((FlutterTextPosition*)range.start).index;
  NSUInteger end = ((FlutterTextPosition*)range.end).index;
  NSMutableArray* rects = [[NSMutableArray alloc] init];
  for (NSUInteger i = 0; i < [_selectionRects count]; i++) {
    if (_selectionRects[i].position >= start && _selectionRects[i].position <= end) {
      float width = _selectionRects[i].rect.size.width;
      if (start == end) {
        width = 0;
      }
      CGRect rect = CGRectMake(_selectionRects[i].rect.origin.x, _selectionRects[i].rect.origin.y,
                               width, _selectionRects[i].rect.size.height);
      FlutterTextSelectionRect* selectionRect = [FlutterTextSelectionRect
          selectionRectWithRectAndInfo:rect
                              position:_selectionRects[i].position
                      writingDirection:UITextWritingDirectionNatural
                         containsStart:(i == 0)
                           containsEnd:(i == fml::RangeForCharactersInRange(
                                                 self.text, NSMakeRange(0, self.text.length))
                                                 .length)
                            isVertical:NO];
      [rects addObject:selectionRect];
    }
  }
  return rects;
}

- (UITextPosition*)closestPositionToPoint:(CGPoint)point withinRange:(UITextRange*)range {
  NSAssert([range.start isKindOfClass:[FlutterTextPosition class]],
           @"Expected a FlutterTextPosition for range.start (got %@).", [range.start class]);
  NSAssert([range.end isKindOfClass:[FlutterTextPosition class]],
           @"Expected a FlutterTextPosition for range.end (got %@).", [range.end class]);
  NSUInteger start = ((FlutterTextPosition*)range.start).index;
  NSUInteger end = ((FlutterTextPosition*)range.end).index;

  NSUInteger _closestIndex = 0;
  CGRect _closestRect = CGRectZero;
  NSUInteger _closestPosition = 0;
  for (NSUInteger i = 0; i < [_selectionRects count]; i++) {
    NSUInteger position = _selectionRects[i].position;
    if (position >= start && position <= end) {
      BOOL isFirst = _closestIndex == 0;
      if (isFirst || IsSelectionRectCloserToPoint(point, _selectionRects[i].rect, _closestRect,
                                                  /*checkRightBoundary=*/NO)) {
        _closestIndex = i;
        _closestRect = _selectionRects[i].rect;
        _closestPosition = position;
      }
    }
  }

  FlutterTextRange* textRange = [FlutterTextRange
      rangeWithNSRange:fml::RangeForCharactersInRange(self.text, NSMakeRange(0, self.text.length))];

  if ([_selectionRects count] > 0 && textRange.range.length == end) {
    NSUInteger i = [_selectionRects count] - 1;
    NSUInteger position = _selectionRects[i].position + 1;
    if (position <= end) {
      if (IsSelectionRectCloserToPoint(point, _selectionRects[i].rect, _closestRect,
                                       /*checkRightBoundary=*/YES)) {
        _closestPosition = position;
      }
    }
  }

  return [FlutterTextPosition positionWithIndex:_closestPosition];
}

- (UITextRange*)characterRangeAtPoint:(CGPoint)point {
  // TODO(cbracken) Implement.
  NSUInteger currentIndex = ((FlutterTextPosition*)_selectedTextRange.start).index;
  return [FlutterTextRange rangeWithNSRange:fml::RangeForCharacterAtIndex(self.text, currentIndex)];
}

// For "beginFloatingCursorAtPoint" and "updateFloatingCursorAtPoint", "point" is roughly:
//
// CGPoint(
//   width >= 0 ? point.x.clamp(boundingBox.left, boundingBox.right) : point.x,
//   height >= 0 ? point.y.clamp(boundingBox.top, boundingBox.bottom) : point.y,
// )
//   where
//     point = keyboardPanGestureRecognizer.translationInView(textInputView)
//           + caretRectForPosition
//     boundingBox = self.convertRect(bounds, fromView:textInputView)
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
- (void)beginFloatingCursorAtPoint:(CGPoint)point {
  _isFloatingCursorActive = true;
  [self.textInputDelegate flutterTextInputView:self
                          updateFloatingCursor:FlutterFloatingCursorDragStateStart
                                    withClient:_clientID
                                  withPosition:@{@"X" : @(point.x), @"Y" : @(point.y)}];
}

- (void)updateFloatingCursorAtPoint:(CGPoint)point {
  _isFloatingCursorActive = true;
  [self.textInputDelegate flutterTextInputView:self
                          updateFloatingCursor:FlutterFloatingCursorDragStateUpdate
                                    withClient:_clientID
                                  withPosition:@{@"X" : @(point.x), @"Y" : @(point.y)}];
}

- (void)endFloatingCursor {
  _isFloatingCursorActive = false;
  [self.textInputDelegate flutterTextInputView:self
                          updateFloatingCursor:FlutterFloatingCursorDragStateEnd
                                    withClient:_clientID
                                  withPosition:@{@"X" : @(0), @"Y" : @(0)}];
}

#pragma mark - UIKeyInput Overrides

- (void)updateEditingState {
  NSUInteger selectionBase = ((FlutterTextPosition*)_selectedTextRange.start).index;
  NSUInteger selectionExtent = ((FlutterTextPosition*)_selectedTextRange.end).index;

  // Empty compositing range is represented by the framework's TextRange.empty.
  NSInteger composingBase = -1;
  NSInteger composingExtent = -1;
  if (self.markedTextRange != nil) {
    composingBase = ((FlutterTextPosition*)self.markedTextRange.start).index;
    composingExtent = ((FlutterTextPosition*)self.markedTextRange.end).index;
  }
  NSDictionary* state = @{
    @"selectionBase" : @(selectionBase),
    @"selectionExtent" : @(selectionExtent),
    @"selectionAffinity" : @(_selectionAffinity),
    @"selectionIsDirectional" : @(false),
    @"composingBase" : @(composingBase),
    @"composingExtent" : @(composingExtent),
    @"text" : [NSString stringWithString:self.text],
  };

  if (_clientID == 0 && _autofillID != nil) {
    [self.textInputDelegate flutterTextInputView:self
                             updateEditingClient:_clientID
                                       withState:state
                                         withTag:_autofillID];
  } else {
    [self.textInputDelegate flutterTextInputView:self
                             updateEditingClient:_clientID
                                       withState:state];
  }
}

- (void)updateEditingStateWithDelta:(flutter::TextEditingDelta)delta {
  NSUInteger selectionBase = ((FlutterTextPosition*)_selectedTextRange.start).index;
  NSUInteger selectionExtent = ((FlutterTextPosition*)_selectedTextRange.end).index;

  // Empty compositing range is represented by the framework's TextRange.empty.
  NSInteger composingBase = -1;
  NSInteger composingExtent = -1;
  if (self.markedTextRange != nil) {
    composingBase = ((FlutterTextPosition*)self.markedTextRange.start).index;
    composingExtent = ((FlutterTextPosition*)self.markedTextRange.end).index;
  }

  NSDictionary* deltaToFramework = @{
    @"oldText" : @(delta.old_text().c_str()),
    @"deltaText" : @(delta.delta_text().c_str()),
    @"deltaStart" : @(delta.delta_start()),
    @"deltaEnd" : @(delta.delta_end()),
    @"selectionBase" : @(selectionBase),
    @"selectionExtent" : @(selectionExtent),
    @"selectionAffinity" : @(_selectionAffinity),
    @"selectionIsDirectional" : @(false),
    @"composingBase" : @(composingBase),
    @"composingExtent" : @(composingExtent),
  };

  NSDictionary* deltas = @{
    @"deltas" : @[ deltaToFramework ],
  };

  [self.textInputDelegate flutterTextInputView:self updateEditingClient:_clientID withDelta:deltas];
}

- (BOOL)hasText {
  return self.text.length > 0;
}

- (void)insertText:(NSString*)text {
  if (self.temporarilyDeletedComposedCharacter.length > 0 && text.length == 1 && !text.UTF8String &&
      [text characterAtIndex:0] == [self.temporarilyDeletedComposedCharacter characterAtIndex:0]) {
    // Workaround for https://github.com/flutter/flutter/issues/111494
    // TODO(cyanglaz): revert this workaround if when flutter supports a minimum iOS version which
    // this bug is fixed by Apple.
    text = self.temporarilyDeletedComposedCharacter;
    self.temporarilyDeletedComposedCharacter = nil;
  }

  NSMutableArray<FlutterTextSelectionRect*>* copiedRects =
      [[NSMutableArray alloc] initWithCapacity:[_selectionRects count]];
  NSAssert([_selectedTextRange.start isKindOfClass:[FlutterTextPosition class]],
           @"Expected a FlutterTextPosition for position (got %@).",
           [_selectedTextRange.start class]);
  NSUInteger insertPosition = ((FlutterTextPosition*)_selectedTextRange.start).index;
  for (NSUInteger i = 0; i < [_selectionRects count]; i++) {
    NSUInteger rectPosition = _selectionRects[i].position;
    if (rectPosition == insertPosition) {
      for (NSUInteger j = 0; j <= text.length; j++) {
        [copiedRects
            addObject:[FlutterTextSelectionRect selectionRectWithRect:_selectionRects[i].rect
                                                             position:rectPosition + j]];
      }
    } else {
      if (rectPosition > insertPosition) {
        rectPosition = rectPosition + text.length;
      }
      [copiedRects addObject:[FlutterTextSelectionRect selectionRectWithRect:_selectionRects[i].rect
                                                                    position:rectPosition]];
    }
  }

  _scribbleFocusStatus = FlutterScribbleFocusStatusUnfocused;
  [self resetScribbleInteractionStatusIfEnding];
  self.selectionRects = copiedRects;
  _selectionAffinity = kTextAffinityDownstream;
  [self replaceRange:_selectedTextRange withText:text];
}

- (UITextPlaceholder*)insertTextPlaceholderWithSize:(CGSize)size API_AVAILABLE(ios(13.0)) {
  [self.textInputDelegate flutterTextInputView:self
                 insertTextPlaceholderWithSize:size
                                    withClient:_clientID];
  _hasPlaceholder = YES;
  return [[FlutterTextPlaceholder alloc] init];
}

- (void)removeTextPlaceholder:(UITextPlaceholder*)textPlaceholder API_AVAILABLE(ios(13.0)) {
  _hasPlaceholder = NO;
  [self.textInputDelegate flutterTextInputView:self removeTextPlaceholder:_clientID];
}

- (void)deleteBackward {
  _selectionAffinity = kTextAffinityDownstream;
  _scribbleFocusStatus = FlutterScribbleFocusStatusUnfocused;
  [self resetScribbleInteractionStatusIfEnding];

  // When deleting Thai vowel, _selectedTextRange has location
  // but does not have length, so we have to manually set it.
  // In addition, we needed to delete only a part of grapheme cluster
  // because it is the expected behavior of Thai input.
  // https://github.com/flutter/flutter/issues/24203
  // https://github.com/flutter/flutter/issues/21745
  // https://github.com/flutter/flutter/issues/39399
  //
  // This is needed for correct handling of the deletion of Thai vowel input.
  // TODO(cbracken): Get a good understanding of expected behavior of Thai
  // input and ensure that this is the correct solution.
  // https://github.com/flutter/flutter/issues/28962
  if (_selectedTextRange.isEmpty && [self hasText]) {
    UITextRange* oldSelectedRange = _selectedTextRange;
    NSRange oldRange = ((FlutterTextRange*)oldSelectedRange).range;
    if (oldRange.location > 0) {
      NSRange newRange = NSMakeRange(oldRange.location - 1, 1);

      // We should check if the last character is a part of emoji.
      // If so, we must delete the entire emoji to prevent the text from being malformed.
      NSRange charRange = fml::RangeForCharacterAtIndex(self.text, oldRange.location - 1);
      if (IsEmoji(self.text, charRange)) {
        newRange = NSMakeRange(charRange.location, oldRange.location - charRange.location);
      }

      _selectedTextRange = [[FlutterTextRange rangeWithNSRange:newRange] copy];
    }
  }

  if (!_selectedTextRange.isEmpty) {
    // Cache the last deleted emoji to use for an iOS bug where the next
    // insertion corrupts the emoji characters.
    // See: https://github.com/flutter/flutter/issues/111494#issuecomment-1248441346
    if (IsEmoji(self.text, _selectedTextRange.range)) {
      NSString* deletedText = [self.text substringWithRange:_selectedTextRange.range];
      NSRange deleteFirstCharacterRange = fml::RangeForCharacterAtIndex(deletedText, 0);
      self.temporarilyDeletedComposedCharacter =
          [deletedText substringWithRange:deleteFirstCharacterRange];
    }
    [self replaceRange:_selectedTextRange withText:@""];
  }
}

- (void)postAccessibilityNotification:(UIAccessibilityNotifications)notification target:(id)target {
  UIAccessibilityPostNotification(notification, target);
}

- (void)accessibilityElementDidBecomeFocused {
  if ([self accessibilityElementIsFocused]) {
    // For most of the cases, this flutter text input view should never
    // receive the focus. If we do receive the focus, we make the best effort
    // to send the focus back to the real text field.
    FML_DCHECK(_backingTextInputAccessibilityObject);
    [self postAccessibilityNotification:UIAccessibilityScreenChangedNotification
                                 target:_backingTextInputAccessibilityObject];
  }
}

- (BOOL)accessibilityElementsHidden {
  return !_accessibilityEnabled;
}

- (void)resetScribbleInteractionStatusIfEnding {
  if (_scribbleInteractionStatus == FlutterScribbleInteractionStatusEnding) {
    _scribbleInteractionStatus = FlutterScribbleInteractionStatusNone;
  }
}

#pragma mark - Key Events Handling
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

@end

@interface FlutterSecureTextInputView ()
@property(nonatomic, retain, readonly) UITextField* textField;
@end

@implementation FlutterSecureTextInputView {
  UITextField* _textField;
}

- (UITextField*)textField {
  if (!_textField) {
    _textField = [[UITextField alloc] init];
  }
  return _textField;
}

- (BOOL)isKindOfClass:(Class)aClass {
  return [super isKindOfClass:aClass] || (aClass == [UITextField class]);
}

- (NSMethodSignature*)methodSignatureForSelector:(SEL)aSelector {
  NSMethodSignature* signature = [super methodSignatureForSelector:aSelector];
  if (!signature) {
    signature = [self.textField methodSignatureForSelector:aSelector];
  }
  return signature;
}

- (void)forwardInvocation:(NSInvocation*)anInvocation {
  [anInvocation invokeWithTarget:self.textField];
}

@end
