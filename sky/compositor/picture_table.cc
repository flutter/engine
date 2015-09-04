// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/compositor/picture_table.h"

namespace sky {

PictureTable::PictureTable() {
}

PictureTable::~PictureTable() {
}

PictureState* PictureTable::GetAndMarkState(SkPicture* picture) {
  auto it = table_.emplace(picture->uniqueID(), nullptr);
  std::unique_ptr<PictureState>& state = it.first->second;
  bool added = it.second;
  if (added)
    state.reset(new PictureState());
  state->Mark();
  return state.get();
}

void PictureTable::Sweep() {
  for (auto& entry : table_)
    entry.second->Sweep();
}

}  // namespace sky
