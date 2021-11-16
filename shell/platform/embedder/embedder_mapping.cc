// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_mapping.h"
#include "flutter/shell/platform/embedder/embedder_struct_macros.h"

namespace flutter {

class EmbedderMapping final : public fml::Mapping {
  public:
  EmbedderMapping(const uint8_t* data,
                  size_t size,
                  void* user_data,
                  VoidCallback destruction_callback)
      : data_(data),
        size_(size),
        user_data_(user_data),
        destruction_callback_(destruction_callback) {}
  
  ~EmbedderMapping() override {
    if (destruction_callback_) destruction_callback_(user_data_);
  }

  // |Mapping|
  size_t GetSize() const override {
    return size_;
  }

  // |Mapping|
  const uint8_t* GetMapping() const override {
    return data_;
  }

  // |Mapping|
  bool IsDontNeedSafe() const override {
    return false;
  }

private:
  const uint8_t* data_;
  size_t size_;
  void* user_data_;
  VoidCallback destruction_callback_;

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderMapping);
};

std::unique_ptr<fml::Mapping> CreateEmbedderMapping(const FlutterEngineMappingCreateInfo* mapping) {
  return std::make_unique<EmbedderMapping>(
    SAFE_ACCESS(mapping, data, nullptr),
    SAFE_ACCESS(mapping, data_size, 0),
    SAFE_ACCESS(mapping, user_data, nullptr),
    SAFE_ACCESS(mapping, destruction_callback, nullptr)
  );
}

}  // namespace flutter
