// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/spring_curve_ios.h"

#include <Foundation/Foundation.h>

@implementation KeyboardSpringCurve
- (instancetype)initWithStiffness:(double)stiffness
                damping:(double)damping
                   mass:(double)mass
        initialVelocity:(double)initialVelocity
       settlingDuration:(double)settlingDuration {
  self = [super init];
  if (self) {
    _dampingRatio = 1;
    _initialVelocity = initialVelocity;
    _settlingDuration = settlingDuration;

    double response = MAX(1e-5, 2 * M_PI / sqrt(stiffness / mass));
    _omega = 2 * M_PI / response;
  }
  return self;
}

- (double)curveFunc:(double)t {
  double v0 = self.initialVelocity;
  double zeta = self.dampingRatio;

  double y;
  if (abs(zeta - 1.0) < 1e-8) {
    double c1 = -1.0;
    double c2 = v0 - self.omega;
    y = (c1 + c2 * t) * exp(-self.omega * t);
  } else if (zeta > 1) {
    double s1 = self.omega * (-zeta + sqrt(zeta * zeta - 1));
    double s2 = self.omega * (-zeta - sqrt(zeta * zeta - 1));
    double c1 = (-s2 - v0) / (s2 - s1);
    double c2 = (s1 + v0) / (s2 - s1);
    y = c1 * exp(s1 * t) + c2 * exp(s2 * t);
  } else {
    double a = -self.omega * zeta;
    double b = self.omega * sqrt(1 - zeta * zeta);
    double c2 = (v0 + a) / b;
    double theta = atan(c2);
    // Alternatively y = (-cos(b * t) + c2 * sin(b * t)) * exp(a * t)
    y = sqrt(1 + c2 * c2) * exp(a * t) * cos(b * t + theta + M_PI);
  }

  return y + 1;
}
@end
