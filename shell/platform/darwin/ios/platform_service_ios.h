// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_PLATFORM_SERVICE_IOS_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_PLATFORM_SERVICE_IOS_H_

#include "flutter/shell/common/platform_service.h"

namespace shell {

class PlatformServiceIOS : public PlatformService {
 public:
  PlatformServiceIOS();
  ~PlatformServiceIOS() final;

 private:
  // PlatformService implementation.
  void Process(std::string data,
               std::function<void(std::string)> callback) final;
};

}  // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_PLATFORM_SERVICE_IOS_H_
