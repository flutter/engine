// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_SCREENSHOT_PROVIDER_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_SCREENSHOT_PROVIDER_H_

#import <UIKit/UIKit.h>
#include "flutter/flow/embedded_views.h"

namespace flutter {

class IOSScreenShotProvider : public PlatformScreenShotProvider {
 protected:
  sk_sp<SkImage> TakeScreenShotForView(UIView* view);
};

}
#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_SCREENSHOT_PROVIDER_H_
