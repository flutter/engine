// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>

#import "flutter/third_party/spring_animation/spring_animation.h"

@interface SpringAnimationTest : XCTestCase

@end

@implementation SpringAnimationTest

- (void)testSpringAnimationInitializeCorrectly {
  SpringAnimation* animation = [[SpringAnimation alloc] initWithStiffness:1000
                                                                  damping:500
                                                                     mass:3
                                                          initialVelocity:0
                                                                fromValue:0
                                                                  toValue:1000];
  XCTAssert(animation.stiffness == 1000);
  XCTAssert(animation.damping == 500);
  XCTAssert(animation.mass == 3);
  XCTAssert(animation.initialVelocity == 0);
  XCTAssert(animation.fromValue == 0);
  XCTAssert(animation.toValue == 1000);
}

- (void)testSpringAnimationCurveFunctionWorksCorrectly {
  // Here is the keyboard curve config on iOS platform.
  SpringAnimation* animation = [[SpringAnimation alloc] initWithStiffness:1000
                                                                  damping:500
                                                                     mass:3
                                                          initialVelocity:0
                                                                fromValue:0
                                                                  toValue:1000];
  const double epsilon = 1.0;
  const double startTime = 0;
  const double endTime = 0.6;

  const double startValue = [animation curveFunction:startTime];
  XCTAssert(fabs(startValue - animation.fromValue) < epsilon);
  const double toValue = [animation curveFunction:endTime];
  XCTAssert(fabs(toValue - animation.toValue) < epsilon);
}

@end
