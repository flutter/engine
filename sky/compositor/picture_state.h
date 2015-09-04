// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_COMPOSITOR_PICTURE_STATE_H_
#define SKY_COMPOSITOR_PICTURE_STATE_H_

#include "base/macros.h"
#include "sky/compositor/saturating_counter.h"
#include "sky/engine/wtf/PassRefPtr.h"
#include "sky/engine/wtf/RefPtr.h"
#include "third_party/skia/include/core/SkImage.h"

namespace sky {

class PictureState {
 public:
  PictureState();
  ~PictureState();

  void Mark();
  void Sweep();

  bool ShouldRasterize();

  void set_cached_image(PassRefPtr<SkImage> image) { cached_image_ = image; }
  SkImage* cached_image() const { return cached_image_.get(); }

 private:
  bool is_marked_;
  SaturatingCounter mark_counter_;
  RefPtr<SkImage> cached_image_;

  DISALLOW_COPY_AND_ASSIGN(PictureState);
};

}  // namespace sky

#endif  // SKY_COMPOSITOR_PICTURE_STATE_H_
