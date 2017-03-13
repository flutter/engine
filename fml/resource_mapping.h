// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_RESOURCE_MAPPING_H_
#define FLUTTER_FML_RESOURCE_MAPPING_H_

#include <memory>
#include "lib/ftl/macros.h"

namespace fml {

class ResourceMapping {
 public:
  static std::unique_ptr<ResourceMapping> Create(const std::string& resource);

  virtual size_t GetSize() const = 0;

  virtual const uint8_t* GetMapping() const = 0;

 protected:
  ResourceMapping();

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(ResourceMapping);
};

}  // namespace fml

#endif  // FLUTTER_FML_RESOURCE_MAPPING_H_
