// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterViewController.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"

#include <Carbon/Carbon.h>
#import <objc/message.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterChannels.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyPrimaryResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyboardManager.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMouseState.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterRenderer.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterTextInputSemanticsObject.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"
#import "flutter/shell/platform/embedder/embedder.h"

namespace {
using flutter::KeyboardLayoutNotifier;
using flutter::LayoutClue;

/**
 * Returns the current Unicode layout data (kTISPropertyUnicodeKeyLayoutData).
 *
 * To use the returned data, convert it to CFDataRef first, finds its bytes
 * with CFDataGetBytePtr, then reinterpret it into const UCKeyboardLayout*.
 * It's returned in NSData* to enable auto reference count.
 */
NSData* currentKeyboardLayoutData() {
  TISInputSourceRef source = TISCopyCurrentKeyboardInputSource();
  CFTypeRef layout_data = TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData);
  if (layout_data == nil) {
    CFRelease(source);
    // TISGetInputSourceProperty returns null with Japanese keyboard layout.
    // Using TISCopyCurrentKeyboardLayoutInputSource to fix NULL return.
    // https://github.com/microsoft/node-native-keymap/blob/5f0699ded00179410a14c0e1b0e089fe4df8e130/src/keyboard_mac.mm#L91
    source = TISCopyCurrentKeyboardLayoutInputSource();
    layout_data = TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData);
  }
  return (__bridge_transfer NSData*)CFRetain(layout_data);
}

}  // namespace

#pragma mark - Private interface declaration.

/**
 * FlutterViewWrapper is a convenience class that wraps a FlutterView and provides
 * a mechanism to attach AppKit views such as FlutterTextField without affecting
 * the accessibility subtree of the wrapped FlutterView itself.
 *
 * The FlutterViewController uses this class to create its content view. When
 * any of the accessibility services (e.g. VoiceOver) is turned on, the accessibility
 * bridge creates FlutterTextFields that interact with the service. The bridge has to
 * attach the FlutterTextField somewhere in the view hierarchy in order for the
 * FlutterTextField to interact correctly with VoiceOver. Those FlutterTextFields
 * will be attached to this view so that they won't affect the accessibility subtree
 * of FlutterView.
 */
@interface FlutterViewWrapper : NSView

- (void)setBackgroundColor:(NSColor*)color;

- (BOOL)performKeyEquivalent:(NSEvent*)event;

@end

/**
 * Private interface declaration for FlutterViewController.
 */
@interface FlutterViewController () <FlutterViewReshapeListener>

/**
 * The tracking area used to generate hover events, if enabled.
 */
@property(nonatomic) NSTrackingArea* trackingArea;

@property(nonatomic) FlutterMouseState* mouseState;

/**
 * Event monitor for keyUp events.
 */
@property(nonatomic) id keyUpMonitor;

/**
 * Pointer to a keyboard manager, a hub that manages how key events are
 * dispatched to various Flutter key responders, and whether the event is
 * propagated to the next NSResponder.
 */
@property(nonatomic, readonly, nonnull) FlutterKeyboardManager* keyboardManager;

@property(nonatomic) KeyboardLayoutNotifier keyboardLayoutNotifier;

@property(nonatomic) NSData* keyboardLayoutData;

/**
 * Starts running |engine|, including any initial setup.
 */
- (BOOL)launchEngine;

/**
 * Updates |trackingArea| for the current tracking settings, creating it with
 * the correct mode if tracking is enabled, or removing it if not.
 */
- (void)configureTrackingArea;

/**
 * Creates and registers keyboard related components.
 */
- (void)initializeKeyboard;

/**
 * Called when the active keyboard input source changes.
 *
 * Input sources may be simple keyboard layouts, or more complex input methods involving an IME,
 * such as Chinese, Japanese, and Korean.
 */
- (void)onKeyboardLayoutChanged;

@end

#pragma mark - NSEvent (KeyEquivalentMarker) protocol

@interface NSEvent (KeyEquivalentMarker)

// Internally marks that the event was received through performKeyEquivalent:.
// When text editing is active, keyboard events that have modifier keys pressed
// are received through performKeyEquivalent: instead of keyDown:. If such event
// is passed to TextInputContext but doesn't result in a text editing action it
// needs to be forwarded by FlutterKeyboardManager to the next responder.
- (void)markAsKeyEquivalent;

