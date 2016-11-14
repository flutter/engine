// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/picture_layer.h"

#include <utility>

#include "flutter/flow/raster_cache.h"
#include "lib/ftl/logging.h"

namespace flow {

PictureLayer::PictureLayer() : picture_bounds_(SkRect::MakeEmpty()) {}

PictureLayer::~PictureLayer() {}

void PictureLayer::set_picture(sk_sp<SkPicture> picture,
                               SkRect picture_bounds) {
  picture_ = std::move(picture);
  picture_bounds_ = picture_bounds;
}

void PictureLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  if (auto cache = context->raster_cache) {
    cached_image_ =
        cache->GetPrerolledImage(context->gr_context,  // context
                                 picture_.get(),       // picture to rasterize
                                 picture_bounds_,      // bounds of the picture
                                 matrix, is_complex_, will_change_);
  }

  context->child_paint_bounds =
      picture_->cullRect().makeOffset(offset_.x(), offset_.y());
}

void PictureLayer::Paint(PaintContext& context) {
  FTL_DCHECK(picture_);

  TRACE_EVENT1("flutter", "PictureLayer::Paint", "image",
               cached_image_ ? "prerolled" : "normal");

  SkAutoCanvasRestore save(&context.canvas, true);
  context.canvas.translate(offset_.x(), offset_.y());

  if (cached_image_) {
    SkImage* image = cached_image_->image().get();
    const SkRect& image_bounds = cached_image_->bounds();
    context.canvas.drawImageRect(
        image,  // image to draw
        SkRect::MakeWH(image_bounds.width(),
                       image_bounds.height()),  // source rect
        image_bounds,                           // dest rect
        nullptr,                                // paint
        SkCanvas::kFast_SrcRectConstraint       // constraint
        );
  } else {
    context.canvas.drawPicture(picture_.get());
  }
}

}  // namespace flow
