// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_SCREENSHOT_PROVIDER_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_SCREENSHOT_PROVIDER_H_

#import <UIKit/UIKit.h>
#include "flutter/flow/embedded_views.h"

namespace flutter {

// Utility to provide screenshots on iOS.
class IOSScreenShotProvider {
 public:
  // Take a screenshot for the `view` and return an SkImage.
  //
  // Has to be run a the platform thread.
  static sk_sp<SkImage> TakeScreenShotForView(UIView* view);
};

}
#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_SCREENSHOT_PROVIDER_H_