// Returns YES if the event is marked as a key equivalent.
- (BOOL)isKeyEquivalent;

@end

@implementation NSEvent (KeyEquivalentMarker)

// This field doesn't need a value because only its address is used as a unique identifier.
static char markerKey;

- (void)markAsKeyEquivalent {
  objc_setAssociatedObject(self, &markerKey, @true, OBJC_ASSOCIATION_RETAIN);
}

- (BOOL)isKeyEquivalent {
  return [objc_getAssociatedObject(self, &markerKey) boolValue] == YES;
}

@end

#pragma mark - Private dependant functions

namespace {
void OnKeyboardLayoutChanged(CFNotificationCenterRef center,
                             void* observer,
                             CFStringRef name,
                             const void* object,
                             CFDictionaryRef userInfo) {
  FlutterViewController* controller = (__bridge FlutterViewController*)observer;
  if (controller != nil) {
    [controller onKeyboardLayoutChanged];
  }
}
}  // namespace

#pragma mark - FlutterViewWrapper implementation.

@implementation FlutterViewWrapper {
  FlutterView* _flutterView;
  FlutterViewController* _controller;
  FlutterMouseState* _mouseState;
}

- (instancetype)initWithFlutterView:(FlutterView*)view
                         controller:(FlutterViewController*)controller {
  self = [super initWithFrame:NSZeroRect];
  if (self) {
    _flutterView = view;
    _controller = controller;

    view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    [self addSubview:view];
  }
  return self;
}

- (void)setBackgroundColor:(NSColor*)color {
  [_flutterView setBackgroundColor:color];
}

- (BOOL)performKeyEquivalent:(NSEvent*)event {
  if ([_controller isDispatchingKeyEvent:event]) {
    // When NSWindow is nextResponder, keyboard manager will send to it
    // unhandled events (through [NSWindow keyDown:]). If event has both
    // control and cmd modifiers set (i.e. cmd+control+space - emoji picker)
    // NSWindow will then send this event as performKeyEquivalent: to first
    // responder, which might be FlutterTextInputPlugin. If that's the case, the
    // plugin must not handle the event, otherwise the emoji picker would not
    // work (due to first responder returning YES from performKeyEquivalent:)
    // and there would be an infinite loop, because FlutterViewController will
    // send the event back to [keyboardManager handleEvent:].
    return NO;
  }
  [event markAsKeyEquivalent];
  [_flutterView keyDown:event];
  return YES;
}

- (NSArray*)accessibilityChildren {
  return @[ _flutterView ];
}

- (void)mouseDown:(NSEvent*)event {
  // Work around an AppKit bug where mouseDown/mouseUp are not called on the view controller if the
  // view is the content view of an NSPopover AND macOS's Reduced Transparency accessibility setting
  // is enabled.
  //
  // This simply calls mouseDown on the next responder in the responder chain as the default
  // implementation on NSResponder is documented to do.
  //
  // See: https://github.com/flutter/flutter/issues/115015
  // See: http://www.openradar.me/FB12050037
  // See: https://developer.apple.com/documentation/appkit/nsresponder/1524634-mousedown
  [self.nextResponder mouseDown:event];
}

- (void)mouseUp:(NSEvent*)event {
  // Work around an AppKit bug where mouseDown/mouseUp are not called on the view controller if the
  // view is the content view of an NSPopover AND macOS's Reduced Transparency accessibility setting
  // is enabled.
  //
  // This simply calls mouseUp on the next responder in the responder chain as the default
  // implementation on NSResponder is documented to do.
  //
  // See: https://github.com/flutter/flutter/issues/115015
  // See: http://www.openradar.me/FB12050037
  // See: https://developer.apple.com/documentation/appkit/nsresponder/1535349-mouseup
  [self.nextResponder mouseUp:event];
}

@end

#pragma mark - FlutterViewController implementation.

@implementation FlutterViewController {
  // The project to run in this controller's engine.
  FlutterDartProject* _project;

  std::shared_ptr<flutter::AccessibilityBridgeMac> _bridge;

  FlutterViewId _id;
}

@synthesize viewId = _viewId;
@dynamic accessibilityBridge;

/**
 * Performs initialization that's common between the different init paths.
 */
