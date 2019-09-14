#include "AppDelegate.h"
#import "TextPlatformView.h"

@interface NoStatusBarFlutterViewController : FlutterViewController

@end

@implementation NoStatusBarFlutterViewController
- (BOOL)prefersStatusBarHidden {
  return YES;
}
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
  self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

  // This argument is used by the XCUITest for Platform Views so that the app
  // under test will create platform views.
  // The launchArgsMap should match the one in the `PlatformVieGoldenTestManager`.
  NSDictionary<NSString*, NSString*>* launchArgsMap = @{
    @"--platform-view" : @"platform_view",
    @"--platform-view-cliprect" : @"platform_view_cliprect",
    @"--platform-view-cliprrect" : @"platform_view_cliprrect",
    @"--platform-view-clippath" : @"platform_view_clippath",
    @"--platform-view-transform" : @"platform_view_transform",
    @"--platform-view-opacity" : @"platform_view_opacity",
  };
  __block BOOL hasGoldenLaunchArg = NO;
  [launchArgsMap enumerateKeysAndObjectsUsingBlock:^(NSString* argument, NSString* testName, BOOL *stop) {
    if ([[[NSProcessInfo processInfo] arguments] containsObject:argument]) {
      [self readyContextForPlatformViewTests:testName];
      hasGoldenLaunchArg = YES;
      *stop = YES;
    }
  }];
  if (!hasGoldenLaunchArg) {
    self.window.rootViewController = [[UIViewController alloc] init];
  }

  [self.window makeKeyAndVisible];

  return [super application:application didFinishLaunchingWithOptions:launchOptions];
}

- (void)readyContextForPlatformViewTests:(NSString*)scenarioIdentifier {
  FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"PlatformViewTest" project:nil];
  [engine runWithEntrypoint:nil];

  FlutterViewController* flutterViewController =
      [[NoStatusBarFlutterViewController alloc] initWithEngine:engine nibName:nil bundle:nil];
  [engine.binaryMessenger
      setMessageHandlerOnChannel:@"scenario_status"
            binaryMessageHandler:^(NSData* _Nullable message, FlutterBinaryReply _Nonnull reply) {
              [engine.binaryMessenger
                  sendOnChannel:@"set_scenario"
                        message:[scenarioIdentifier dataUsingEncoding:NSUTF8StringEncoding]];
            }];
  TextPlatformViewFactory* textPlatformViewFactory =
      [[TextPlatformViewFactory alloc] initWithMessenger:flutterViewController.binaryMessenger];
  NSObject<FlutterPluginRegistrar>* registrar =
      [flutterViewController.engine registrarForPlugin:@"scenarios/TextPlatformViewPlugin"];
  [registrar registerViewFactory:textPlatformViewFactory withId:@"scenarios/textPlatformView"];
  self.window.rootViewController = flutterViewController;
}

@end
