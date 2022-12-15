// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextInputPlugin.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextInputClient.h"

#import <Foundation/Foundation.h>

FLUTTER_ASSERT_ARC

// A delay before enabling the accessibility of FlutterTextInputView after
// it is activated.
static constexpr double kUITextInputAccessibilityEnablingDelaySeconds = 0.5;

// The "canonical" invalid CGRect, similar to CGRectNull, used to
// indicate a CGRect involved in firstRectForRange calculation is
// invalid. The specific value is chosen so that if firstRectForRange
// returns kInvalidFirstRect, iOS will not show the IME candidates view.
const CGRect kInvalidFirstRect = {{-1, -1}, {9999, 9999}};

#pragma mark - TextInput channel method names.
// See https://api.flutter.dev/flutter/services/SystemChannels/textInput-constant.html
static NSString* const kShowMethod = @"TextInput.show";
static NSString* const kHideMethod = @"TextInput.hide";
static NSString* const kSetClientMethod = @"TextInput.setClient";
static NSString* const kSetPlatformViewClientMethod = @"TextInput.setPlatformViewClient";
static NSString* const kSetEditingStateMethod = @"TextInput.setEditingState";
static NSString* const kClearClientMethod = @"TextInput.clearClient";
static NSString* const kSetEditableSizeAndTransformMethod =
    @"TextInput.setEditableSizeAndTransform";
static NSString* const kSetMarkedTextRectMethod = @"TextInput.setMarkedTextRect";
static NSString* const kFinishAutofillContextMethod = @"TextInput.finishAutofillContext";
static NSString* const kSetSelectionRectsMethod = @"Scribble.setSelectionRects";
static NSString* const kStartLiveTextInputMethod = @"TextInput.startLiveTextInput";

#pragma mark - TextInputConfiguration Field Names
static NSString* const kSecureTextEntry = @"obscureText";
static NSString* const kKeyboardType = @"inputType";
static NSString* const kKeyboardAppearance = @"keyboardAppearance";
static NSString* const kInputAction = @"inputAction";
static NSString* const kEnableDeltaModel = @"enableDeltaModel";
static NSString* const kEnableInteractiveSelection = @"enableInteractiveSelection";

static NSString* const kSmartDashesType = @"smartDashesType";
static NSString* const kSmartQuotesType = @"smartQuotesType";

static NSString* const kAssociatedAutofillFields = @"fields";

// TextInputConfiguration.autofill and sub-field names
static NSString* const kAutofillProperties = @"autofill";
static NSString* const kAutofillID = @"uniqueIdentifier";
static NSString* const kAutofillEditingValue = @"editingValue";
static NSString* const kAutofillHints = @"hints";

static NSString* const kAutocorrectionType = @"autocorrect";

#pragma mark - Static Functions
// "TextInputType.none" is a made-up input type that's typically
// used when there's an in-app virtual keyboard. If
// "TextInputType.none" is specified, disable the system
// keyboard.
static BOOL ShouldShowSystemKeyboard(NSDictionary* type) {
  NSString* inputType = type[@"name"];
  return ![inputType isEqualToString:@"TextInputType.none"];
}
static UIKeyboardType ToUIKeyboardType(NSDictionary* type) {
  NSString* inputType = type[@"name"];
  if ([inputType isEqualToString:@"TextInputType.address"]) {
    return UIKeyboardTypeDefault;
  }
  if ([inputType isEqualToString:@"TextInputType.datetime"]) {
    return UIKeyboardTypeNumbersAndPunctuation;
  }
  if ([inputType isEqualToString:@"TextInputType.emailAddress"]) {
    return UIKeyboardTypeEmailAddress;
  }
  if ([inputType isEqualToString:@"TextInputType.multiline"]) {
    return UIKeyboardTypeDefault;
  }
  if ([inputType isEqualToString:@"TextInputType.name"]) {
    return UIKeyboardTypeNamePhonePad;
  }
  if ([inputType isEqualToString:@"TextInputType.number"]) {
    if ([type[@"signed"] boolValue]) {
      return UIKeyboardTypeNumbersAndPunctuation;
    }
    if ([type[@"decimal"] boolValue]) {
      return UIKeyboardTypeDecimalPad;
    }
    return UIKeyboardTypeNumberPad;
  }
  if ([inputType isEqualToString:@"TextInputType.phone"]) {
    return UIKeyboardTypePhonePad;
  }
  if ([inputType isEqualToString:@"TextInputType.text"]) {
    return UIKeyboardTypeDefault;
  }
  if ([inputType isEqualToString:@"TextInputType.url"]) {
    return UIKeyboardTypeURL;
  }
  return UIKeyboardTypeDefault;
}

static UITextAutocapitalizationType ToUITextAutoCapitalizationType(NSDictionary* type) {
  NSString* textCapitalization = type[@"textCapitalization"];
  if ([textCapitalization isEqualToString:@"TextCapitalization.characters"]) {
    return UITextAutocapitalizationTypeAllCharacters;
  } else if ([textCapitalization isEqualToString:@"TextCapitalization.sentences"]) {
    return UITextAutocapitalizationTypeSentences;
  } else if ([textCapitalization isEqualToString:@"TextCapitalization.words"]) {
    return UITextAutocapitalizationTypeWords;
  }
  return UITextAutocapitalizationTypeNone;
}

