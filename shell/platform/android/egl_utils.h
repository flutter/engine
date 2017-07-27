// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_EGL_UTILS_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_EGL_UTILS_H_

#include <utility>
#include "lib/ftl/macros.h"

namespace shell {

template <class T>
using EGLResult = std::pair<bool, T>;

void LogLastEGLError();

}  // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_EGL_UTILS_H_
