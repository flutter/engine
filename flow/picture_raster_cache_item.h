// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_PICTURE_RASTER_CACHE_ITEM_H_
#define FLUTTER_FLOW_PICTURE_RASTER_CACHE_ITEM_H_

#include <memory>
#include <optional>

#include "flutter/display_list/display_list_utils.h"
#include "flutter/flow/embedded_views.h"
#include "flutter/flow/raster_cache_item.h"
#include "flutter/flow/raster_cache_util.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"

namespace flutter {

class DisplayList;

class SkPictureRasterCacheItem : public RasterCacheItem {
 public:
  SkPictureRasterCacheItem(SkPicture* sk_picture,
                           const SkPoint& offset,
                           bool is_complex = true,
                           bool will_change = false);

  void PrerollSetup(PrerollContext* context, const SkMatrix& matrix) override;

  void PrerollFinalize(PrerollContext* context,
                       const SkMatrix& matrix) override;

  bool Draw(const PaintContext& context, const SkPaint* paint) const override;

  bool Draw(const PaintContext& context,
            SkCanvas* canvas,
            const SkPaint* paint) const override;

  bool TryToPrepareRasterCache(const PaintContext& context) const override;

  void ModifyMatrix(SkPoint offset) const {
    matrix_ = matrix_.preTranslate(offset.x(), offset.y());
  }

  const SkPicture* sk_picture() const { return sk_picture_; }

 private:
  SkMatrix transformation_matrix_;
  SkPicture* sk_picture_;
  SkPoint offset_;
  bool is_complex_;
  bool will_change_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_PICTURE_RASTER_CACHE_ITEM_H_