static UIReturnKeyType ToUIReturnKeyType(NSString* inputType) {
  // Where did the term "unspecified" come from? iOS has a "default" and Android
  // has "unspecified." These 2 terms seem to mean the same thing but we need
  // to pick just one. "unspecified" was chosen because "default" is often a
  // reserved word in languages with switch statements (dart, java, etc).
  if ([inputType isEqualToString:@"TextInputAction.unspecified"]) {
    return UIReturnKeyDefault;
  }

  if ([inputType isEqualToString:@"TextInputAction.done"]) {
    return UIReturnKeyDone;
  }

  if ([inputType isEqualToString:@"TextInputAction.go"]) {
    return UIReturnKeyGo;
  }

  if ([inputType isEqualToString:@"TextInputAction.send"]) {
    return UIReturnKeySend;
  }

  if ([inputType isEqualToString:@"TextInputAction.search"]) {
    return UIReturnKeySearch;
  }

  if ([inputType isEqualToString:@"TextInputAction.next"]) {
    return UIReturnKeyNext;
  }

  if ([inputType isEqualToString:@"TextInputAction.continueAction"]) {
    return UIReturnKeyContinue;
  }

  if ([inputType isEqualToString:@"TextInputAction.join"]) {
    return UIReturnKeyJoin;
  }

  if ([inputType isEqualToString:@"TextInputAction.route"]) {
    return UIReturnKeyRoute;
  }

  if ([inputType isEqualToString:@"TextInputAction.emergencyCall"]) {
    return UIReturnKeyEmergencyCall;
  }

  if ([inputType isEqualToString:@"TextInputAction.newline"]) {
    return UIReturnKeyDefault;
  }

  // Present default key if bad input type is given.
  return UIReturnKeyDefault;
}

static UITextContentType ToUITextContentType(NSArray<NSString*>* hints) {
  if (!hints || hints.count == 0) {
    // If no hints are specified, use the default content type nil.
    return nil;
  }

  NSString* hint = hints[0];
  if ([hint isEqualToString:@"addressCityAndState"]) {
    return UITextContentTypeAddressCityAndState;
  }

  if ([hint isEqualToString:@"addressState"]) {
    return UITextContentTypeAddressState;
  }

  if ([hint isEqualToString:@"addressCity"]) {
    return UITextContentTypeAddressCity;
  }

  if ([hint isEqualToString:@"sublocality"]) {
    return UITextContentTypeSublocality;
  }

  if ([hint isEqualToString:@"streetAddressLine1"]) {
    return UITextContentTypeStreetAddressLine1;
  }

  if ([hint isEqualToString:@"streetAddressLine2"]) {
    return UITextContentTypeStreetAddressLine2;
  }

  if ([hint isEqualToString:@"countryName"]) {
    return UITextContentTypeCountryName;
  }

  if ([hint isEqualToString:@"fullStreetAddress"]) {
    return UITextContentTypeFullStreetAddress;
  }

  if ([hint isEqualToString:@"postalCode"]) {
    return UITextContentTypePostalCode;
  }

  if ([hint isEqualToString:@"location"]) {
    return UITextContentTypeLocation;
  }

  if ([hint isEqualToString:@"creditCardNumber"]) {
    return UITextContentTypeCreditCardNumber;
  }

  if ([hint isEqualToString:@"email"]) {
    return UITextContentTypeEmailAddress;
  }

  if ([hint isEqualToString:@"jobTitle"]) {
    return UITextContentTypeJobTitle;
  }

  if ([hint isEqualToString:@"givenName"]) {
    return UITextContentTypeGivenName;
  }

  if ([hint isEqualToString:@"middleName"]) {
    return UITextContentTypeMiddleName;
  }

  if ([hint isEqualToString:@"familyName"]) {
    return UITextContentTypeFamilyName;
  }

  if ([hint isEqualToString:@"name"]) {
    return UITextContentTypeName;
  }

  if ([hint isEqualToString:@"namePrefix"]) {
    return UITextContentTypeNamePrefix;
  }

  if ([hint isEqualToString:@"nameSuffix"]) {
    return UITextContentTypeNameSuffix;
  }

  if ([hint isEqualToString:@"nickname"]) {
    return UITextContentTypeNickname;
  }

  if ([hint isEqualToString:@"organizationName"]) {
    return UITextContentTypeOrganizationName;
  }

  if ([hint isEqualToString:@"telephoneNumber"]) {
    return UITextContentTypeTelephoneNumber;
  }

  if ([hint isEqualToString:@"password"]) {
    return UITextContentTypePassword;
  }

  if (@available(iOS 12.0, *)) {
    if ([hint isEqualToString:@"oneTimeCode"]) {
      return UITextContentTypeOneTimeCode;
    }

    if ([hint isEqualToString:@"newPassword"]) {
      return UITextContentTypeNewPassword;
    }
  }

  return hints[0];
}

// Retrieves the autofillID from an input field's configuration. Returns
// nil if the field is nil and the input field is not a password field.
static NSString* AutofillIDFromDictionary(NSDictionary* dictionary) {
  NSDictionary* autofill = dictionary[kAutofillProperties];
  if (autofill) {
    return autofill[kAutofillID];
  }

  // When autofill is nil, the field may still need an autofill id
  // if the field is for password.
  return [dictionary[kSecureTextEntry] boolValue] ? @"password" : nil;
}

