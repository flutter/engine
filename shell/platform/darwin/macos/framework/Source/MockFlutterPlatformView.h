#import <Foundation/Foundation.h>
#import <Foundation/NSObject.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterPlatformViews.h"

@interface MockFlutterPlatformView : NSObject <FlutterPlatformView>
  @property(nonatomic, strong) NSView* view;
@end

@interface MockPlatformView : NSTextView
@end

@interface MockFlutterPlatformFactory : NSObject <FlutterPlatformViewFactory>
@end
