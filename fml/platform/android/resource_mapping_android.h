// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FML_PLATFORM_ANDROID_RESOURCE_MAPPING_ANDROID_H_
#define FML_PLATFORM_ANDROID_RESOURCE_MAPPING_ANDROID_H_

#include "flutter/fml/resource_mapping.h"
#include "lib/ftl/macros.h"

namespace fml {

class ResourceMappingAndroid : public ResourceMapping {
 public:
  ResourceMappingAndroid(const std::string& resource);

  ~ResourceMappingAndroid();

  size_t GetSize() const override;

  const uint8_t* GetMapping() const override;

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(ResourceMappingAndroid);
};

}  // namespace fml

#endif  // FML_PLATFORM_ANDROID_RESOURCE_MAPPING_ANDROID_H_