// # Autofill Implementation Notes:
//
// Currently there're 2 types of autofills on iOS:
// - Regular autofill, including contact information and one-time-code,
//   takes place in the form of predictive text in the quick type bar.
//   This type of autofill does not save user input, and the keyboard
//   currently only populates the focused field when a predictive text entry
//   is selected by the user.
//
// - Password autofill, includes automatic strong password and regular
//   password autofill. The former happens automatically when a
//   "new password" field is detected and focused, and only that password
//   field will be populated. The latter appears in the quick type bar when
//   an eligible input field (which either has a UITextContentTypePassword
//   contentType, or is a secure text entry) becomes the first responder, and may
//   fill both the username and the password fields. iOS will attempt
//   to save user input for both kinds of password fields. It's relatively
//   tricky to deal with password autofill since it can autofill more than one
//   field at a time and may employ heuristics based on what other text fields
//   are in the same view controller.
//
// When a flutter text field is focused, and autofill is not explicitly disabled
// for it ("autofillable"), the framework collects its attributes and checks if
// it's in an AutofillGroup, and collects the attributes of other autofillable
// text fields in the same AutofillGroup if so. The attributes are sent to the
// text input plugin via a "TextInput.setClient" platform channel message. If
// autofill is disabled for a text field, its "autofill" field will be nil in
// the configuration json.
//
// The text input plugin then tries to determine which kind of autofill the text
// field needs. If the AutofillGroup the text field belongs to contains an
// autofillable text field that's password related, this text 's autofill type
// will be kFlutterAutofillTypePassword. If autofill is disabled for a text field,
// then its type will be kFlutterAutofillTypeNone. Otherwise the text field will
// have an autofill type of kFlutterAutofillTypeRegular.
//
// The text input plugin creates a new UIView for every kFlutterAutofillTypeNone
// text field. The UIView instance is never reused for other flutter text fields
// since the software keyboard often uses the identity of a UIView to distinguish
// different views and provides the same predictive text suggestions or restore
// the composing region if a UIView is reused for a different flutter text field.
//
// The text input plugin creates a new "autofill context" if the text field has
// the type of kFlutterAutofillTypePassword, to represent the AutofillGroup of
// the text field, and creates one FlutterTextInputView for every text field in
// the AutofillGroup.
//
// The text input plugin will try to reuse a UIView if a flutter text field's
// type is kFlutterAutofillTypeRegular, and has the same autofill id.
typedef NS_ENUM(NSInteger, FlutterAutofillType) {
  // The field does not have autofillable content. Additionally if
  // the field is currently in the autofill context, it will be
  // removed from the context without triggering autofill save.
  kFlutterAutofillTypeNone,
  kFlutterAutofillTypeRegular,
  kFlutterAutofillTypePassword,
};

static BOOL IsFieldPasswordRelated(NSDictionary* configuration) {
  // Autofill is explicitly disabled if the id isn't present.
  if (!AutofillIDFromDictionary(configuration)) {
    return NO;
  }

  BOOL isSecureTextEntry = [configuration[kSecureTextEntry] boolValue];
  if (isSecureTextEntry) {
    return YES;
  }

  NSDictionary* autofill = configuration[kAutofillProperties];
  UITextContentType contentType = ToUITextContentType(autofill[kAutofillHints]);

  if ([contentType isEqualToString:UITextContentTypePassword] ||
      [contentType isEqualToString:UITextContentTypeUsername]) {
    return YES;
  }

  if (@available(iOS 12.0, *)) {
    if ([contentType isEqualToString:UITextContentTypeNewPassword]) {
      return YES;
    }
  }
  return NO;
}

static FlutterAutofillType AutofillTypeOf(NSDictionary* configuration) {
  for (NSDictionary* field in configuration[kAssociatedAutofillFields]) {
    if (IsFieldPasswordRelated(field)) {
      return kFlutterAutofillTypePassword;
    }
  }

  if (IsFieldPasswordRelated(configuration)) {
    return kFlutterAutofillTypePassword;
  }

  NSDictionary* autofill = configuration[kAutofillProperties];
  UITextContentType contentType = ToUITextContentType(autofill[kAutofillHints]);
  return !autofill || [contentType isEqualToString:@""] ? kFlutterAutofillTypeNone
                                                        : kFlutterAutofillTypeRegular;
}

#pragma mark - FlutterTextPosition

@implementation FlutterTextPosition

+ (instancetype)positionWithIndex:(NSUInteger)index {
  return [[FlutterTextPosition alloc] initWithIndex:index];
}

- (instancetype)initWithIndex:(NSUInteger)index {
  self = [super init];
  if (self) {
    _index = index;
  }
  return self;
}

@end

#pragma mark - FlutterTextRange

@implementation FlutterTextRange

+ (instancetype)rangeWithNSRange:(NSRange)range {
  return [[FlutterTextRange alloc] initWithNSRange:range];
}

- (instancetype)initWithNSRange:(NSRange)range {
  self = [super init];
  if (self) {
    _range = range;
  }
  return self;
}

- (UITextPosition*)start {
  return [FlutterTextPosition positionWithIndex:self.range.location];
}

- (UITextPosition*)end {
  return [FlutterTextPosition positionWithIndex:self.range.location + self.range.length];
}

- (BOOL)isEmpty {
  return self.range.length == 0;
}

- (id)copyWithZone:(NSZone*)zone {
  return [[FlutterTextRange allocWithZone:zone] initWithNSRange:self.range];
}

- (BOOL)isEqualTo:(FlutterTextRange*)other {
  return NSEqualRanges(self.range, other.range);
}
@end

#pragma mark - FlutterTokenizer

@interface FlutterTokenizer ()

@property(nonatomic, weak) UIView<FlutterTextInputClient>* textInputView;

@end

@implementation FlutterTokenizer

- (instancetype)initWithTextInput:(UIResponder<UITextInput>*)textInput {
  NSAssert([textInput conformsToProtocol:@protocol(FlutterTextInputClient)],
           @"The FlutterTokenizer can only be used in a FlutterTextInputClient");
  self = [super initWithTextInput:textInput];
  if (self) {
    _textInputView = (UIView<FlutterTextInputClient>*)textInput;
  }
  return self;
}

- (UITextRange*)rangeEnclosingPosition:(UITextPosition*)position
                       withGranularity:(UITextGranularity)granularity
                           inDirection:(UITextDirection)direction {
  UITextRange* result;
  switch (granularity) {
    case UITextGranularityLine:
      // The default UITextInputStringTokenizer does not handle line granularity
      // correctly. We need to implement our own line tokenizer.
      result = [self lineEnclosingPosition:position];
      break;
    case UITextGranularityCharacter:
    case UITextGranularityWord:
    case UITextGranularitySentence:
    case UITextGranularityParagraph:
    case UITextGranularityDocument:
      // The UITextInputStringTokenizer can handle all these cases correctly.
      result = [super rangeEnclosingPosition:position
                             withGranularity:granularity
                                 inDirection:direction];
      break;
  }
  return result;
}

