// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "FlutterMenuPlugin.h"

#include <map>

#import "flutter/shell/platform/common/platform_provided_menu.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterChannels.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"

// Channel constants
static NSString* const kChannelName = @"flutter/menu";
static NSString* const kMenuSetMethod = @"Menu.setMenu";
static NSString* const kMenuItemSelectedCallbackMethod = @"Menu.selectedCallback";
static NSString* const kMenuItemOpenedMethod = @"Menu.opened";
static NSString* const kMenuItemClosedMethod = @"Menu.closed";

// Serialization keys for menu objects
static NSString* const kIdKey = @"id";
static NSString* const kLabelKey = @"label";
static NSString* const kEnabledKey = @"enabled";
static NSString* const kChildrenKey = @"children";
static NSString* const kDividerKey = @"isDivider";
static NSString* const kShortcutEquivalentKey = @"shortcutEquivalent";
static NSString* const kShortcutTriggerKey = @"shortcutTrigger";
static NSString* const kShortcutModifiersKey = @"shortcutModifiers";
static NSString* const kPlatformProvidedMenuKey = @"platformProvidedMenu";

// Key shortcut constants
static const int kFlutterShortcutModifierMeta = 1 << 0;
static const int kFlutterShortcutModifierShift = 1 << 1;
static const int kFlutterShortcutModifierAlt = 1 << 2;
static const int kFlutterShortcutModifierControl = 1 << 3;

static const uint64_t kFlutterKeyIdPlaneMask = 0xff00000000l;
static const uint64_t kFlutterKeyIdUnicodePlane = 0x0000000000l;
static const uint64_t kFlutterKeyIdValueMask = 0x00ffffffffl;

static const NSDictionary* logicalKeyToKeyCode = {};

// What to look for in menu titles to replace with the application name.
static NSString* const kAppName = @"APP_NAME";

// Odd facts about AppKit key equivalents:
//
// 1) ⌃⇧1 and ⇧1 cannot exist in the same app, or the former triggers the latter’s
//    action.
// 2) ⌃⌥⇧1 and ⇧1 cannot exist in the same app, or the former triggers the latter’s
//    action.
// 3) ⌃⌥⇧1 and ⌃⇧1 cannot exist in the same app, or the former triggers the latter’s
//    action.
// 4) ⌃⇧a is equivalent to ⌃A: If a keyEquivalent is a capitalized alphabetical
//    letter and keyEquivalentModifierMask does not include
//    NSEventModifierFlagShift, AppKit will add ⇧ automatically in the UI.

/*
 * Maps the string used by NSMenuItem for the given special key equivalent.
 * Keys are the logical key ids of matching trigger keys.
 */
static NSDictionary<NSNumber*, NSNumber*>* GetMacOsSpecialKeys() {
  return @{
    @0x00100000008 : [NSNumber numberWithInt:NSBackspaceCharacter],
    @0x00100000009 : [NSNumber numberWithInt:NSTabCharacter],
    @0x0010000000a : [NSNumber numberWithInt:NSNewlineCharacter],
    @0x0010000000c : [NSNumber numberWithInt:NSFormFeedCharacter],
    @0x0010000000d : [NSNumber numberWithInt:NSCarriageReturnCharacter],
    @0x0010000007f : [NSNumber numberWithInt:NSDeleteCharacter],
    @0x00100000801 : [NSNumber numberWithInt:NSF1FunctionKey],
    @0x00100000802 : [NSNumber numberWithInt:NSF2FunctionKey],
    @0x00100000803 : [NSNumber numberWithInt:NSF3FunctionKey],
    @0x00100000804 : [NSNumber numberWithInt:NSF4FunctionKey],
    @0x00100000805 : [NSNumber numberWithInt:NSF5FunctionKey],
    @0x00100000806 : [NSNumber numberWithInt:NSF6FunctionKey],
    @0x00100000807 : [NSNumber numberWithInt:NSF7FunctionKey],
    @0x00100000808 : [NSNumber numberWithInt:NSF8FunctionKey],
    @0x00100000809 : [NSNumber numberWithInt:NSF9FunctionKey],
    @0x0010000080a : [NSNumber numberWithInt:NSF10FunctionKey],
    @0x0010000080b : [NSNumber numberWithInt:NSF11FunctionKey],
    @0x0010000080c : [NSNumber numberWithInt:NSF12FunctionKey],
    @0x0010000080d : [NSNumber numberWithInt:NSF13FunctionKey],
    @0x0010000080e : [NSNumber numberWithInt:NSF14FunctionKey],
    @0x0010000080f : [NSNumber numberWithInt:NSF15FunctionKey],
    @0x00100000810 : [NSNumber numberWithInt:NSF16FunctionKey],
    @0x00100000811 : [NSNumber numberWithInt:NSF17FunctionKey],
    @0x00100000812 : [NSNumber numberWithInt:NSF18FunctionKey],
    @0x00100000813 : [NSNumber numberWithInt:NSF19FunctionKey],
    @0x00100000814 : [NSNumber numberWithInt:NSF20FunctionKey],

    // For some reason, there don't appear to be constants for these in ObjC. In
    // Swift, there is a class with static members for these: KeyEquivalent. The
    // values below are taken from that (where they don't already appear above).
    @0x00100000302 : @0xf702,  // ArrowLeft
    @0x00100000303 : @0xf703,  // ArrowRight
    @0x00100000304 : @0xf700,  // ArrowUp
    @0x00100000301 : @0xf701,  // ArrowDown
    @0x00100000306 : @0xf729,  // Home
    @0x00100000305 : @0xf72B,  // End
    @0x00100000308 : @0xf72c,  // PageUp
    @0x00100000307 : @0xf72d,  // PageDown
    @0x0010000001b : @0x001B,  // Escape
  };
}

