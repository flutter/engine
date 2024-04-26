// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/toolkit/wasm/fixtures_store.h"

#include <emscripten.h>

#include <string>

#include "impeller/base/strings.h"

namespace impeller::wasm {

static std::vector<std::string> kPreloadedAssets = {
    "table_mountain_nx.png", "table_mountain_ny.png", "table_mountain_nz.png",
    "table_mountain_px.png", "table_mountain_py.png", "table_mountain_pz.png",
    "blue_noise.png",
};

FixturesStore::FixturesStore(PreloadCompletionCallback callback)
    : preload_completion_callback_(std::move(callback)) {
  for (const auto& asset : kPreloadedAssets) {
    PreloadMapping(asset.c_str());
  }
}

FixturesStore::~FixturesStore() = default;

std::shared_ptr<fml::Mapping> FixturesStore::GetMapping(
    const char* fixture_name) const {
  if (fixture_name == nullptr) {
    return nullptr;
  }
  auto found = assets_.find(fixture_name);
  if (found == assets_.end()) {
    FML_LOG(ERROR) << "Asset not found in asset store: " << fixture_name;
    return nullptr;
  }
  return found->second;
}

void FixturesStore::PreloadMapping(const char* fixture_name) const {
  if (fixture_name == nullptr) {
    return;
  }

  struct Captures {
    FixturesStore* store = nullptr;
    std::string asset_name;
  };

  auto captures = std::make_unique<Captures>();
  captures->store = const_cast<FixturesStore*>(this);
  captures->asset_name = fixture_name;

  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = [](emscripten_fetch_t* fetch) {
    auto captures = reinterpret_cast<Captures*>(fetch->userData);
    captures->store->OnPreloadDone(captures->asset_name, fetch, true);
  };
  attr.onerror = [](emscripten_fetch_t* fetch) {
    auto captures = reinterpret_cast<Captures*>(fetch->userData);
    captures->store->OnPreloadDone(captures->asset_name, fetch, false);
  };
  attr.userData = captures.release();
  emscripten_fetch(
      &attr,
      SPrintF("gen/flutter/impeller/fixtures/assets/%s", fixture_name).c_str());
}

void FixturesStore::OnPreloadDone(const std::string& fixture_name,
                                  emscripten_fetch_t* fetch,
                                  bool success) {
  auto mapping = std::make_shared<fml::NonOwnedMapping>(
      reinterpret_cast<const uint8_t*>(fetch->data),          //
      fetch->numBytes,                                        //
      [fetch](auto, auto) { emscripten_fetch_close(fetch); }  //
  );
  if (success) {
    assets_[fixture_name] = std::move(mapping);
  }
  if (++preload_count_ == kPreloadedAssets.size() &&
      preload_completion_callback_) {
    preload_completion_callback_();
  }
}

static std::shared_ptr<FixturesStore> gDefaultStore;
void SetDefaultStore(std::shared_ptr<FixturesStore> store) {
  gDefaultStore = std::move(store);
}

const std::shared_ptr<FixturesStore>& GetDefaultStore() {
  return gDefaultStore;
}

}  // namespace impeller::wasm
