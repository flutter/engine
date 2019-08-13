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

- (void)testItReportsPlatformBrightnessWhenViewWillAppear {
  // Setup test.
  id engine = OCMClassMock([FlutterEngine class]);

  id settingsChannel = OCMClassMock([FlutterBasicMessageChannel class]);
  OCMStub([engine settingsChannel]).andReturn(settingsChannel);

  FlutterViewController* vc = [[FlutterViewController alloc] initWithEngine:engine
                                                                    nibName:nil
                                                                     bundle:nil];

  // Exercise behavior under test.
  [vc viewWillAppear:false];

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

  FlutterViewController* realVC = [[FlutterViewController alloc] initWithEngine:engine
                                                                        nibName:nil
                                                                         bundle:nil];
  id mockTraitCollection = [self setupFakeUserInterfaceStyle:UIUserInterfaceStyleDark];
  
  // We partially mock the real FlutterViewController to act as the OS and report
  // the UITraitCollection of our choice. Mocking the object under test is not
  // desirable, but given that the OS does not offer a DI approach to providing
  // our own UITraitCollection, this seems to be the least bad option.
  id partialMockVC = OCMPartialMock(realVC);
  OCMStub([partialMockVC traitCollection]).andReturn(mockTraitCollection);

  // Exercise behavior under test.
  [partialMockVC traitCollectionDidChange:nil];

  // Verify behavior.
  OCMVerify([settingsChannel sendMessage:[OCMArg checkWithBlock:^BOOL(id message) {
                               return [message[@"platformBrightness"] isEqualToString:@"dark"];
                             }]]);

  // Restore UIUserInterfaceStyle
  [partialMockVC stopMocking];
  [mockTraitCollection stopMocking];
}

- (UITraitCollection*)setupFakeUserInterfaceStyle:(UIUserInterfaceStyle)style {
  id mockTraitCollection = OCMClassMock([UITraitCollection class]);
  OCMStub([mockTraitCollection userInterfaceStyle]).andReturn(UIUserInterfaceStyleDark);
  return mockTraitCollection;
}

- (void)testItReportsNormalPlatformContrastByDefault {
  if (!@available(iOS 13, *)) {
    return;
  }

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
                               return [message[@"platformContrast"] isEqualToString:@"normal"];
                             }]]);
}

- (void)testItReportsPlatformContrastWhenViewWillAppear {
  if (!@available(iOS 13, *)) {
    return;
  }

  // Setup test.
  id engine = OCMClassMock([FlutterEngine class]);

  id settingsChannel = OCMClassMock([FlutterBasicMessageChannel class]);
  OCMStub([engine settingsChannel]).andReturn(settingsChannel);

  FlutterViewController* vc = [[FlutterViewController alloc] initWithEngine:engine
                                                                    nibName:nil
                                                                     bundle:nil];

  // Exercise behavior under test.
  [vc viewWillAppear:false];

  // Verify behavior.
  OCMVerify([settingsChannel sendMessage:[OCMArg checkWithBlock:^BOOL(id message) {
                               return [message[@"platformContrast"] isEqualToString:@"normal"];
                             }]]);
}

- (void)testItReportsHighContrastWhenTraitCollectionRequestsIt {
  if (!@available(iOS 13, *)) {
    return;
  }

  // Setup test.
  id engine = OCMClassMock([FlutterEngine class]);

  id settingsChannel = OCMClassMock([FlutterBasicMessageChannel class]);
  OCMStub([engine settingsChannel]).andReturn(settingsChannel);

  FlutterViewController* realVC = [[FlutterViewController alloc] initWithEngine:engine
                                                                        nibName:nil
                                                                         bundle:nil];
  id mockTraitCollection = [self setupFakeTraitCollectionWithContrast:UIAccessibilityContrastHigh];

  // We partially mock the real FlutterViewController to act as the OS and report
  // the UITraitCollection of our choice. Mocking the object under test is not
  // desirable, but given that the OS does not offer a DI approach to providing
  // our own UITraitCollection, this seems to be the least bad option.
  id partialMockVC = OCMPartialMock(realVC);
  OCMStub([partialMockVC traitCollection]).andReturn(mockTraitCollection);

  // Exercise behavior under test.
  [partialMockVC traitCollectionDidChange:mockTraitCollection];

  // Verify behavior.
  OCMVerify([settingsChannel sendMessage:[OCMArg checkWithBlock:^BOOL(id message) {
                               return [message[@"platformContrast"] isEqualToString:@"high"];
                             }]]);

  // Restore UIUserInterfaceStyle
  [partialMockVC stopMocking];
  [mockTraitCollection stopMocking];
}

- (UITraitCollection*)setupFakeTraitCollectionWithContrast:(UIAccessibilityContrast)contrast {
  id mockTraitCollection = OCMClassMock([UITraitCollection class]);
  OCMStub([mockTraitCollection accessibilityContrast]).andReturn(UIAccessibilityContrastHigh);
  return mockTraitCollection;
}

@end