static void CommonInit(FlutterViewController* controller, FlutterEngine* engine) {
  if (!engine) {
    engine = [[FlutterEngine alloc] initWithName:@"io.flutter"
                                         project:controller->_project
                          allowHeadlessExecution:NO];
  }

  NSCAssert(controller.engine == nil,
            @"The FlutterViewController is unexpectedly attached to "
            @"engine %@ before initialization.",
            controller.engine);
  [engine addViewController:controller];
  NSCAssert(controller.engine != nil,
            @"The FlutterViewController unexpectedly stays unattached after initialization. "
            @"In unit tests, this is likely because either the FlutterViewController or "
            @"the FlutterEngine is mocked. Please subclass these classes instead.",
            controller.engine, controller.viewId);
  controller->_mouseState =
      [[FlutterMouseState alloc] initWithCallback:^(const FlutterPointerEvent& event) {
        [engine sendPointerEvent:event];
      }];
  controller->_mouseTrackingMode = FlutterMouseTrackingModeInKeyWindow;
  controller->_textInputPlugin = [[FlutterTextInputPlugin alloc] initWithViewController:controller];
  [controller initializeKeyboard];
  [controller notifySemanticsEnabledChanged];
  // macOS fires this message when changing IMEs.
  CFNotificationCenterRef cfCenter = CFNotificationCenterGetDistributedCenter();
  __weak FlutterViewController* weakSelf = controller;
  CFNotificationCenterAddObserver(cfCenter, (__bridge void*)weakSelf, OnKeyboardLayoutChanged,
                                  kTISNotifySelectedKeyboardInputSourceChanged, NULL,
                                  CFNotificationSuspensionBehaviorDeliverImmediately);
}

- (instancetype)initWithCoder:(NSCoder*)coder {
  self = [super initWithCoder:coder];
  NSAssert(self, @"Super init cannot be nil");

  CommonInit(self, nil);
  return self;
}

- (instancetype)initWithNibName:(NSString*)nibNameOrNil bundle:(NSBundle*)nibBundleOrNil {
  self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
  NSAssert(self, @"Super init cannot be nil");

  CommonInit(self, nil);
  return self;
}

- (instancetype)initWithProject:(nullable FlutterDartProject*)project {
  self = [super initWithNibName:nil bundle:nil];
  NSAssert(self, @"Super init cannot be nil");

  _project = project;
  CommonInit(self, nil);
  return self;
}

- (instancetype)initWithEngine:(nonnull FlutterEngine*)engine
                       nibName:(nullable NSString*)nibName
                        bundle:(nullable NSBundle*)nibBundle {
  NSAssert(engine != nil, @"Engine is required");

  self = [super initWithNibName:nibName bundle:nibBundle];
  if (self) {
    CommonInit(self, engine);
  }

  return self;
}

- (BOOL)isDispatchingKeyEvent:(NSEvent*)event {
  return [_keyboardManager isDispatchingKeyEvent:event];
}

- (void)loadView {
  FlutterView* flutterView;
  id<MTLDevice> device = _engine.renderer.device;
  id<MTLCommandQueue> commandQueue = _engine.renderer.commandQueue;
  if (!device || !commandQueue) {
    NSLog(@"Unable to create FlutterView; no MTLDevice or MTLCommandQueue available.");
    return;
  }
  flutterView = [self createFlutterViewWithMTLDevice:device commandQueue:commandQueue];
  if (_backgroundColor != nil) {
    [flutterView setBackgroundColor:_backgroundColor];
  }
  FlutterViewWrapper* wrapperView = [[FlutterViewWrapper alloc] initWithFlutterView:flutterView
                                                                         controller:self];
  self.view = wrapperView;
  _flutterView = flutterView;
}

- (void)viewDidLoad {
  [self configureTrackingArea];
  [self.view setAllowedTouchTypes:NSTouchTypeMaskIndirect];
  [self.view setWantsRestingTouches:YES];
}

- (void)viewWillAppear {
  [super viewWillAppear];
  if (!_engine.running) {
    [self launchEngine];
  }
  [self listenForMetaModifiedKeyUpEvents];
}

- (void)viewWillDisappear {
  // Per Apple's documentation, it is discouraged to call removeMonitor: in dealloc, and it's
  // recommended to be called earlier in the lifecycle.
  [NSEvent removeMonitor:_keyUpMonitor];
  _keyUpMonitor = nil;
}

