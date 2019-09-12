#include "AppDelegate.h"
#import "TestExternalTexture.h"
#import "TextPlatformView.h"

@interface NoStatusBarFlutterViewController : FlutterViewController
@end

@implementation NoStatusBarFlutterViewController
- (BOOL)prefersStatusBarHidden {
  return YES;
}
@end

@interface AppDelegate ()
@property(nonatomic, strong) TestExternalTexture* externalTexture;
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
  self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
  // This argument is used by the XCUITest for Platform Views so that the app
  // under test will create platform views.
  if ([[[NSProcessInfo processInfo] arguments] containsObject:@"--platform-view"]) {
    FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"PlatformViewTest" project:nil];
    [engine runWithEntrypoint:nil];

    FlutterViewController* flutterViewController =
        [[NoStatusBarFlutterViewController alloc] initWithEngine:engine nibName:nil bundle:nil];
    [engine.binaryMessenger
        setMessageHandlerOnChannel:@"scenario_status"
              binaryMessageHandler:^(NSData* _Nullable message, FlutterBinaryReply _Nonnull reply) {
                [engine.binaryMessenger
                    sendOnChannel:@"set_scenario"
                          message:[@"text_platform_view" dataUsingEncoding:NSUTF8StringEncoding]];
              }];
    TextPlatformViewFactory* textPlatformViewFactory =
        [[TextPlatformViewFactory alloc] initWithMessenger:flutterViewController.binaryMessenger];
    NSObject<FlutterPluginRegistrar>* registrar =
        [flutterViewController.engine registrarForPlugin:@"scenarios/TextPlatformViewPlugin"];
    [registrar registerViewFactory:textPlatformViewFactory withId:@"scenarios/textPlatformView"];
    self.window.rootViewController = flutterViewController;
  } else if ([[[NSProcessInfo processInfo] arguments] containsObject:@"--external-texture"]) {
    FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"ExternalTextureTest" project:nil];
    [engine runWithEntrypoint:nil];

    FlutterViewController* flutterViewController =
        [[NoStatusBarFlutterViewController alloc] initWithEngine:engine nibName:nil bundle:nil];

    [engine.binaryMessenger
        setMessageHandlerOnChannel:@"scenario_status"
              binaryMessageHandler:^(NSData* _Nullable message, FlutterBinaryReply _Nonnull reply) {
                [engine.binaryMessenger
                    sendOnChannel:@"set_scenario"
                          message:[@"external_texture" dataUsingEncoding:NSUTF8StringEncoding]];
              }];

    [engine.binaryMessenger
        setMessageHandlerOnChannel:@"create_external_texture"
              binaryMessageHandler:^(NSData* _Nullable message, FlutterBinaryReply _Nonnull reply) {
                NSObject<FlutterPluginRegistrar>* registrar = [flutterViewController.engine
                    registrarForPlugin:@"scenarios/ExternalTexturePlugin"];
                self.externalTexture =
                    [[TestExternalTexture alloc] initWithWithRegistrar:registrar];
                int64_t textureID = [registrar.textures registerShareTexture:self.externalTexture];
                [self.externalTexture startWithID:textureID];
                [engine.binaryMessenger
                    sendOnChannel:@"update_data"
                          message:[[NSString stringWithFormat:@"%lld", textureID]
                                      dataUsingEncoding:NSUTF8StringEncoding]];
              }];

    self.window.rootViewController = flutterViewController;
  } else {
    self.window.rootViewController = [[UIViewController alloc] init];
  }
  [self.window makeKeyAndVisible];

  return [super application:application didFinishLaunchingWithOptions:launchOptions];
}

@end
