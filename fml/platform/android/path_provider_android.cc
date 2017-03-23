// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/android/path_provider_android.h"

namespace fml {
namespace paths {

PathProviderAndroid::PathProviderAndroid() = default;

PathProviderAndroid::~PathProviderAndroid() = default;

std::pair<bool, std::string> PathProviderAndroid::GetPath(PathType type) {
  // None of the current path types make sense for Android.
  return {false, ""};
}

}  // namespace paths
}  // namespace fml
