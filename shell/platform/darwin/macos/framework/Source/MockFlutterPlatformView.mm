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

- (instancetype)init {
  self = [super init];
  [[NSColor blueColor] setFill];
  if (self) {
    // gMockPlatformView = self;
  }
  return self;
}

- (void)dealloc {
  // [super dealloc];
}

@end

@implementation MockFlutterPlatformView

- (instancetype)init {
  if (self = [super init]) {
    _view = [[MockPlatformView alloc] init];
    _test_view_id = 1;
  }
  return self;
}

- (instancetype)initWithFrame:(CGRect) frame
                arguments:(id _Nullable) args {
  if (self = [super init]) {
    _view = [[NSTextField alloc] initWithFrame:frame];
    [(NSTextField*)_view setStringValue:@"hello2"];
    // [(NSTextField*)_view setStringValue:args];
    [(NSTextField*)_view setTextColor:[NSColor blueColor]];
    _test_view_id = 1;
  }
  return self;
}

- (void)dealloc {
  // [_view release];
  _view = nil;
  // [super dealloc];
}

- (int64_t) GetTestViewID {
  return _test_view_id;
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
