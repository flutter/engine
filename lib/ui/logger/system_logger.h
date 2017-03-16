// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_LOGGER_SYSTEM_LOGGER_H_
#define FLUTTER_LIB_UI_LOGGER_SYSTEM_LOGGER_H_

#include <cstddef>
#include <cstdint>

namespace blink {

void SytemLoggerLog(const uint8_t* message, size_t length);

}  // namespace blink

#endif  // FLUTTER_LIB_UI_LOGGER_SYSTEM_LOGGER_H_
