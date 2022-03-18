// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>

#import <objc/message.h>
#include <sys/_types/_int64_t.h>
#include <sys/_types/_size_t.h>

#import "FlutterMenuPlugin.h"
#import "flutter/shell/platform/common/platform_provided_menu.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterChannels.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"

// Channel constants
static NSString* const kChannelName = @"flutter/menu";
static NSString* const kMenuSetMethod = @"Menu.SetMenu";
static NSString* const kMenuItemSelectedCallbackMethod = @"Menu.SelectedCallback";
static NSString* const kMenuItemOpenedMethod = @"Menu.Opened";
static NSString* const kMenuItemClosedMethod = @"Menu.Closed";

// Serialization keys for menu objects
static NSString* const kIdKey = @"id";
static NSString* const kLabelKey = @"label";
static NSString* const kEnabledKey = @"enabled";
static NSString* const kChildrenKey = @"children";
static NSString* const kDividerKey = @"isDivider";
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

// Maps the string used by NSMenuItem for the given special key equivalent.
// Keys are the logical key ids of matching trigger keys.
static const NSDictionary<NSNumber*, NSNumber*>* kMacOSSpecialKeys = @{
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

// The mapping from the PlatformProvidedMenu enum to the macOS selectors for the provided
// menus.
static const std::map<int, SEL> kMacOSProvidedMenus = {
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
    {PlatformProvidedMenu::arrangeWindowInFront, @selector(arrangeInFront:)},
};

// Returns the NSEventModifierFlags of |modifiers|, a value from
// kShortcutKeyModifiers.
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

@interface FlutterMenuDelegate : NSObject <NSMenuDelegate>
- (instancetype)initWithId:(int64_t)id andChannel:(FlutterMethodChannel*)channel;
- (void)menuWillOpen:(NSMenu*)menu;
- (void)menuDidClose:(NSMenu*)menu;
@end

@implementation FlutterMenuDelegate {
  FlutterMethodChannel* _channel;
  int64_t _id;
}

- (instancetype)initWithId:(int64_t)id andChannel:(FlutterMethodChannel*)channel {
  self = [super init];
  if (self) {
    _id = id;
    _channel = channel;
  }
  return self;
}

- (void)menuWillOpen:(NSMenu*)menu {
  [_channel invokeMethod:kMenuItemOpenedMethod arguments:@(_id)];
}

- (void)menuDidClose:(NSMenu*)menu {
  [_channel invokeMethod:kMenuItemClosedMethod arguments:@(_id)];
}
@end

@implementation FlutterMenuPlugin {
  // The channel used to communicate with Flutter.
  FlutterMethodChannel* _channel;

  NSArray<NSMenuItem*>* _platformProvidedItemArray;
  NSMutableArray<FlutterMenuDelegate*>* _menuDelegates;
}

- (instancetype)initWithChannel:(FlutterMethodChannel*)channel {
  self = [super init];
  if (self) {
    _channel = channel;
    _platformProvidedItemArray = @[];
    _menuDelegates = [[NSMutableArray alloc] init];

    // Make a copy of all the platform provided menus for later use.
    _platformProvidedItemArray = [[NSArray alloc] initWithArray:[NSApp.mainMenu itemArray]
                                                            copyItems:YES];

    // As copied, these platform provided menu items don't yet have the APP_NAME
    // applied to them, so this rectifies that.
    [self reapplyAppName:_platformProvidedItemArray];

    // Now that we've copied them, clear out all the default menus in
    // NSApp.mainMenu, but leave one empty menu to avoid AppKit squirrelly-ness
    // around empty menu bars.
    [self removeAllMenus];
  }
  return self;
}

/**
 * Iterates through the given menu hierarchy, and replaces "APP_NAME"
 * with the localized running application name.
 */
- (void)reapplyAppName:(NSArray<NSMenuItem*>*)items {
  NSString* appName = [NSRunningApplication currentApplication].localizedName;
  for (NSUInteger i = 0; i < [items count]; ++i) {
    NSMenuItem* item = [items objectAtIndex:i];
    if ([[item title] containsString:kAppName]) {
      [item setTitle:[[item title] stringByReplacingOccurrencesOfString:kAppName
                                                             withString:appName]];
    }
    if ([item hasSubmenu]) {
      [self reapplyAppName:[[item submenu] itemArray]];
    }
  }
}

/**
 * Removes all but one top-level menu from the app menu, leaving the last one
 * with no menu items.
 */
- (void)removeAllMenus {
  if ([NSApp.mainMenu numberOfItems] == 0) {
    // There should always be at least one item to represent the APP_NAME menu.
    [NSApp.mainMenu addItem:[[NSMenuItem alloc] init]];
  }
  // Clean out any existing menu items. We can't just call removeAllItems
  // because that destroys the <APP_NAME> menu, which causes an  button (in
  // addition to the usual  logo system menu) to be put into the menu as a
  // placeholder, and it can't be removed once it appears. So, we remove all the
  // menus at index 1+, and then all the items in the menu at index 0. When we
  // add our first menu, we skip creating a new one, and just use the existing
  // one.
  while ([NSApp.mainMenu numberOfItems] > 1) {
    [NSApp.mainMenu removeItemAtIndex:1];
  }
  NSMenu* appNameMenu = [[NSApp.mainMenu itemArray][0] submenu];
  while ([appNameMenu numberOfItems] > 0) {
    [appNameMenu removeItemAtIndex:0];
  }
  [_menuDelegates removeAllObjects];
}

/**
 * Removes Flutter menu items that were previously added, then builds and adds
 * new top-level menu items constructed from |representation|.
 */
- (void)setMenu:(NSArray*)representation {
  [self removeAllMenus];
  bool firstMenu = true;
  int index = 0;
  for (NSDictionary* item in representation.objectEnumerator) {
    NSMenuItem* menuItem = [self menuItemFromFlutterRepresentation:item];
    menuItem.representedObject = self;
    if (firstMenu) {
      // If we're adding the first menu in the menu bar, then just set the items
      // in the submenu, rather than adding a new menu. This is because AppKit
      // gets very squirrelly when you remove the <APP_NAME> menu, so we just
      // empty it out in removeAllMenus, and refill it here. This ignores the
      // label of the first item (because it will always be set to the APP_NAME
      // by AppKit).
      NSMenu* appNameMenu = [[NSApp.mainMenu itemAtIndex:0] submenu];
      for (int i = 0; i < [menuItem.submenu numberOfItems]; ++i) {
        [appNameMenu addItem:[[menuItem.submenu itemAtIndex:i] copy]];
      }
      firstMenu = false;
    } else {
      [NSApp.mainMenu addItem:menuItem];
    }
    NSNumber* boxedID = item[kIdKey];
    FlutterMenuDelegate* delegate = [[FlutterMenuDelegate alloc] initWithId:boxedID.longLongValue
                                                                 andChannel:_channel];
    [_menuDelegates addObject:delegate];
    [[NSApp.mainMenu itemAtIndex:index++] submenu].delegate = delegate;
  }
}

- (NSMenuItem*)findDefaultMenu:(NSMenu*)menu ofType:(SEL)selector withTag:(int)tag {
  auto enumerator = menu != nullptr ? menu.itemArray.objectEnumerator
                                    : _platformProvidedItemArray.objectEnumerator;
  for (NSMenuItem* item in enumerator) {
    if ([item action] == selector && (tag == 0 || [item tag] == tag)) {
      return item;
    }
    if ([[item submenu] numberOfItems] > 0) {
      NSMenuItem* child_find = [self findDefaultMenu:[item submenu] ofType:selector withTag:tag];
      if (child_find != nullptr) {
        return child_find;
      }
    }
  }
  return nullptr;
}

- (NSMenuItem*)createPlatformProvidedMenu:(PlatformProvidedMenu)type {
  NSMenuItem* found = nullptr;
  auto found_type = kMacOSProvidedMenus.find(type);
  if (found_type == kMacOSProvidedMenus.end()) {
    return nullptr;
  }
  SEL selector_target = found_type->second;
  // Since it doesn't have a definitive selector, the Services submenu is
  // assumed to be the first item with a submenu action in the first menu item
  // of the default menu set. We can't just get the title to check, since that
  // is localized.
  NSMenu* starting_menu = type == PlatformProvidedMenu::servicesSubmenu
                              ? [_platformProvidedItemArray[0] submenu]
                              : nullptr;
  found = [self findDefaultMenu:starting_menu ofType:selector_target withTag:0];
  if (found != nullptr) {
    return [found copy];
  }
  return nullptr;
}

/**
 * Constructs and returns an NSMenuItem corresponding to the item in
 * |representation|, including recursively creating children if it has a
 * submenu.
 */
- (NSMenuItem*)menuItemFromFlutterRepresentation:(NSDictionary*)representation {
  if ([(NSNumber*)([representation valueForKey:kDividerKey]) intValue] == YES) {
    return [NSMenuItem separatorItem];
  } else {
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
      return
          [self createPlatformProvidedMenu:(PlatformProvidedMenu)platformProvidedMenuId.intValue];
    } else {
      NSNumber* triggerKeyId = representation[kShortcutTriggerKey];
      if ([kMacOSSpecialKeys objectForKey:triggerKeyId] != nil) {
        keyEquivalent =
            [NSString stringWithFormat:@"%C", [kMacOSSpecialKeys[triggerKeyId] unsignedShortValue]];
      } else {
        if (([triggerKeyId unsignedLongLongValue] & kFlutterKeyIdPlaneMask) ==
            kFlutterKeyIdUnicodePlane) {
          keyEquivalent = [[NSString
              stringWithFormat:@"%C", (unichar)([triggerKeyId unsignedLongLongValue] &
                                                kFlutterKeyIdValueMask)] lowercaseString];
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
      FlutterMenuDelegate* delegate = [[FlutterMenuDelegate alloc] initWithId:item.tag
                                                                   andChannel:_channel];
      [_menuDelegates addObject:delegate];
      submenu.delegate = delegate;
      submenu.autoenablesItems = NO;
      for (NSDictionary* child in children) {
        NSMenuItem* new_item = [self menuItemFromFlutterRepresentation:child];
        if (new_item != nil) {
          [submenu addItem:new_item];
        }
      }
      item.submenu = submenu;
    }
    return item;
  }
}

/**
 * Invokes kMenuItemSelectedCallbackMethod with the senders ID.
 *
 * Used as the callback for all Flutter-created menu items that have IDs.
 */
- (void)flutterMenuItemSelected:(id)sender {
  NSMenuItem* item = sender;
  [_channel invokeMethod:kMenuItemSelectedCallbackMethod arguments:@(item.tag)];
}

+ (void)registerWithRegistrar:(nonnull id<FlutterPluginRegistrar>)registrar {
  FlutterMethodChannel* channel = [FlutterMethodChannel methodChannelWithName:kChannelName
                                                              binaryMessenger:registrar.messenger];
  FlutterMenuPlugin* instance = [[FlutterMenuPlugin alloc] initWithChannel:channel];
  [registrar addMethodCallDelegate:instance channel:channel];
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
@end
