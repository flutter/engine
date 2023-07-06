// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "FlutterBinaryMessenger.h"
#import "FlutterChannels.h"
#import "FlutterMacros.h"
#import "FlutterPlatformViews.h"
#import "FlutterPluginMacOS.h"
#import "FlutterTexture.h"

// TODO: Merge this file and FlutterPluginMacOS.h with the iOS FlutterPlugin.h, sharing all but
// the platform-specific methods.

/**
 * The protocol for an object managing registration for a plugin. It provides access to application
 * context, as allowing registering for callbacks for handling various conditions.
 *
 * Currently the macOS PluginRegistrar has very limited functionality, but is expected to expand
 * over time to more closely match the functionality of FlutterPluginRegistrar.
 */
FLUTTER_DARWIN_EXPORT
@protocol FlutterPluginRegistrar <NSObject>

/**
 * The binary messenger used for creating channels to communicate with the Flutter engine.
 */
@property(nonnull, readonly) id<FlutterBinaryMessenger> messenger;

/**
 * Returns a `FlutterTextureRegistry` for registering textures
 * provided by the plugin.
 */
@property(nonnull, readonly) id<FlutterTextureRegistry> textures;

/**
 * The view displaying Flutter content.
 *
 * Currently Flutter only supports one view, and this is the view.
 *
 * Flutter plans to support multiple views in the future. Although single-view
 * APIs will eventually be replaced by their multi-view variants, during the
 * deprecation period, the single-view APIs will coexist with and work with the
 * multi-view APIs as if the other views don't exist. To achieve this,
 * all behaviors of "the single view" (which is called "the implicit view") are
 * preserved, allowing legacy single-view APIs to continue working, while
 * new-style views created by new ways must be operated by the upcoming
 * multi-view APIs.
 *
 * Plugins written for a single view can keep operating on this view and expect
 * unchanged behavior for the implicit view. This includes that:
 *
 *  - The first view controller attached to the engine will be linked to the
 *    implicit view.
 *  - Single-view Flutter APIs will operate the implicit view.
 *
 * This method may return |nil| if the view is not assigned. In single-view
 * apps, this means that the app is running headlessly. In multi-view apps,
 * this means that the compatible mode is disabled, or that the compatible mode
 * is on but no view controller has been attached to the engine yet.
 */
@property(nullable, readonly) NSView* view;

/**
 * Registers |delegate| to receive handleMethodCall:result: callbacks for the given |channel|.
 */
- (void)addMethodCallDelegate:(nonnull id<FlutterPlugin>)delegate
                      channel:(nonnull FlutterMethodChannel*)channel;

/**
 * Registers a `FlutterPlatformViewFactory` for creation of platform views.
 *
 * Plugins expose `NSView` for embedding in Flutter apps by registering a view factory.
 *
 * @param factory The view factory that will be registered.
 * @param factoryId A unique identifier for the factory, the Dart code of the Flutter app can use
 *   this identifier to request creation of a `NSView` by the registered factory.
 */
- (void)registerViewFactory:(nonnull NSObject<FlutterPlatformViewFactory>*)factory
                     withId:(nonnull NSString*)factoryId;

/**
 * Returns the file name for the given asset.
 * The returned file name can be used to access the asset in the application's main bundle.
 *
 * @param asset The name of the asset. The name can be hierarchical.
 * @return the file name to be used for lookup in the main bundle.
 */
- (nonnull NSString*)lookupKeyForAsset:(nonnull NSString*)asset;

/**
 * Returns the file name for the given asset which originates from the specified package.
 * The returned file name can be used to access the asset in the application's main bundle.
 *
 *
 * @param asset The name of the asset. The name can be hierarchical.
 * @param package The name of the package from which the asset originates.
 * @return the file name to be used for lookup in the main bundle.
 */
- (nonnull NSString*)lookupKeyForAsset:(nonnull NSString*)asset
                           fromPackage:(nonnull NSString*)package;

@end

/**
 * A registry of Flutter macOS plugins.
 *
 * Plugins are identified by unique string keys, typically the name of the
 * plugin's main class.
 *
 * Plugins typically need contextual information and the ability to register
 * callbacks for various application events. To keep the API of the registry
 * focused, these facilities are not provided directly by the registry, but by
 * a `FlutterPluginRegistrar`, created by the registry in exchange for the unique
 * key of the plugin.
 *
 * There is no implied connection between the registry and the registrar.
 * Specifically, callbacks registered by the plugin via the registrar may be
 * relayed directly to the underlying iOS application objects.
 */
@protocol FlutterPluginRegistry <NSObject>

/**
 * Returns a registrar for registering a plugin.
 *
 * @param pluginKey The unique key identifying the plugin.
 */
- (nonnull id<FlutterPluginRegistrar>)registrarForPlugin:(nonnull NSString*)pluginKey;

@end