- (UITextRange*)lineEnclosingPosition:(UITextPosition*)position {
  // Gets the first line break position after the input position.
  NSString* textAfter = [_textInputView
      textInRange:[_textInputView textRangeFromPosition:position
                                             toPosition:[_textInputView endOfDocument]]];
  NSArray<NSString*>* linesAfter = [textAfter componentsSeparatedByString:@"\n"];
  NSInteger offSetToLineBreak = [linesAfter firstObject].length;
  UITextPosition* lineBreakAfter = [_textInputView positionFromPosition:position
                                                                 offset:offSetToLineBreak];
  // Gets the first line break position before the input position.
  NSString* textBefore = [_textInputView
      textInRange:[_textInputView textRangeFromPosition:[_textInputView beginningOfDocument]
                                             toPosition:position]];
  NSArray<NSString*>* linesBefore = [textBefore componentsSeparatedByString:@"\n"];
  NSInteger offSetFromLineBreak = [linesBefore lastObject].length;
  UITextPosition* lineBreakBefore = [_textInputView positionFromPosition:position
                                                                  offset:-offSetFromLineBreak];

  return [_textInputView textRangeFromPosition:lineBreakBefore toPosition:lineBreakAfter];
}

@end

#pragma mark - FlutterTextSelectionRect

@implementation FlutterTextSelectionRect

@synthesize rect = _rect;
@synthesize writingDirection = _writingDirection;
@synthesize containsStart = _containsStart;
@synthesize containsEnd = _containsEnd;
@synthesize isVertical = _isVertical;

+ (instancetype)selectionRectWithRectAndInfo:(CGRect)rect
                                    position:(NSUInteger)position
                            writingDirection:(NSWritingDirection)writingDirection
                               containsStart:(BOOL)containsStart
                                 containsEnd:(BOOL)containsEnd
                                  isVertical:(BOOL)isVertical {
  return [[FlutterTextSelectionRect alloc] initWithRectAndInfo:rect
                                                      position:position
                                              writingDirection:writingDirection
                                                 containsStart:containsStart
                                                   containsEnd:containsEnd
                                                    isVertical:isVertical];
}

+ (instancetype)selectionRectWithRect:(CGRect)rect position:(NSUInteger)position {
  return [[FlutterTextSelectionRect alloc] initWithRectAndInfo:rect
                                                      position:position
                                              writingDirection:UITextWritingDirectionNatural
                                                 containsStart:NO
                                                   containsEnd:NO
                                                    isVertical:NO];
}

- (instancetype)initWithRectAndInfo:(CGRect)rect
                           position:(NSUInteger)position
                   writingDirection:(NSWritingDirection)writingDirection
                      containsStart:(BOOL)containsStart
                        containsEnd:(BOOL)containsEnd
                         isVertical:(BOOL)isVertical {
  self = [super init];
  if (self) {
    self.rect = rect;
    self.position = position;
    self.writingDirection = writingDirection;
    self.containsStart = containsStart;
    self.containsEnd = containsEnd;
    self.isVertical = isVertical;
  }
  return self;
}

@end

#pragma mark - FlutterTextPlaceholder

@implementation FlutterTextPlaceholder

- (NSArray<UITextSelectionRect*>*)rects {
  // Returning anything other than an empty array here seems to cause PencilKit to enter an
  // infinite loop of allocating placeholders until the app crashes
  return @[];
}

@end

static void ConfigureInputClientWithDictionary(UIView<FlutterTextInputClient>* client,
                                               NSDictionary* configuration) {
  NSDictionary* inputType = configuration[kKeyboardType];
  NSString* keyboardAppearance = configuration[kKeyboardAppearance];
  NSDictionary* autofill = configuration[kAutofillProperties];

  client.secureTextEntry = [configuration[kSecureTextEntry] boolValue];
  client.enableDeltaModel = [configuration[kEnableDeltaModel] boolValue];

  client.enableSoftwareKeyboard = ShouldShowSystemKeyboard(inputType);
  client.keyboardType = ToUIKeyboardType(inputType);
  client.returnKeyType = ToUIReturnKeyType(configuration[kInputAction]);
  client.autocapitalizationType = ToUITextAutoCapitalizationType(configuration);
  client.enableInteractiveSelection = [configuration[kEnableInteractiveSelection] boolValue];
  NSString* smartDashesType = configuration[kSmartDashesType];
  // This index comes from the SmartDashesType enum in the framework.
  bool smartDashesIsDisabled = smartDashesType && [smartDashesType isEqualToString:@"0"];
  client.smartDashesType =
      smartDashesIsDisabled ? UITextSmartDashesTypeNo : UITextSmartDashesTypeYes;
  NSString* smartQuotesType = configuration[kSmartQuotesType];
  // This index comes from the SmartQuotesType enum in the framework.
  bool smartQuotesIsDisabled = smartQuotesType && [smartQuotesType isEqualToString:@"0"];
  client.smartQuotesType =
      smartQuotesIsDisabled ? UITextSmartQuotesTypeNo : UITextSmartQuotesTypeYes;
  if ([keyboardAppearance isEqualToString:@"Brightness.dark"]) {
    client.keyboardAppearance = UIKeyboardAppearanceDark;
  } else if ([keyboardAppearance isEqualToString:@"Brightness.light"]) {
    client.keyboardAppearance = UIKeyboardAppearanceLight;
  } else {
    client.keyboardAppearance = UIKeyboardAppearanceDefault;
  }
  NSString* autocorrect = configuration[kAutocorrectionType];
  client.autocorrectionType = autocorrect && ![autocorrect boolValue]
                                  ? UITextAutocorrectionTypeNo
                                  : UITextAutocorrectionTypeDefault;
  if ([client conformsToProtocol:@protocol(FlutterTextAutofillClient)]) {
    UIView<FlutterTextAutofillClient, FlutterTextInputClient>* autofillClient =
        (UIView<FlutterTextAutofillClient, FlutterTextInputClient>*)client;
    autofillClient.autofillID = AutofillIDFromDictionary(configuration);
    // The input field needs to be visible for the system autofill
    // to find it.
    autofillClient.isVisibleToAutofill = autofill || client.isSecureTextEntry;
  }

  if (autofill == nil) {
    client.textContentType = @"";
  } else {
    client.textContentType = ToUITextContentType(autofill[kAutofillHints]);
    [client setTextInputState:autofill[kAutofillEditingValue]];
  }
}

