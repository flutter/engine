// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_PLATFORM_SERVICE_H_
#define FLUTTER_SHELL_COMMON_PLATFORM_SERVICE_H_

#include <functional>
#include <string>

namespace shell {

class PlatformService {
 public:
  virtual ~PlatformService() {}

  virtual void Process(std::string data,
                       std::function<void(std::string)> callback) = 0;
};

}  // namespace shell

#endif  // FLUTTER_SHELL_COMMON_PLATFORM_SERVICE_H_
