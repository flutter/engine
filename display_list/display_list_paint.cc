// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_paint.h"

namespace flutter {

DlPaint::DlPaint()
    : blendMode_(static_cast<unsigned>(DlBlendMode::kDefaultMode)),
      drawStyle_(static_cast<unsigned>(DlDrawStyle::kDefaultStyle)),
      strokeCap_(static_cast<unsigned>(DlStrokeCap::kDefaultCap)),
      strokeJoin_(static_cast<unsigned>(DlStrokeJoin::kDefaultJoin)),
      isAntiAlias_(false),
      isDither_(false),
      isInvertColors_(false),
      strokeWidth_(0.0),
      strokeMiter_(4.0) {}

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

}  // namespace flutter