/**
 * Hides `FlutterTextInputView` from iOS accessibility system so it
 * does not show up twice, once where it is in the `UIView` hierarchy,
 * and a second time as part of the `SemanticsObject` hierarchy.
 *
 * This prevents the `FlutterTextInputView` from receiving the focus
 * due to swiping gesture.
 *
 * There are other cases the `FlutterTextInputView` may receive
 * focus. One example is during screen changes, the accessibility
 * tree will undergo a dramatic structural update. The Voiceover may
 * decide to focus the `FlutterTextInputView` that is not involved
 * in the structural update instead. If that happens, the
 * `FlutterTextInputView` will make a best effort to direct the
 * focus back to the `SemanticsObject`.
 */
@interface FlutterTextInputViewAccessibilityHider : UIView {
}

@end

@implementation FlutterTextInputViewAccessibilityHider {
}

- (BOOL)accessibilityElementsHidden {
  return YES;
}

@end

@interface FlutterTextInputPlugin ()
- (void)enableActiveViewAccessibility;
@end

@interface FlutterTimerProxy : NSObject
@property(nonatomic, weak) FlutterTextInputPlugin* target;
@end

@implementation FlutterTimerProxy

+ (instancetype)proxyWithTarget:(FlutterTextInputPlugin*)target {
  FlutterTimerProxy* proxy = [[self alloc] init];
  if (proxy) {
    proxy.target = target;
  }
  return proxy;
}

- (void)enableActiveViewAccessibility {
  [self.target enableActiveViewAccessibility];
}

@end

@interface FlutterTextInputPlugin ()
// The current password-autofillable input fields that have yet to be saved.
@property(nonatomic, readonly)
    NSMutableDictionary<NSString*, UIView<FlutterTextInputClient, FlutterTextAutofillClient>*>*
        autofillContext;
@property(nonatomic, retain) UIView<FlutterTextInputClient>* activeClient;
@property(nonatomic, retain) FlutterTextInputViewAccessibilityHider* inputHider;
@property(nonatomic, readonly) id<FlutterViewResponder> viewResponder;
@property(nonatomic, weak) id<FlutterTextInputDelegate> textInputDelegate;

@end

@implementation FlutterTextInputPlugin {
  NSTimer* _enableFlutterTextInputViewAccessibilityTimer;
}

- (instancetype)initWithDelegate:(id<FlutterTextInputDelegate>)textInputDelegate {
  self = [super init];

  if (self) {
    // `_textInputDelegate` is a weak reference because it should retain FlutterTextInputPlugin.
    _textInputDelegate = textInputDelegate;
    _autofillContext = [[NSMutableDictionary alloc] init];
    _inputHider = [[FlutterTextInputViewAccessibilityHider alloc] init];
    _scribbleElements = [[NSMutableDictionary alloc] init];
  }

  return self;
}

- (void)dealloc {
  [self hideTextInput];
}

- (void)removeEnableFlutterTextInputViewAccessibilityTimer {
  if (_enableFlutterTextInputViewAccessibilityTimer) {
    [_enableFlutterTextInputViewAccessibilityTimer invalidate];
    _enableFlutterTextInputViewAccessibilityTimer = nil;
  }
}

- (UIView<UITextInput>*)textInputView {
  return _activeClient;
}

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
  NSString* method = call.method;
  id args = call.arguments;
  if ([method isEqualToString:kShowMethod]) {
    [self showTextInput];
    result(nil);
  } else if ([method isEqualToString:kHideMethod]) {
    [self hideTextInput];
    result(nil);
  } else if ([method isEqualToString:kSetClientMethod]) {
    [self setTextInputClient:[args[0] intValue] withConfiguration:args[1]];
    result(nil);
  } else if ([method isEqualToString:kSetPlatformViewClientMethod]) {
    // This method call has a `platformViewId` argument, but we do not need it for iOS for now.
    [self setPlatformViewTextInputClient];
    result(nil);
  } else if ([method isEqualToString:kSetEditingStateMethod]) {
    [self setTextInputEditingState:args];
    result(nil);
  } else if ([method isEqualToString:kClearClientMethod]) {
    [self clearTextInputClient];
    result(nil);
  } else if ([method isEqualToString:kSetEditableSizeAndTransformMethod]) {
    [self setEditableSizeAndTransform:args];
    result(nil);
  } else if ([method isEqualToString:kSetMarkedTextRectMethod]) {
    [self updateMarkedRect:args];
    result(nil);
  } else if ([method isEqualToString:kFinishAutofillContextMethod]) {
    [self triggerAutofillSave:[args boolValue]];
    result(nil);
  } else if ([method isEqualToString:kSetSelectionRectsMethod]) {
    [self setSelectionRects:args];
    result(nil);
  } else if ([method isEqualToString:kStartLiveTextInputMethod]) {
    [self startLiveTextInput];
    result(nil);
  } else {
    result(FlutterMethodNotImplemented);
  }
}

- (void)setEditableSizeAndTransform:(NSDictionary*)dictionary {
  CGSize size = CGSizeZero;
  if (FlutterTextInputPlugin.isScribbleAvailable) {
    // This is necessary to set up where the scribble interactable element will be.
    int leftIndex = 12;
    int topIndex = 13;
    _inputHider.frame =
        CGRectMake([dictionary[@"transform"][leftIndex] intValue],
                   [dictionary[@"transform"][topIndex] intValue], [dictionary[@"width"] intValue],
                   [dictionary[@"height"] intValue]);
    size = CGSizeMake([dictionary[@"width"] intValue], [dictionary[@"height"] intValue]);
    _activeClient.tintColor = [UIColor clearColor];
  }
  [_activeClient setEditableSize:size transform:dictionary[@"transform"]];
}

