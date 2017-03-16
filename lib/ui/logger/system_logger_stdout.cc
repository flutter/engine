// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/logger/system_logger.h"

#include <stdio.h>

namespace blink {

void SytemLoggerLog(const uint8_t* message, size_t length) {
  fwrite(message, 1, length, stdout);
  fputs("\n", stdout);
  fflush(stdout);
}

}  // namespace blink
