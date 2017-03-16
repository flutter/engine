// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/logger/system_logger.h"

#include <android/log.h>
#include "flutter/common/settings.h"

namespace blink {

void SytemLoggerLog(const uint8_t* message, size_t length) {
  __android_log_print(ANDROID_LOG_INFO, Settings::Get().log_tag.c_str(), "%.*s",
                      static_cast<int>(length), chars);
}

}  // namespace blink
