// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <XCTest/XCTest.h>
#import "../Scenarios/TextPlatformView.h"
#import "PlatformViewUITestUtil.h"

// Clip Rect Tests
@interface PlatformViewMutationClipRectTests : XCTestCase

@property(nonatomic, strong) XCUIApplication* application;

@end

@implementation PlatformViewMutationClipRectTests

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;

  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[@"--platform-view-cliprect"];
  [self.application launch];
}

- (void)testPlatformView {

  XCUIElement *element = self.application.textViews.firstMatch;
  BOOL exists = [element waitForExistenceWithTimeout:kTimeToWaitForPlatformView];
  if (!exists) {
    XCTFail(@"It took longer than %@ second to find the platform view."
            @"There might be issues with the platform view's construction,"
            @"or with how the scenario is built.", @(kTimeToWaitForPlatformView));
  }

  NSBundle* bundle = [NSBundle bundleForClass:[self class]];
  NSString* goldenName =
  [NSString stringWithFormat:@"golden_platform_view_cliprect_%@", [PlatformViewUITestUtil platformName]];
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

// Clip RRect Tests
@interface PlatformViewMutationClipRRectTests : XCTestCase

@property(nonatomic, strong) XCUIApplication* application;

@end

@implementation PlatformViewMutationClipRRectTests

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;

  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[@"--platform-view-cliprrect"];
  [self.application launch];
}

- (void)testPlatformView {

  XCUIElement *element = self.application.otherElements.firstMatch;
  BOOL exists = [element waitForExistenceWithTimeout:kTimeToWaitForPlatformView];
  if (!exists) {
    XCTFail(@"It took longer than %@ second to find the platform view."
            @"There might be issues with the platform view's construction,"
            @"or with how the scenario is built.", @(kTimeToWaitForPlatformView));
  }

  NSBundle* bundle = [NSBundle bundleForClass:[self class]];
  NSString* goldenName =
  [NSString stringWithFormat:@"golden_platform_view_cliprrect_%@", [PlatformViewUITestUtil platformName]];
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

// Clip Path Tests
@interface PlatformViewMutationClipPathTests : XCTestCase

@property(nonatomic, strong) XCUIApplication* application;

@end

@implementation PlatformViewMutationClipPathTests

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;

  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[@"--platform-view-clippath"];
  [self.application launch];
}

- (void)testPlatformView {

  XCUIElement *element = self.application.otherElements.firstMatch;
  BOOL exists = [element waitForExistenceWithTimeout:kTimeToWaitForPlatformView];
  if (!exists) {
    XCTFail(@"It took longer than %@ second to find the platform view."
            @"There might be issues with the platform view's construction,"
            @"or with how the scenario is built.", @(kTimeToWaitForPlatformView));
  }

  NSBundle* bundle = [NSBundle bundleForClass:[self class]];
  NSString* goldenName =
  [NSString stringWithFormat:@"golden_platform_view_clippath_%@", [PlatformViewUITestUtil platformName]];
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

// Transform tests
@interface PlatformViewMutationTransformTests : XCTestCase

@property(nonatomic, strong) XCUIApplication* application;

@end

@implementation PlatformViewMutationTransformTests

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;

  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[@"--platform-view-transform"];
  [self.application launch];
}

- (void)testPlatformView {

  XCUIElement *element = self.application.otherElements.firstMatch;
  BOOL exists = [element waitForExistenceWithTimeout:kTimeToWaitForPlatformView];
  if (!exists) {
    XCTFail(@"It took longer than %@ second to find the platform view."
            @"There might be issues with the platform view's construction,"
            @"or with how the scenario is built.", @(kTimeToWaitForPlatformView));
  }

  NSBundle* bundle = [NSBundle bundleForClass:[self class]];
  NSString* goldenName =
  [NSString stringWithFormat:@"golden_platform_view_transform_%@", [PlatformViewUITestUtil platformName]];
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

// Opacity tests
@interface PlatformViewMutationOpacityTests : XCTestCase

@property(nonatomic, strong) XCUIApplication* application;

@end

@implementation PlatformViewMutationOpacityTests

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;

  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[@"--platform-view-opacity"];
  [self.application launch];
}

- (void)testPlatformView {

  XCUIElement *element = self.application.textViews.firstMatch;
  BOOL exists = [element waitForExistenceWithTimeout:kTimeToWaitForPlatformView];
  if (!exists) {
    XCTFail(@"It took longer than %@ second to find the platform view."
            @"There might be issues with the platform view's construction,"
            @"or with how the scenario is built.", @(kTimeToWaitForPlatformView));
  }


  NSBundle* bundle = [NSBundle bundleForClass:[self class]];
  NSString* goldenName =
  [NSString stringWithFormat:@"golden_platform_view_opacity_%@", [PlatformViewUITestUtil platformName]];
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
