// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Flutter/flutter.h>
#import <XCTest/XCTest.h>

#import "../Scenarios/TextPlatformView.h"
#import "PlatformViewUITestUtil.h"

@interface PlatformViewUITests : XCTestCase
@property(nonatomic, strong) XCUIApplication* application;
@end

@implementation PlatformViewUITests

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;

  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[ @"--platform-view" ];
  [self.application launch];
}

- (void)testPlatformView {
  XCUIElement* element = self.application.textViews.firstMatch;
  BOOL exists = [element waitForExistenceWithTimeout:kTimeToWaitForPlatformView];
  if (!exists) {
    XCTFail(@"It took longer than %@ second to find the platform view."
            @"There might be issues with the platform view's construction,"
            @"or with how the scenario is built.",
            @(kTimeToWaitForPlatformView));
  }

  NSBundle* bundle = [NSBundle bundleForClass:[self class]];
  NSString* goldenName =
      [NSString stringWithFormat:@"golden_platform_view_%@", [PlatformViewUITestUtil platformName]];
  NSString* path = [bundle pathForResource:goldenName ofType:@"png"];
  UIImage* golden = [[UIImage alloc] initWithContentsOfFile:path];

  XCUIScreenshot* screenshot = [[XCUIScreen mainScreen] screenshot];
  XCTAttachment* attachment = [XCTAttachment attachmentWithScreenshot:screenshot];
  attachment.name = @"current screen shot";
  attachment.lifetime = XCTAttachmentLifetimeKeepAlways;
  [self addAttachment:attachment];

  if (golden) {
    XCTAttachment* goldenAttachment = [XCTAttachment attachmentWithImage:golden];
    goldenAttachment.lifetime = XCTAttachmentLifetimeKeepAlways;
    goldenAttachment.name = @"golden";
    [self addAttachment:goldenAttachment];
  } else {
    XCTFail(@"This test will fail - no golden named %@ found. Follow the steps in the "
            @"README to add a new golden.",
            goldenName);
  }

  XCTAssertTrue([PlatformViewUITestUtil compareImage:golden toOther:screenshot.image]);
}

@end