- (void)updateMarkedRect:(NSDictionary*)dictionary {
  NSAssert(dictionary[@"x"] != nil && dictionary[@"y"] != nil && dictionary[@"width"] != nil &&
               dictionary[@"height"] != nil,
           @"Expected a dictionary representing a CGRect, got %@", dictionary);
  CGRect rect = CGRectMake([dictionary[@"x"] doubleValue], [dictionary[@"y"] doubleValue],
                           [dictionary[@"width"] doubleValue], [dictionary[@"height"] doubleValue]);
  _activeClient.markedRect = rect.size.width < 0 && rect.size.height < 0 ? kInvalidFirstRect : rect;
}

- (void)setSelectionRects:(NSArray*)rects {
  NSMutableArray<FlutterTextSelectionRect*>* rectsAsRect =
      [[NSMutableArray alloc] initWithCapacity:[rects count]];
  for (NSUInteger i = 0; i < [rects count]; i++) {
    NSArray<NSNumber*>* rect = rects[i];
    [rectsAsRect
        addObject:[FlutterTextSelectionRect
                      selectionRectWithRect:CGRectMake([rect[0] floatValue], [rect[1] floatValue],
                                                       [rect[2] floatValue], [rect[3] floatValue])
                                   position:[rect[4] unsignedIntegerValue]]];
  }
  [_activeClient setSelectionRects:rectsAsRect];
}

- (void)startLiveTextInput {
  if (@available(iOS 15.0, *)) {
    if (_activeClient == nil || !_activeClient.isFirstResponder) {
      return;
    }
    [_activeClient captureTextFromCamera:nil];
  }
}

- (void)showTextInput {
  [_activeClient setViewResponder:_viewResponder];
  [self addToInputParentViewIfNeeded:_activeClient];
  // Adds a delay to prevent the text view from receiving accessibility
  // focus in case it is activated during semantics updates.
  //
  // One common case is when the app navigates to a page with an auto
  // focused text field. The text field will activate the FlutterTextInputView
  // with a semantics update sent to the engine. The voiceover will focus
  // the newly attached active view while performing accessibility update.
  // This results in accessibility focus stuck at the FlutterTextInputView.
  if (!_enableFlutterTextInputViewAccessibilityTimer) {
    _enableFlutterTextInputViewAccessibilityTimer =
        [NSTimer scheduledTimerWithTimeInterval:kUITextInputAccessibilityEnablingDelaySeconds
                                         target:[FlutterTimerProxy proxyWithTarget:self]
                                       selector:@selector(enableActiveViewAccessibility)
                                       userInfo:nil
                                        repeats:NO];
  }
  [_activeClient becomeFirstResponder];
}

- (void)enableActiveViewAccessibility {
  if (_activeClient.isFirstResponder) {
    _activeClient.accessibilityEnabled = YES;
  }
  [self removeEnableFlutterTextInputViewAccessibilityTimer];
}

- (void)hideTextInput {
  [self removeEnableFlutterTextInputViewAccessibilityTimer];
  _activeClient.accessibilityEnabled = NO;
  [_activeClient resignFirstResponder];
  [_activeClient removeFromSuperview];
  [_inputHider removeFromSuperview];
}

- (void)triggerAutofillSave:(BOOL)saveEntries {
  [_activeClient resignFirstResponder];

  if (saveEntries) {
    // Make all the input fields in the autofill context visible,
    // then remove them to trigger autofill save.
    [self cleanUpViewHierarchy:YES clearText:YES delayRemoval:NO];
    [_autofillContext removeAllObjects];
    [self changeInputViewsAutofillVisibility:YES];
  } else {
    [_autofillContext removeAllObjects];
  }

  [self cleanUpViewHierarchy:YES clearText:!saveEntries delayRemoval:NO];
  [self addToInputParentViewIfNeeded:_activeClient];
}

- (void)setPlatformViewTextInputClient {
  // No need to track the platformViewID (unlike in Android). When a platform view
  // becomes the first responder, simply hide this dummy text input view (`_activeView`)
  // for the previously focused widget.
  [self removeEnableFlutterTextInputViewAccessibilityTimer];
  _activeClient.accessibilityEnabled = NO;
  [_activeClient removeFromSuperview];
  [_inputHider removeFromSuperview];
}

- (void)setTextInputClient:(int)client withConfiguration:(NSDictionary*)configuration {
  [self resetAllClientIds];
  // Hide all input views from autofill, only make those in the new configuration visible
  // to autofill.
  [self changeInputViewsAutofillVisibility:NO];

  // Update the current active view.
  switch (AutofillTypeOf(configuration)) {
    case kFlutterAutofillTypeNone:
      self.activeClient = [self createInputViewWith:configuration];
      break;
    case kFlutterAutofillTypeRegular:
      // If the group does not involve password autofill, only install the
      // input view that's being focused.
      self.activeClient = [self updateAndShowAutofillViews:nil
                                              focusedField:configuration
                                         isPasswordRelated:NO];
      break;
    case kFlutterAutofillTypePassword:
      self.activeClient = [self updateAndShowAutofillViews:configuration[kAssociatedAutofillFields]
                                              focusedField:configuration
                                         isPasswordRelated:YES];
      break;
  }
  _activeClient.clientID = client;
  [_activeClient reloadInputViews];

  // Clean up views that no longer need to be in the view hierarchy, according to
  // the current autofill context. The "garbage" input views are already made
  // invisible to autofill and they can't `becomeFirstResponder`, we only remove
  // them to free up resources and reduce the number of input views in the view
  // hierarchy.
  //
  // The removeFromSuperview call is scheduled on the runloop and delayed by 0.1s
  // so we don't remove the text fields immediately (which seems to make the keyboard flicker).
  // See: https://github.com/flutter/flutter/issues/64628.
  [self cleanUpViewHierarchy:NO clearText:YES delayRemoval:YES];
}

