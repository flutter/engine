#import <Foundation/NSString.h>
#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterDartProject_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
#import "flutter/testing/testing.h"

namespace flutter::testing {

// Returns a mock FlutterViewController that is able to work in environments
// without a real pasteboard.
static id CreateMockViewController(NSString* pasteboardString) {
  NSString* fixtures = @(testing::GetFixturesPath());
  FlutterDartProject* project = [[FlutterDartProject alloc]
      initWithAssetsPath:fixtures
             ICUDataPath:[fixtures stringByAppendingString:@"/icudtl.dat"]];
  FlutterViewController* viewController = [[FlutterViewController alloc] initWithProject:project];

  // Mock pasteboard so that this test will work in environments without a
  // real pasteboard.
  id pasteboardMock = OCMClassMock([NSPasteboard class]);
  OCMExpect([pasteboardMock stringForType:[OCMArg any]]).andDo(^(NSInvocation* invocation) {
    NSString* returnValue = pasteboardString.length > 0 ? pasteboardString : nil;
    [invocation setReturnValue:&returnValue];
  });
  id viewControllerMock = OCMPartialMock(viewController);
  OCMStub([viewControllerMock pasteboard]).andReturn(pasteboardMock);
  return viewControllerMock;
}

}
