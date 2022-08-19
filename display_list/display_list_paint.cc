// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_paint.h"

#include "flutter/display_list/display_list_utils.h"

namespace flutter {

DlPaint::DlPaint()
    : blendMode_(static_cast<unsigned>(DlBlendMode::kDefaultMode)),
      drawStyle_(static_cast<unsigned>(DlDrawStyle::kDefaultStyle)),
      strokeCap_(static_cast<unsigned>(DlStrokeCap::kDefaultCap)),
      strokeJoin_(static_cast<unsigned>(DlStrokeJoin::kDefaultJoin)),
      isAntiAlias_(false),
      isDither_(false),
      isInvertColors_(false),
      strokeWidth_(kDefaultWidth),
      strokeMiter_(kDefaultMiter) {}

bool DlPaint::operator==(DlPaint const& other) const {
  return blendMode_ == other.blendMode_ &&            //
         drawStyle_ == other.drawStyle_ &&            //
         strokeCap_ == other.strokeCap_ &&            //
         strokeJoin_ == other.strokeJoin_ &&          //
         isAntiAlias_ == other.isAntiAlias_ &&        //
         isDither_ == other.isDither_ &&              //
         isInvertColors_ == other.isInvertColors_ &&  //
         color_ == other.color_ &&                    //
         strokeWidth_ == other.strokeWidth_ &&        //
         strokeMiter_ == other.strokeMiter_ &&        //
         Equals(colorSource_, other.colorSource_) &&  //
         Equals(colorFilter_, other.colorFilter_) &&  //
         Equals(imageFilter_, other.imageFilter_) &&  //
         Equals(maskFilter_, other.maskFilter_);
}

void DlPaint::toSkPaint(SkPaint& paint) const {
  paint.setAntiAlias(isAntiAlias_);
  paint.setDither(isDither_);
  paint.setColor(color_);
  paint.setBlendMode(getSkBlendMode());
  paint.setStyle(getSkDrawStyle());
  paint.setStrokeCap(getSkStrokeCap());
  paint.setStrokeJoin(getSkStrokeJoin());
  paint.setStrokeWidth(strokeWidth_);
  paint.setStrokeMiter(strokeMiter_);
  paint.setShader(colorSource_ ? colorSource_->skia_object() : nullptr);
  paint.setColorFilter(SkPaintDispatchHelper::MakeColorFilter(
      isInvertColors_, colorFilter_.get()));
  paint.setImageFilter(imageFilter_->skia_object());
  paint.setMaskFilter(maskFilter_->skia_object());
}

}  // namespace flutter