// Creates and shows an input field that is not password related and has no autofill
// info. This method returns a new FlutterTextInputClient instance when called, since
// UIKit uses the identity of `UITextInput` instances (or the identity of the input
// views) to decide whether the IME's internal states should be reset. See:
// https://github.com/flutter/flutter/issues/79031 .
- (UIView<FlutterTextInputClient>*)createInputViewWith:(NSDictionary*)configuration {
  NSString* autofillID = AutofillIDFromDictionary(configuration);
  if (autofillID) {
    [_autofillContext removeObjectForKey:autofillID];
  }
  UIView<FlutterTextInputClient>* newView = [[RegularInputClient alloc] initWithOwner:self];
  ConfigureInputClientWithDictionary(newView, configuration);
  [self addToInputParentViewIfNeeded:newView];

  for (NSDictionary* field in configuration[kAssociatedAutofillFields]) {
    NSString* autofillID = AutofillIDFromDictionary(field);
    if (autofillID && AutofillTypeOf(field) == kFlutterAutofillTypeNone) {
      [_autofillContext removeObjectForKey:autofillID];
    }
  }
  return newView;
}

- (UIView<FlutterTextInputClient>*)updateAndShowAutofillViews:(NSArray*)fields
                                                 focusedField:(NSDictionary*)focusedField
                                            isPasswordRelated:(BOOL)isPassword {
  UIView<FlutterTextInputClient, FlutterTextAutofillClient>* focused = nil;
  NSString* focusedId = AutofillIDFromDictionary(focusedField);
  NSAssert(focusedId, @"autofillID must not be null for the focused field: %@", focusedField);

  if (!fields) {
    // DO NOT push the current autofillable input fields to the context even
    // if it's password-related, because it is not in an autofill group.
    focused = [self getOrCreateAutofillableView:focusedField isPasswordAutofill:isPassword];
    [_autofillContext removeObjectForKey:focusedId];
  }

  for (NSDictionary* field in fields) {
    NSString* autofillID = AutofillIDFromDictionary(field);
    NSAssert(autofillID, @"autofillID must not be null for field: %@", field);

    BOOL hasHints = AutofillTypeOf(field) != kFlutterAutofillTypeNone;
    BOOL isFocused = [focusedId isEqualToString:autofillID];

    if (isFocused) {
      focused = [self getOrCreateAutofillableView:field isPasswordAutofill:isPassword];
    }

    if (hasHints) {
      // Push the current input field to the context if it has hints.
      _autofillContext[autofillID] = isFocused ? focused
                                               : [self getOrCreateAutofillableView:field
                                                                isPasswordAutofill:isPassword];
    } else {
      // Mark for deletion.
      [_autofillContext removeObjectForKey:autofillID];
    }
  }

  NSAssert(focused, @"The current focused input view must not be nil.");
  return focused;
}

// Returns a new non-reusable input view (and put it into the view hierarchy), or get the
// view from the current autofill context, if an input view with the same autofill id
// already exists in the context.
// This is generally used for input fields that are autofillable (UIKit tracks these veiws
// for autofill purposes so they should not be reused for a different type of views).
- (UIView<FlutterTextInputClient, FlutterTextAutofillClient>*)
    getOrCreateAutofillableView:(NSDictionary*)field
             isPasswordAutofill:(BOOL)needsPasswordAutofill {
  NSString* autofillID = AutofillIDFromDictionary(field);
  UIView<FlutterTextInputClient, FlutterTextAutofillClient>* inputView =
      _autofillContext[autofillID];
  if (!inputView) {
    inputView = needsPasswordAutofill ? [SecureInputClient alloc] : [RegularInputClient alloc];
    inputView = [inputView initWithOwner:self];
    [self addToInputParentViewIfNeeded:inputView];
  }

  ConfigureInputClientWithDictionary(inputView, field);
  return inputView;
}

// The UIView to add FlutterTextInputViews to.
- (UIView*)hostView {
  UIView* host = _viewController.view;
  NSAssert(host != nullptr,
           @"The application must have a host view since the keyboard client "
           @"must be part of the responder chain to function. The host view controller is %@",
           _viewController);
  return host;
}

// The UIView to add FlutterTextInputClients to.
- (NSArray<UIView*>*)textInputViews {
  return _inputHider.subviews;
}

// Removes every installed input field, unless it's in the current autofill context.
//
// The active view will be removed from its superview, if includeActiveView is YES.
// When clearText is YES, the text on the input fields will be set to empty before
// they are removed from the view hierarchy, to avoid triggering autofill save.
// If delayRemoval is true, removeFromSuperview will be scheduled on the runloop and
// will be delayed by 0.1s so we don't remove the text fields immediately (which seems
// to make the keyboard flicker).
// See: https://github.com/flutter/flutter/issues/64628.

- (void)cleanUpViewHierarchy:(BOOL)includeActiveView
                   clearText:(BOOL)clearText
                delayRemoval:(BOOL)delayRemoval {
  for (UIView* view in self.textInputViews) {
    if ([view conformsToProtocol:@protocol(FlutterTextInputClient)] &&
        [view conformsToProtocol:@protocol(FlutterTextAutofillClient)] &&
        (includeActiveView || view != _activeClient)) {
      UIView<FlutterTextInputClient, FlutterTextAutofillClient>* inputView =
          (UIView<FlutterTextInputClient, FlutterTextAutofillClient>*)view;
      if (_autofillContext[inputView.autofillID] != view) {
        if (clearText) {
          inputView.selectedTextRange =
              [inputView textRangeFromPosition:inputView.beginningOfDocument
                                    toPosition:inputView.endOfDocument];
          [inputView deleteBackward];
        }
        if (delayRemoval) {
          [inputView performSelector:@selector(removeFromSuperview) withObject:nil afterDelay:0.1];
        } else {
          [inputView removeFromSuperview];
        }
      }
    }
  }
}

// Changes the visibility of every FlutterTextInputView currently in the
// view hierarchy.
- (void)changeInputViewsAutofillVisibility:(BOOL)newVisibility {
  for (UIView* view in self.textInputViews) {
    if ([view conformsToProtocol:@protocol(FlutterTextAutofillClient)]) {
      UIView<FlutterTextAutofillClient>* inputView = (UIView<FlutterTextAutofillClient>*)view;
      inputView.isVisibleToAutofill = newVisibility;
    }
  }
}

