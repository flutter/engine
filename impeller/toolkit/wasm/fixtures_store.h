// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include <emscripten/fetch.h>

#include "flutter/fml/mapping.h"

namespace impeller::wasm {

class FixturesStore {
 public:
  using PreloadCompletionCallback = std::function<void(void)>;
  explicit FixturesStore(PreloadCompletionCallback callback);

  ~FixturesStore();

  FixturesStore(const FixturesStore&) = delete;

  FixturesStore& operator=(const FixturesStore&) = delete;

  std::shared_ptr<fml::Mapping> GetMapping(const char* fixture_name) const;

 private:
  PreloadCompletionCallback preload_completion_callback_;
  size_t preload_count_ = 0;
  std::map<std::string, std::shared_ptr<fml::Mapping>> assets_;

  void PreloadMapping(const char* fixture_name) const;

  void OnPreloadDone(const std::string& fixture_name,
                     emscripten_fetch_t* fetch,
                     bool success);
};

void SetDefaultStore(std::shared_ptr<FixturesStore> store);

const std::shared_ptr<FixturesStore>& GetDefaultStore();

}  // namespace impeller::wasm
