// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>
#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterViewController.h"

#include "FlutterBinaryMessenger.h"

#if !__has_feature(objc_arc)
#error ARC must be enabled!
#endif

@interface FlutterViewControllerTest : XCTestCase
@end

@implementation FlutterViewControllerTest

- (void)testBinaryMessenger {
  id engine = OCMClassMock([FlutterEngine class]);
  FlutterViewController* vc = [[FlutterViewController alloc] initWithEngine:engine
                                                                    nibName:nil
                                                                     bundle:nil];
  XCTAssertNotNil(vc);
  id messenger = OCMProtocolMock(@protocol(FlutterBinaryMessenger));
  OCMStub([engine binaryMessenger]).andReturn(messenger);
  XCTAssertEqual(vc.binaryMessenger, messenger);
  OCMVerify([engine binaryMessenger]);
}

- (void)testItReportsLightPlatformBrightnessByDefault {
  // Setup test.
  id engine = OCMClassMock([FlutterEngine class]);

  id settingsChannel = OCMClassMock([FlutterBasicMessageChannel class]);
  OCMStub([engine settingsChannel]).andReturn(settingsChannel);

  FlutterViewController* vc = [[FlutterViewController alloc] initWithEngine:engine
                                                                    nibName:nil
                                                                     bundle:nil];

  // Exercise behavior under test.
  [vc traitCollectionDidChange:nil];

  // Verify behavior.
  OCMVerify([settingsChannel sendMessage:[OCMArg checkWithBlock:^BOOL(id message) {
                               return [message[@"platformBrightness"] isEqualToString:@"light"];
                             }]]);
}

- (void)testItReportsPlatformBrightnessWhenViewLoaded {
  // Setup test.
  id engine = OCMClassMock([FlutterEngine class]);

  id settingsChannel = OCMClassMock([FlutterBasicMessageChannel class]);
  OCMStub([engine settingsChannel]).andReturn(settingsChannel);

  FlutterViewController* vc = [[FlutterViewController alloc] initWithEngine:engine
                                                                    nibName:nil
                                                                     bundle:nil];

  // Exercise behavior under test.
  [vc viewDidLoad];

  // Verify behavior.
  OCMVerify([settingsChannel sendMessage:[OCMArg checkWithBlock:^BOOL(id message) {
                               return [message[@"platformBrightness"] isEqualToString:@"light"];
                             }]]);
}

- (void)testItReportsDarkPlatformBrightnessWhenTraitCollectionRequestsIt {
  // Setup test.
  id engine = OCMClassMock([FlutterEngine class]);

  id settingsChannel = OCMClassMock([FlutterBasicMessageChannel class]);
  OCMStub([engine settingsChannel]).andReturn(settingsChannel);

  FlutterViewController* vc = [[FlutterViewController alloc] initWithEngine:engine
                                                                    nibName:nil
                                                                     bundle:nil];
  id mockTraitCollection = [self setupFakeUserInterfaceStyle:UIUserInterfaceStyleDark];

  // Exercise behavior under test.
  [vc traitCollectionDidChange:nil];

  // Verify behavior.
  OCMVerify([settingsChannel sendMessage:[OCMArg checkWithBlock:^BOOL(id message) {
                               return [message[@"platformBrightness"] isEqualToString:@"light"];
                             }]]);

  // Restore UIUserInterfaceStyle
  [mockTraitCollection stopMocking];
}

- (UITraitCollection*)setupFakeUserInterfaceStyle:(UIUserInterfaceStyle)style {
  id mockTraitCollection = OCMClassMock([UITraitCollection class]);
  OCMStub([mockTraitCollection userInterfaceStyle]).andReturn(UIUserInterfaceStyleDark);
  OCMStub([mockTraitCollection currentTraitCollection]).andReturn(mockTraitCollection);
  return mockTraitCollection;
}

@end
