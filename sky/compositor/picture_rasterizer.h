// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_COMPOSITOR_RASTER_TASK_H_
#define SKY_COMPOSITOR_RASTER_TASK_H_

#include "base/macros.h"
#include "sky/compositor/layer.h"
#include "third_party/skia/include/core/SkMultiPictureDraw.h"

namespace sky {
class PictureTable;

class PictureRasterizer {
 public:
  explicit PictureRasterizer(PictureTable* picture_table);
  ~PictureRasterizer();

  void Visit(PictureLayer* layer);
  void Rasterize();

 private:
  PictureTable* picture_table_;
  SkMultiPictureDraw drawer_;

  DISALLOW_COPY_AND_ASSIGN(PictureRasterizer);
};

}  // namespace sky

#endif  // SKY_COMPOSITOR_RASTER_TASK_H_
