// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PLATFORM_DARWIN_RESOURCE_MAPPING_DARWIN_H_
#define FLUTTER_FML_PLATFORM_DARWIN_RESOURCE_MAPPING_DARWIN_H_

#include "flutter/fml/file_mapping.h"
#include "flutter/fml/resource_mapping.h"
#include "lib/ftl/macros.h"

namespace fml {

class ResourceMappingDarwin : public ResourceMapping {
 public:
  ResourceMappingDarwin(const std::string& resource);

  ~ResourceMappingDarwin();

  size_t GetSize() const override;

  const uint8_t* GetMapping() const override;

 private:
  FileMapping actual_;

  FTL_DISALLOW_COPY_AND_ASSIGN(ResourceMappingDarwin);
};

}  // namespace fml

#endif  // FLUTTER_FML_PLATFORM_DARWIN_RESOURCE_MAPPING_DARWIN_H_
