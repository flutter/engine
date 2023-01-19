// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "FlutterEngine.h"
#import "FlutterMacros.h"
#import "FlutterPlatformViews.h"
#import "FlutterPluginRegistrarMacOS.h"

/**
 * Values for the `mouseTrackingMode` property.
 */
typedef NS_ENUM(NSInteger, FlutterMouseTrackingMode) {
  // Hover events will never be sent to Flutter.
  FlutterMouseTrackingModeNone = 0,
  // Hover events will be sent to Flutter when the view is in the key window.
  FlutterMouseTrackingModeInKeyWindow,
  // Hover events will be sent to Flutter when the view is in the active app.
  FlutterMouseTrackingModeInActiveApp,
  // Hover events will be sent to Flutter regardless of window and app focus.
  FlutterMouseTrackingModeAlways,
};

/**
 * Controls a view that displays Flutter content and manages input.
 */
FLUTTER_DARWIN_EXPORT
@interface FlutterViewController : NSViewController <FlutterPluginRegistry>

/**
 * The Flutter engine associated with this view controller.
 *
 * The engine is strongly referenced by the FlutterViewController, and weakly
 * vice versa.
 */
@property(nonatomic, nonnull, readonly) FlutterEngine* engine;

/**
 * The identifier for this view controller.
 *
 * The ID is assigned when the view controller is added to FlutterEngine.
 */
@property(nonatomic, readonly) uint64_t id;

/**
 * The style of mouse tracking to use for the view. Defaults to
 * FlutterMouseTrackingModeInKeyWindow.
 */
@property(nonatomic) FlutterMouseTrackingMode mouseTrackingMode;

/**
 * Initializes a controller that will run the given project.
 *
 * In this initializer, this controller creates an engine, and is attached to
 * this engine as the default controller. In this way, this controller can not
 * be set to other engines. This initializer is suitable for the first Flutter
 * view controller of the app. To use the controller for an existing engine,
 * use initWithEngine:nibName:bundle: instead.
 *
 * @param project The project to run in this view controller. If nil, a default `FlutterDartProject`
 *                will be used.
 */
- (nonnull instancetype)initWithProject:(nullable FlutterDartProject*)project
    NS_DESIGNATED_INITIALIZER;

- (nonnull instancetype)initWithNibName:(nullable NSString*)nibNameOrNil
                                 bundle:(nullable NSBundle*)nibBundleOrNil
    NS_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithCoder:(nonnull NSCoder*)nibNameOrNil NS_DESIGNATED_INITIALIZER;
/**
 * Initializes this FlutterViewController with the specified `FlutterEngine`.
 *
 * This initializer is suitable for both the first Flutter view controller and
 * the following ones of the app.
 *
 * @param engine The `FlutterEngine` instance to attach to. Cannot be nil.
 * @param nibName The NIB name to initialize this controller with.
 * @param nibBundle The NIB bundle.
 */
- (nonnull instancetype)initWithEngine:(nonnull FlutterEngine*)engine
                               nibName:(nullable NSString*)nibName
                                bundle:(nullable NSBundle*)nibBundle NS_DESIGNATED_INITIALIZER;
/**
 * Invoked by the engine right before the engine is restarted.
 *
 * This should reset states to as if the application has just started.  It
 * usually indicates a hot restart (Shift-R in Flutter CLI.)
 */
- (void)onPreEngineRestart;

/**
 * The contentView (FlutterView)'s background color is set to black during
 * its instantiation.
 *
 * The containing layer's color can be set to the NSColor provided to this method.
 *
 * For example, the background may be set after the FlutterViewController
 * is instantiated in MainFlutterWindow.swift in the Flutter project.
 * ```swift
 * import Cocoa
 * import FlutterMacOS
 *
 * class MainFlutterWindow: NSWindow {
 *   override func awakeFromNib() {
 *     let flutterViewController = FlutterViewController.init()
 *
 *     // The background color of the window and `FlutterViewController`
 *     // are retained separately.
 *     //
 *     // In this example, both the MainFlutterWindow and FlutterViewController's
 *     // FlutterView's backgroundColor are set to clear to achieve a fully
 *     // transparent effect.
 *     //
 *     // If the window's background color is not set, it will use the system
 *     // default.
 *     //
 *     // If the `FlutterView`'s color is not set via `FlutterViewController.setBackgroundColor`
 *     // it's default will be black.
 *     self.backgroundColor = NSColor.clear
 *     flutterViewController.backgroundColor = NSColor.clear
 *
 *     let windowFrame = self.frame
 *     self.contentViewController = flutterViewController
 *     self.setFrame(windowFrame, display: true)
 *
 *     RegisterGeneratedPlugins(registry: flutterViewController)
 *
 *     super.awakeFromNib()
 *   }
 * }
 * ```
 */
@property(readwrite, nonatomic, nullable, copy) NSColor* backgroundColor;

@end