- (void)dealloc {
  if ([self attached]) {
    [_engine removeViewController:self];
  }
  CFNotificationCenterRef cfCenter = CFNotificationCenterGetDistributedCenter();
  CFNotificationCenterRemoveEveryObserver(cfCenter, (__bridge void*)self);
}

#pragma mark - Public methods

- (void)setMouseTrackingMode:(FlutterMouseTrackingMode)mode {
  if (_mouseTrackingMode == mode) {
    return;
  }
  _mouseTrackingMode = mode;
  [self configureTrackingArea];
}

- (void)setBackgroundColor:(NSColor*)color {
  _backgroundColor = color;
  [_flutterView setBackgroundColor:_backgroundColor];
}

- (FlutterViewId)viewId {
  NSAssert([self attached], @"This view controller is not attched.");
  return _viewId;
}

- (void)onPreEngineRestart {
  [self initializeKeyboard];
}

- (void)notifySemanticsEnabledChanged {
  BOOL mySemanticsEnabled = !!_bridge;
  BOOL newSemanticsEnabled = _engine.semanticsEnabled;
  if (newSemanticsEnabled == mySemanticsEnabled) {
    return;
  }
  if (newSemanticsEnabled) {
    _bridge = [self createAccessibilityBridgeWithEngine:_engine];
  } else {
    // Remove the accessibility children from flutter view before resetting the bridge.
    _flutterView.accessibilityChildren = nil;
    _bridge.reset();
  }
  NSAssert(newSemanticsEnabled == !!_bridge, @"Failed to update semantics for the view.");
}

- (std::weak_ptr<flutter::AccessibilityBridgeMac>)accessibilityBridge {
  return _bridge;
}

- (void)attachToEngine:(nonnull FlutterEngine*)engine withId:(FlutterViewId)viewId {
  NSAssert(_engine == nil, @"Already attached to an engine %@.", _engine);
  _engine = engine;
  _viewId = viewId;
}

- (void)detachFromEngine {
  NSAssert(_engine != nil, @"Not attached to any engine.");
  _engine = nil;
}

- (BOOL)attached {
  return _engine != nil;
}

- (void)updateSemantics:(const FlutterSemanticsUpdate2*)update {
  NSAssert(_engine.semanticsEnabled, @"Semantics must be enabled.");
  if (!_engine.semanticsEnabled) {
    return;
  }
  for (size_t i = 0; i < update->node_count; i++) {
    const FlutterSemanticsNode2* node = update->nodes[i];
    _bridge->AddFlutterSemanticsNodeUpdate(*node);
  }

  for (size_t i = 0; i < update->custom_action_count; i++) {
    const FlutterSemanticsCustomAction2* action = update->custom_actions[i];
    _bridge->AddFlutterSemanticsCustomActionUpdate(*action);
  }

  _bridge->CommitUpdates();

  // Accessibility tree can only be used when the view is loaded.
  if (!self.viewLoaded) {
    return;
  }
  // Attaches the accessibility root to the flutter view.
  auto root = _bridge->GetFlutterPlatformNodeDelegateFromID(0).lock();
  if (root) {
    if ([self.flutterView.accessibilityChildren count] == 0) {
      NSAccessibilityElement* native_root = root->GetNativeViewAccessible();
      self.flutterView.accessibilityChildren = @[ native_root ];
    }
  } else {
    self.flutterView.accessibilityChildren = nil;
  }
}

#pragma mark - Private methods

- (BOOL)launchEngine {
  if (![_engine runWithEntrypoint:nil]) {
    return NO;
  }
  return YES;
}

// macOS does not call keyUp: on a key while the command key is pressed. This results in a loss
// of a key event once the modified key is released. This method registers the
// ViewController as a listener for a keyUp event before it's handled by NSApplication, and should
// NOT modify the event to avoid any unexpected behavior.
- (void)listenForMetaModifiedKeyUpEvents {
  if (_keyUpMonitor != nil) {
    // It is possible for [NSViewController viewWillAppear] to be invoked multiple times
    // in a row. https://github.com/flutter/flutter/issues/105963
    return;
  }
  FlutterViewController* __weak weakSelf = self;
  _keyUpMonitor = [NSEvent
      addLocalMonitorForEventsMatchingMask:NSEventMaskKeyUp
                                   handler:^NSEvent*(NSEvent* event) {
                                     // Intercept keyUp only for events triggered on the current
                                     // view or textInputPlugin.
                                     NSResponder* firstResponder = [[event window] firstResponder];
                                     if (weakSelf.viewLoaded && weakSelf.flutterView &&
                                         (firstResponder == weakSelf.flutterView ||
                                          firstResponder == weakSelf.textInputPlugin) &&
                                         ([event modifierFlags] & NSEventModifierFlagCommand) &&
                                         ([event type] == NSEventTypeKeyUp)) {
                                       [weakSelf keyUp:event];
                                     }
                                     return event;
                                   }];
}

