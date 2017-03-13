// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/icu_util.h"
#include <memory>
#include <mutex>
#include "flutter/fml/file_mapping.h"
#include "flutter/fml/resource_mapping.h"
#include "lib/ftl/logging.h"
#include "third_party/icu/source/common/unicode/udata.h"

namespace fml {
namespace icu {

static const char kIcuDataFileName[] = "icudtl.dat";

class ICUContext {
 public:
  ICUContext(const std::string& icu_data_path)
      : valid_(SetupMapping(icu_data_path) && SetupICU()) {}

  ~ICUContext() = default;

  bool SetupMapping(const std::string& icu_data_path) {
    // Check if the explicit path specified exists.
    auto overriden_path_mapping = std::make_unique<FileMapping>(icu_data_path);
    if (overriden_path_mapping->GetSize() != 0) {
      file_mapping_ = std::move(overriden_path_mapping);
      return true;
    }

    // Check to see if the mapping is in the resources bundle.
    auto resource = ResourceMapping::Create(kIcuDataFileName);
    if (resource != nullptr && resource->GetSize() != 0) {
      resource_mapping_ = std::move(resource);
      return true;
    }

    // Check if the mapping can by directly accessed via a file path. In this
    // case, the data file needs to be next to the executable.
    auto file = std::make_unique<FileMapping>(kIcuDataFileName);
    if (file->GetSize() != 0) {
      file_mapping_ = std::move(file);
      return true;
    }

    return false;
  }

  bool SetupICU() {
    if (GetSize() == 0) {
      return false;
    }

    UErrorCode err_code = U_ZERO_ERROR;
    udata_setCommonData(GetMapping(), &err_code);
    return (err_code == U_ZERO_ERROR);
  }

  const uint8_t* GetMapping() const {
    if (resource_mapping_ != nullptr) {
      return resource_mapping_->GetMapping();
    }

    if (file_mapping_ != nullptr) {
      return file_mapping_->GetMapping();
    }

    return nullptr;
  }

  size_t GetSize() const {
    if (resource_mapping_ != nullptr) {
      return resource_mapping_->GetSize();
    }

    if (file_mapping_ != nullptr) {
      return file_mapping_->GetSize();
    }

    return 0;
  }

  bool IsValid() const { return valid_; }

 private:
  bool valid_;
  std::unique_ptr<ResourceMapping> resource_mapping_;
  std::unique_ptr<FileMapping> file_mapping_;

  FTL_DISALLOW_COPY_AND_ASSIGN(ICUContext);
};

void InitializeICUOnce(const std::string& icu_data_path) {
  return;
  static ICUContext* context = new ICUContext(icu_data_path);
  FTL_CHECK(context->IsValid()) << "Must be able to initialize the ICU context";
}

std::once_flag gICUInitFlag;
void InitializeICU(const std::string& icu_data_path) {
  std::call_once(gICUInitFlag,
                 [&icu_data_path]() { InitializeICUOnce(icu_data_path); });
}

}  // namespace icu
}  // namespace fml
