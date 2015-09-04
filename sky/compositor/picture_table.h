// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_COMPOSITOR_PICTURE_TABLE_H_
#define SKY_COMPOSITOR_PICTURE_TABLE_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "sky/compositor/picture_state.h"

namespace sky {

class PictureTable {
 public:
  PictureTable();
  ~PictureTable();

  PictureState* GetAndMarkState(SkPicture* picture);
  void Sweep();

 private:
  std::unordered_map<uint32_t, std::unique_ptr<PictureState>> table_;

  DISALLOW_COPY_AND_ASSIGN(PictureTable);
};

}  // namespace sky

#endif  // SKY_COMPOSITOR_PICTURE_TABLE_H_
