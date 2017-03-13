// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/android/resource_mapping_android.h"
#include "lib/ftl/logging.h"

namespace fml {

ResourceMappingAndroid::ResourceMappingAndroid(const std::string& resource) {
  // FTL_DCHECK(false) << "WIP";
}

ResourceMappingAndroid::~ResourceMappingAndroid() {
  // FTL_DCHECK(false) << "WIP";
}

size_t ResourceMappingAndroid::GetSize() const {
  // FTL_DCHECK(false) << "WIP";
  return 0;
}

const uint8_t* ResourceMappingAndroid::GetMapping() const {
  // FTL_DCHECK(false) << "WIP";
  return nullptr;
}

}  // namespace fml