// Resets the client id of every FlutterTextInputView in the view hierarchy
// to 0.
// Called before establishing a new text input connection.
// For views in the current autofill context, they need to
// stay in the view hierachy but should not be allowed to
// send messages (other than autofill related ones) to the
// framework.
- (void)resetAllClientIds {
  for (UIView* view in self.textInputViews) {
    if ([view conformsToProtocol:@protocol(FlutterTextInputClient)]) {
      UIView<FlutterTextInputClient>* inputView = (UIView<FlutterTextInputClient>*)view;
      inputView.clientID = 0;
    }
  }
}

- (void)addToInputParentViewIfNeeded:(UIView<FlutterTextInputClient>*)inputView {
  if (![inputView isDescendantOfView:_inputHider]) {
    [_inputHider addSubview:inputView];
  }
  UIView* parentView = self.hostView;
  if (_inputHider.superview != parentView) {
    [parentView addSubview:_inputHider];
  }
}

- (void)setTextInputEditingState:(NSDictionary*)state {
  _activeClient.textInputState = state;
}

- (void)clearTextInputClient {
  _activeClient.clientID = 0;
  _activeClient.frame = CGRectZero;
}

#pragma mark UIIndirectScribbleInteractionDelegate

- (BOOL)indirectScribbleInteraction:(UIIndirectScribbleInteraction*)interaction
                   isElementFocused:(UIScribbleElementIdentifier)elementIdentifier
    API_AVAILABLE(ios(14.0)) {
  return _activeClient.scribbleFocusStatus == FlutterScribbleFocusStatusFocused;
}

- (void)indirectScribbleInteraction:(UIIndirectScribbleInteraction*)interaction
               focusElementIfNeeded:(UIScribbleElementIdentifier)elementIdentifier
                     referencePoint:(CGPoint)focusReferencePoint
                         completion:(void (^)(UIResponder<UITextInput>* focusedInput))completion
    API_AVAILABLE(ios(14.0)) {
  _activeClient.scribbleFocusStatus = FlutterScribbleFocusStatusFocusing;
  [_indirectScribbleDelegate flutterTextInputPlugin:self
                                       focusElement:elementIdentifier
                                            atPoint:focusReferencePoint
                                             result:^(id _Nullable result) {
                                               _activeClient.scribbleFocusStatus =
                                                   FlutterScribbleFocusStatusFocused;
                                               completion(_activeClient);
                                             }];
}

- (BOOL)indirectScribbleInteraction:(UIIndirectScribbleInteraction*)interaction
         shouldDelayFocusForElement:(UIScribbleElementIdentifier)elementIdentifier
    API_AVAILABLE(ios(14.0)) {
  return NO;
}

- (void)indirectScribbleInteraction:(UIIndirectScribbleInteraction*)interaction
          willBeginWritingInElement:(UIScribbleElementIdentifier)elementIdentifier
    API_AVAILABLE(ios(14.0)) {
}

- (void)indirectScribbleInteraction:(UIIndirectScribbleInteraction*)interaction
          didFinishWritingInElement:(UIScribbleElementIdentifier)elementIdentifier
    API_AVAILABLE(ios(14.0)) {
}

- (CGRect)indirectScribbleInteraction:(UIIndirectScribbleInteraction*)interaction
                      frameForElement:(UIScribbleElementIdentifier)elementIdentifier
    API_AVAILABLE(ios(14.0)) {
  NSValue* elementValue = [_scribbleElements objectForKey:elementIdentifier];
  if (elementValue == nil) {
    return CGRectZero;
  }
  return [elementValue CGRectValue];
}

- (void)indirectScribbleInteraction:(UIIndirectScribbleInteraction*)interaction
              requestElementsInRect:(CGRect)rect
                         completion:
                             (void (^)(NSArray<UIScribbleElementIdentifier>* elements))completion
    API_AVAILABLE(ios(14.0)) {
  [_indirectScribbleDelegate
      flutterTextInputPlugin:self
       requestElementsInRect:rect
                      result:^(id _Nullable result) {
                        NSMutableArray<UIScribbleElementIdentifier>* elements =
                            [[NSMutableArray alloc] init];
                        if ([result isKindOfClass:[NSArray class]]) {
                          for (NSArray* elementArray in result) {
                            [elements addObject:elementArray[0]];
                            [_scribbleElements
                                setObject:[NSValue
                                              valueWithCGRect:CGRectMake(
                                                                  [elementArray[1] floatValue],
                                                                  [elementArray[2] floatValue],
                                                                  [elementArray[3] floatValue],
                                                                  [elementArray[4] floatValue])]
                                   forKey:elementArray[0]];
                          }
                        }
                        completion(elements);
                      }];
}

#pragma mark - Methods related to Scribble support

// Checks whether Scribble features are possibly available – meaning this is an iPad running iOS
// 14 or higher.
+ (BOOL)isScribbleAvailable {
  if (@available(iOS 14.0, *)) {
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
      return YES;
    }
  }
  return NO;
}

- (void)setupIndirectScribbleInteraction:(id<FlutterViewResponder>)viewResponder {
  if (_viewResponder != viewResponder) {
    if (@available(iOS 14.0, *)) {
      UIView* parentView = viewResponder.view;
      if (parentView != nil) {
        UIIndirectScribbleInteraction* scribbleInteraction = [[UIIndirectScribbleInteraction alloc]
            initWithDelegate:(id<UIIndirectScribbleInteractionDelegate>)self];
        [parentView addInteraction:scribbleInteraction];
      }
    }
  }
  _viewResponder = viewResponder;
}

- (void)resetViewResponder {
  _viewResponder = nil;
}

#pragma mark -
#pragma mark FlutterKeySecondaryResponder

/**
 * Handles key down events received from the view controller, responding YES if
 * the event was handled.
 */
- (BOOL)handlePress:(nonnull FlutterUIPressProxy*)press API_AVAILABLE(ios(13.4)) {
  return NO;
}
@end