- (void)configureTrackingArea {
  if (!self.viewLoaded) {
    // The viewDidLoad will call configureTrackingArea again when
    // the view is actually loaded.
    return;
  }
  if (_mouseTrackingMode != FlutterMouseTrackingModeNone && self.flutterView) {
    NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved |
                                    NSTrackingInVisibleRect | NSTrackingEnabledDuringMouseDrag;
    switch (_mouseTrackingMode) {
      case FlutterMouseTrackingModeInKeyWindow:
        options |= NSTrackingActiveInKeyWindow;
        break;
      case FlutterMouseTrackingModeInActiveApp:
        options |= NSTrackingActiveInActiveApp;
        break;
      case FlutterMouseTrackingModeAlways:
        options |= NSTrackingActiveAlways;
        break;
      default:
        NSLog(@"Error: Unrecognized mouse tracking mode: %ld", _mouseTrackingMode);
        return;
    }
    _trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                 options:options
                                                   owner:self
                                                userInfo:nil];
    [self.flutterView addTrackingArea:_trackingArea];
  } else if (_trackingArea) {
    [self.flutterView removeTrackingArea:_trackingArea];
    _trackingArea = nil;
  }
}

- (void)initializeKeyboard {
  // TODO(goderbauer): Seperate keyboard/textinput stuff into ViewController specific and Engine
  // global parts. Move the global parts to FlutterEngine.
  _keyboardManager = [[FlutterKeyboardManager alloc] initWithViewDelegate:self];
}

- (void)onAccessibilityStatusChanged:(BOOL)enabled {
  if (!enabled && self.viewLoaded && [_textInputPlugin isFirstResponder]) {
    // Normally TextInputPlugin, when editing, is child of FlutterViewWrapper.
    // When accessiblity is enabled the TextInputPlugin gets added as an indirect
    // child to FlutterTextField. When disabling the plugin needs to be reparented
    // back.
    [self.view addSubview:_textInputPlugin];
  }
}

- (std::shared_ptr<flutter::AccessibilityBridgeMac>)createAccessibilityBridgeWithEngine:
    (nonnull FlutterEngine*)engine {
  return std::make_shared<flutter::AccessibilityBridgeMac>(engine, self);
}

- (nonnull FlutterView*)createFlutterViewWithMTLDevice:(id<MTLDevice>)device
                                          commandQueue:(id<MTLCommandQueue>)commandQueue {
  return [[FlutterView alloc] initWithMTLDevice:device
                                   commandQueue:commandQueue
                                reshapeListener:self];
}

- (void)onKeyboardLayoutChanged {
  _keyboardLayoutData = nil;
  if (_keyboardLayoutNotifier != nil) {
    _keyboardLayoutNotifier();
  }
}

- (NSString*)lookupKeyForAsset:(NSString*)asset {
  return [FlutterDartProject lookupKeyForAsset:asset];
}

- (NSString*)lookupKeyForAsset:(NSString*)asset fromPackage:(NSString*)package {
  return [FlutterDartProject lookupKeyForAsset:asset fromPackage:package];
}

#pragma mark - FlutterViewReshapeListener

/**
 * Responds to view reshape by notifying the engine of the change in dimensions.
 */
- (void)viewDidReshape:(NSView*)view {
  [_engine updateWindowMetricsForViewController:self];
}

#pragma mark - FlutterPluginRegistry

- (id<FlutterPluginRegistrar>)registrarForPlugin:(NSString*)pluginName {
  return [_engine registrarForPlugin:pluginName];
}

#pragma mark - FlutterKeyboardViewDelegate