/*
 * The mapping from the PlatformProvidedMenu enum to the macOS selectors for the provided
 * menus.
 */
static const std::map<int, SEL> GetMacOSProvidedMenus() {
  return {
      {PlatformProvidedMenu::about, @selector(orderFrontStandardAboutPanel:)},
      {PlatformProvidedMenu::quit, @selector(terminate:)},
      // servicesSubmenu is handled specially below: it is assumed to be the first
      // submenu in the preserved platform provided menus, since it doesn't have a
      // definitive selector like the rest.
      {PlatformProvidedMenu::servicesSubmenu, @selector(submenuAction:)},
      {PlatformProvidedMenu::hide, @selector(hide:)},
      {PlatformProvidedMenu::hideOtherApplications, @selector(hideOtherApplications:)},
      {PlatformProvidedMenu::showAllApplications, @selector(unhideAllApplications:)},
      {PlatformProvidedMenu::startSpeaking, @selector(startSpeaking:)},
      {PlatformProvidedMenu::stopSpeaking, @selector(stopSpeaking:)},
      {PlatformProvidedMenu::toggleFullScreen, @selector(toggleFullScreen:)},
      {PlatformProvidedMenu::minimizeWindow, @selector(performMiniaturize:)},
      {PlatformProvidedMenu::zoomWindow, @selector(performZoom:)},
      {PlatformProvidedMenu::arrangeWindowsInFront, @selector(arrangeInFront:)},
  };
}

/*
 * Returns the NSEventModifierFlags of |modifiers|, a value from
 * kShortcutKeyModifiers.
 */
static NSEventModifierFlags KeyEquivalentModifierMaskForModifiers(NSNumber* modifiers) {
  int flutterModifierFlags = modifiers.intValue;
  NSEventModifierFlags flags = 0;
  if (flutterModifierFlags & kFlutterShortcutModifierMeta) {
    flags |= NSEventModifierFlagCommand;
  }
  if (flutterModifierFlags & kFlutterShortcutModifierShift) {
    flags |= NSEventModifierFlagShift;
  }
  if (flutterModifierFlags & kFlutterShortcutModifierAlt) {
    flags |= NSEventModifierFlagOption;
  }
  if (flutterModifierFlags & kFlutterShortcutModifierControl) {
    flags |= NSEventModifierFlagControl;
  }
  // There are also modifier flags for things like the function (Fn) key, but
  // the framework doesn't support those.
  return flags;
}

/*
 * An NSMenuDelegate used to listen for changes in the menu: when it opens and
 * closes.
 */
@interface FlutterMenuDelegate : NSObject <NSMenuDelegate>
/*
 * When this delegate receives notification that the menu opened or closed, it
 * will send a message on the given channel to that effect for the menu item
 * with the given id (the ID comes from the data supplied by the framework to
 * FlutterMenuPlugin.setMenu).
 */
- (instancetype)initWithIdentifier:(int64_t)identifier channel:(FlutterMethodChannel*)channel;
@end

@implementation FlutterMenuDelegate {
  FlutterMethodChannel* _channel;
  int64_t _identifier;
}

- (instancetype)initWithIdentifier:(int64_t)identifier channel:(FlutterMethodChannel*)channel {
  self = [super init];
  if (self) {
    _identifier = identifier;
    _channel = channel;
  }
  return self;
}

- (void)menuWillOpen:(NSMenu*)menu {
  [_channel invokeMethod:kMenuItemOpenedMethod arguments:@(_identifier)];
}

- (void)menuDidClose:(NSMenu*)menu {
  [_channel invokeMethod:kMenuItemClosedMethod arguments:@(_identifier)];
}
@end

