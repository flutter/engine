// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#import <XCTest/XCTest.h>
#import "GoldenImage.h"

static const NSInteger kSecondsToWaitForReady = 30;
static NSString* const kGoldenPrefix = @"golden_image_scenario_";


@interface ImageScenaroTest : XCTestCase

@property(nonatomic, strong) XCUIApplication* application;
@property(nonatomic, strong) GoldenImage *golden;

@end

@implementation ImageScenaroTest

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;
  self.golden = [[GoldenImage alloc] initWithGoldenNamePrefix:kGoldenPrefix];
  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[ @"--image-scenario" ];
  [self.application launch];
}
- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testGolden {
    NSPredicate* predicateToFindPlatformView =
        [NSPredicate predicateWithBlock:^BOOL(id _Nullable evaluatedObject,
                                              NSDictionary<NSString*, id>* _Nullable bindings) {
          XCUIElement* element = evaluatedObject;
          return [element.label isEqualToString:@"ready"];
        }];
    XCUIElement* firstElement =
        [self.application.otherElements elementMatchingPredicate:predicateToFindPlatformView];
    if (![firstElement waitForExistenceWithTimeout:kSecondsToWaitForReady]) {
      NSLog(@"%@", self.application.debugDescription);
      XCTFail(@"Failed due to not able to make the scene ready with %@ seconds", @(kSecondsToWaitForReady));
    }
    GoldenImage* golden = self.golden;

    XCUIScreenshot* screenshot = [[XCUIScreen mainScreen] screenshot];
    XCTAttachment* attachment = [XCTAttachment attachmentWithScreenshot:screenshot];
    attachment.lifetime = XCTAttachmentLifetimeKeepAlways;
    [self addAttachment:attachment];

    if (golden.image) {
      XCTAttachment* goldenAttachment = [XCTAttachment attachmentWithImage:golden.image];
      goldenAttachment.lifetime = XCTAttachmentLifetimeKeepAlways;
      [self addAttachment:goldenAttachment];
    } else {
      XCTFail(@"This test will fail - no golden named %@ found. Follow the steps in the "
              @"README to add a new golden.",
              golden.goldenName);
    }

    XCTAssertTrue([golden compareGoldenToImage:screenshot.image]);
}

@end