- (void)sendKeyEvent:(const FlutterKeyEvent&)event
            callback:(nullable FlutterKeyEventCallback)callback
            userData:(nullable void*)userData {
  [_engine sendKeyEvent:event callback:callback userData:userData];
}

- (id<FlutterBinaryMessenger>)getBinaryMessenger {
  return _engine.binaryMessenger;
}

- (BOOL)onTextInputKeyEvent:(nonnull NSEvent*)event {
  return [_textInputPlugin handleKeyEvent:event];
}

- (void)subscribeToKeyboardLayoutChange:(nullable KeyboardLayoutNotifier)callback {
  _keyboardLayoutNotifier = callback;
}

- (LayoutClue)lookUpLayoutForKeyCode:(uint16_t)keyCode shift:(BOOL)shift {
  if (_keyboardLayoutData == nil) {
    _keyboardLayoutData = currentKeyboardLayoutData();
  }
  const UCKeyboardLayout* layout = reinterpret_cast<const UCKeyboardLayout*>(
      CFDataGetBytePtr((__bridge CFDataRef)_keyboardLayoutData));

  UInt32 deadKeyState = 0;
  UniCharCount stringLength = 0;
  UniChar resultChar;

  UInt32 modifierState = ((shift ? shiftKey : 0) >> 8) & 0xFF;
  UInt32 keyboardType = LMGetKbdLast();

  bool isDeadKey = false;
  OSStatus status =
      UCKeyTranslate(layout, keyCode, kUCKeyActionDown, modifierState, keyboardType,
                     kUCKeyTranslateNoDeadKeysBit, &deadKeyState, 1, &stringLength, &resultChar);
  // For dead keys, press the same key again to get the printable representation of the key.
  if (status == noErr && stringLength == 0 && deadKeyState != 0) {
    isDeadKey = true;
    status =
        UCKeyTranslate(layout, keyCode, kUCKeyActionDown, modifierState, keyboardType,
                       kUCKeyTranslateNoDeadKeysBit, &deadKeyState, 1, &stringLength, &resultChar);
  }

  if (status == noErr && stringLength == 1 && !std::iscntrl(resultChar)) {
    return LayoutClue{resultChar, isDeadKey};
  }
  return LayoutClue{0, false};
}

#pragma mark - NSResponder

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (void)keyDown:(NSEvent*)event {
  [_keyboardManager handleEvent:event];
}

- (void)keyUp:(NSEvent*)event {
  [_keyboardManager handleEvent:event];
}

- (void)flagsChanged:(NSEvent*)event {
  [_keyboardManager handleEvent:event];
}

- (void)mouseEntered:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState mouseEntered:event inView:self.flutterView];
}

- (void)mouseExited:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState mouseExited:event inView:self.flutterView];
}

- (void)mouseDown:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState mouseDown:event inView:self.flutterView];
}

- (void)mouseUp:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState mouseUp:event inView:self.flutterView];
}

- (void)mouseDragged:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState mouseDragged:event inView:self.flutterView];
}

- (void)rightMouseDown:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState rightMouseDown:event inView:self.flutterView];
}

- (void)rightMouseUp:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState rightMouseUp:event inView:self.flutterView];
}

- (void)rightMouseDragged:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState rightMouseDragged:event inView:self.flutterView];
}

- (void)otherMouseDown:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState otherMouseDown:event inView:self.flutterView];
}

- (void)otherMouseUp:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState otherMouseUp:event inView:self.flutterView];
}

- (void)otherMouseDragged:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState otherMouseDragged:event inView:self.flutterView];
}

- (void)mouseMoved:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState mouseMoved:event inView:self.flutterView];
}

- (void)scrollWheel:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState scrollWheel:event inView:self.flutterView];
}

- (void)magnifyWithEvent:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState magnifyWithEvent:event inView:self.flutterView];
}

- (void)rotateWithEvent:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState rotateWithEvent:event inView:self.flutterView];
}

- (void)swipeWithEvent:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  // Not needed, it's handled by scrollWheel.
}

- (void)touchesBeganWithEvent:(NSEvent*)event {
  [self syncKeyboardModifiers:event];
  [_mouseState touchesBeganWithEvent:event inView:self.flutterView];
}

- (void)syncKeyboardModifiers:(NSEvent*)event {
  [_keyboardManager syncModifiersIfNeeded:event.modifierFlags timestamp:event.timestamp];
}

@end