@interface FlutterMenuPlugin ()
// Initialize the plugin with the given method channel.
- (instancetype)initWithChannel:(FlutterMethodChannel*)channel;

// Iterates through the given menu hierarchy, and replaces "APP_NAME"
// with the localized running application name.
- (void)replaceAppName:(NSArray<NSMenuItem*>*)items;

// Look up the menu item with the given selector and tag in the list of provided
// menus and return it.
- (NSMenuItem*)findProvidedMenu:(NSMenu*)menu ofType:(SEL)selector withTag:(int)tag;

// Create a platform-provided menu from the given enum type.
- (NSMenuItem*)createPlatformProvidedMenu:(PlatformProvidedMenu)type;

// Create an NSMenuItem from information in the dictionary sent by the framework.
- (NSMenuItem*)menuItemFromFlutterRepresentation:(NSDictionary*)representation;

// Invokes kMenuItemSelectedCallbackMethod with the senders ID.
//
// Used as the callback for all Flutter-created menu items that have IDs.
- (void)flutterMenuItemSelected:(id)sender;

// Replaces the NSApp.mainMenu with menus created from an array of top level
// menus sent by the framework.
- (void)setMenu:(nonnull NSArray*)arguments;
@end

@implementation FlutterMenuPlugin {
  // The channel used to communicate with Flutter.
  FlutterMethodChannel* _channel;

  // This contains a copy of the default platform provided items.
  NSArray<NSMenuItem*>* _platformProvidedItems;
  // These are the menu delegates that will listen to open/close events for menu
  // items. This array is holding them so that we can deallocate them when
  // rebuilding the menus.
  NSMutableArray<FlutterMenuDelegate*>* _menuDelegates;
}

#pragma mark - Private Methods

- (instancetype)initWithChannel:(FlutterMethodChannel*)channel {
  self = [super init];
  if (self) {
    _channel = channel;
    _platformProvidedItems = @[];
    _menuDelegates = [[NSMutableArray alloc] init];

    // Make a copy of all the platform provided menus for later use.
    _platformProvidedItems = [[NSApp.mainMenu itemArray] mutableCopy];

    // As copied, these platform provided menu items don't yet have the APP_NAME
    // string replaced in them, so this rectifies that.
    [self replaceAppName:_platformProvidedItems];
  }
  return self;
}

/**
 * Iterates through the given menu hierarchy, and replaces "APP_NAME"
 * with the localized running application name.
 */
- (void)replaceAppName:(NSArray<NSMenuItem*>*)items {
  NSString* appName = [NSRunningApplication currentApplication].localizedName;
  for (NSMenuItem* item in items) {
    if ([[item title] containsString:kAppName]) {
      [item setTitle:[[item title] stringByReplacingOccurrencesOfString:kAppName
                                                             withString:appName]];
    }
    if ([item hasSubmenu]) {
      [self replaceAppName:[[item submenu] itemArray]];
    }
  }
}

- (NSMenuItem*)findProvidedMenu:(NSMenu*)menu ofType:(SEL)selector withTag:(int)tag {
  const NSArray<NSMenuItem*>* items = menu != nil ? menu.itemArray : _platformProvidedItems;
  for (NSMenuItem* item in items) {
    if ([item action] == selector && (tag == 0 || [item tag] == tag)) {
      return item;
    }
    if ([[item submenu] numberOfItems] > 0) {
      NSMenuItem* foundChild = [self findProvidedMenu:[item submenu] ofType:selector withTag:tag];
      if (foundChild != nil) {
        return foundChild;
      }
    }
  }
  return nil;
}

- (NSMenuItem*)createPlatformProvidedMenu:(PlatformProvidedMenu)type {
  const std::map<int, SEL> providedMenus = GetMacOSProvidedMenus();
  auto found_type = providedMenus.find(type);
  if (found_type == providedMenus.end()) {
    return nil;
  }
  SEL selectorTarget = found_type->second;
  // Since it doesn't have a definitive selector, the Services submenu is
  // assumed to be the first item with a submenu action in the first menu item
  // of the default menu set. We can't just get the title to check, since that
  // is localized, and the contents of the menu aren't fixed (or even available).
  NSMenu* startingMenu =
      type == PlatformProvidedMenu::servicesSubmenu ? [_platformProvidedItems[0] submenu] : nil;
  NSMenuItem* found = [self findProvidedMenu:startingMenu ofType:selectorTarget withTag:0];
  if (found != nil) {
    // Return a copy because the original menu item might not have been removed
    // from the main menu yet, and AppKit doesn't like menu items that exist in
    // more than one menu at a time.
    return [found copy];
  }
  return nil;
}

