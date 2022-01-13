// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_mapping.h"
#include "flutter/shell/platform/embedder/embedder_struct_macros.h"

namespace flutter {

std::unique_ptr<fml::Mapping> CreateEmbedderMapping(
    const FlutterMappingCreateInfo* mapping) {
  auto user_data = SAFE_ACCESS(mapping, user_data, nullptr);
  auto destruction_callback =
      SAFE_ACCESS(mapping, destruction_callback, nullptr);
  return std::make_unique<fml::NonOwnedMapping>(
      SAFE_ACCESS(mapping, data, nullptr), SAFE_ACCESS(mapping, data_size, 0),
      [user_data, destruction_callback](const uint8_t* data, size_t size) {
        if (destruction_callback) {
          destruction_callback(data, size, user_data);
        }
      });
}

}  // namespace flutter
