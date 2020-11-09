#import <Cocoa/Cocoa.h>

#import <Foundation/Foundation.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterPlatformViews.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/MockFlutterPlatformView.h"

@implementation MockPlatformView

- (instancetype)initWithFrame:(CGRect) frame {
   self = [super initWithFrame: frame];
   [super setString:@"hello1"];
   [super setTextColor:[NSColor blueColor]];
   return self;
}

@end

@implementation MockFlutterPlatformView

- (instancetype)init {
  if (self = [super init]) {
    _view = [[MockPlatformView alloc] init];
  }
  return self;
}

- (instancetype)initWithFrame:(CGRect) frame
                arguments:(id _Nullable) args {
  if (self = [super init]) {
    _view = [[MockPlatformView alloc] initWithFrame:frame];
  }
  return self;
}

- (void)dealloc {
  _view = nil;
}

@end

@implementation MockFlutterPlatformFactory
- (NSObject<FlutterPlatformView>*)createWithFrame:(CGRect)frame
                                   viewIdentifier:(int64_t)viewId
                                        arguments:(id _Nullable)args {
  return [[MockFlutterPlatformView alloc] initWithFrame:frame arguments: args];
}

- (NSObject<FlutterMessageCodec>*)createArgsCodec {
  return [FlutterStandardMessageCodec sharedInstance];
}

@end