- (NSMenuItem*)menuItemFromFlutterRepresentation:(NSDictionary*)representation {
  if ([(NSNumber*)([representation valueForKey:kDividerKey]) intValue] == YES) {
    return [NSMenuItem separatorItem];
  }
  NSString* appName = [NSRunningApplication currentApplication].localizedName;
  NSString* title = representation[kLabelKey];
  if (title != nil) {
    title = [title stringByReplacingOccurrencesOfString:kAppName withString:appName];
  }
  NSNumber* boxedID = representation[kIdKey];
  NSNumber* platformProvidedMenuId = representation[kPlatformProvidedMenuKey];
  SEL action = (boxedID ? @selector(flutterMenuItemSelected:) : NULL);
  NSString* keyEquivalent = @"";

  if (platformProvidedMenuId != nil) {
    return [self createPlatformProvidedMenu:(PlatformProvidedMenu)platformProvidedMenuId.intValue];
  } else {
    if (representation[kShortcutEquivalentKey] != nil) {
      keyEquivalent = representation[kShortcutEquivalentKey];
    } else {
      NSNumber* triggerKeyId = representation[kShortcutTriggerKey];
      const NSDictionary<NSNumber*, NSNumber*>* specialKeys = GetMacOsSpecialKeys();
      NSNumber* trigger = specialKeys[triggerKeyId];
      if (trigger != nil) {
        keyEquivalent = [NSString stringWithFormat:@"%C", [trigger unsignedShortValue]];
      } else {
        if (([triggerKeyId unsignedLongLongValue] & kFlutterKeyIdPlaneMask) ==
            kFlutterKeyIdUnicodePlane) {
          keyEquivalent = [[NSString
              stringWithFormat:@"%C", (unichar)([triggerKeyId unsignedLongLongValue] &
                                                kFlutterKeyIdValueMask)] lowercaseString];
        }
      }
    }
  }
  NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:title
                                                action:action
                                         keyEquivalent:keyEquivalent];
  if ([keyEquivalent length] > 0) {
    item.keyEquivalentModifierMask =
        KeyEquivalentModifierMaskForModifiers(representation[kShortcutModifiersKey]);
  }
  if (boxedID) {
    item.tag = boxedID.longLongValue;
    item.target = self;
  }
  NSNumber* enabled = representation[kEnabledKey];
  if (enabled) {
    item.enabled = enabled.boolValue;
  }

  NSArray* children = representation[kChildrenKey];
  if (children && children.count > 0) {
    NSMenu* submenu = [[NSMenu alloc] initWithTitle:title];
    FlutterMenuDelegate* delegate = [[FlutterMenuDelegate alloc] initWithIdentifier:item.tag
                                                                            channel:_channel];
    [_menuDelegates addObject:delegate];
    submenu.delegate = delegate;
    submenu.autoenablesItems = NO;
    for (NSDictionary* child in children) {
      NSMenuItem* newItem = [self menuItemFromFlutterRepresentation:child];
      if (newItem != nil) {
        [submenu addItem:newItem];
      }
    }
    item.submenu = submenu;
  }
  return item;
}

- (void)flutterMenuItemSelected:(id)sender {
  NSMenuItem* item = sender;
  [_channel invokeMethod:kMenuItemSelectedCallbackMethod arguments:@(item.tag)];
}

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
  if ([call.method isEqualToString:kMenuSetMethod]) {
    NSArray* menus = call.arguments;
    [self setMenu:menus];
    result(nil);
  } else {
    result(FlutterMethodNotImplemented);
  }
}

- (void)setMenu:(NSArray*)representation {
  [_menuDelegates removeAllObjects];
  NSMenu* newMenu = [[NSMenu alloc] init];
  for (NSDictionary* item in representation.objectEnumerator) {
    NSMenuItem* menuItem = [self menuItemFromFlutterRepresentation:item];
    menuItem.representedObject = self;
    NSNumber* boxedID = item[kIdKey];
    FlutterMenuDelegate* delegate =
        [[FlutterMenuDelegate alloc] initWithIdentifier:boxedID.longLongValue channel:_channel];
    [_menuDelegates addObject:delegate];
    [menuItem submenu].delegate = delegate;
    [newMenu addItem:menuItem];
  }
  NSApp.mainMenu = newMenu;
}

#pragma mark - Public Class Methods

+ (void)registerWithRegistrar:(nonnull id<FlutterPluginRegistrar>)registrar {
  FlutterMethodChannel* channel = [FlutterMethodChannel methodChannelWithName:kChannelName
                                                              binaryMessenger:registrar.messenger];
  FlutterMenuPlugin* instance = [[FlutterMenuPlugin alloc] initWithChannel:channel];
  [registrar addMethodCallDelegate:instance channel:channel];
}

@end
