#import <Foundation/NSString.h>
#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterDartProject_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
#import "flutter/testing/testing.h"

namespace flutter::testing {

// Returns a mock FlutterViewController that is able to work in environments
// without a real pasteboard.
id CreateMockViewController(NSString* pasteboardString);

}
