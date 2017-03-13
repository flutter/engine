// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/resource_mapping.h"

#include "lib/ftl/build_config.h"

#if OS_MACOSX

#include "flutter/fml/platform/darwin/resource_mapping_darwin.h"
using PlatformResourceMapping = fml::ResourceMappingDarwin;

#elif OS_ANDROID

#include "flutter/fml/platform/android/resource_mapping_android.h"
using PlatformResourceMapping = fml::ResourceMappingAndroid;

#else

#error This platform does not have a resource mapping implementation.

#endif

namespace fml {

std::unique_ptr<ResourceMapping> ResourceMapping::Create(
    const std::string& resource) {
  return std::make_unique<PlatformResourceMapping>(resource);
}

ResourceMapping::ResourceMapping() = default;

}  // namespace fml
