// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/display_list/dl_color.h"
#include "flutter/display_list/geometry/dl_path.h"
#include "flutter/display_list/geometry/dl_point.h"
#include "flutter/display_list/geometry/dl_rect.h"
#include "flutter/display_list/geometry/dl_round_rect.h"
#include "flutter/display_list/geometry/dl_rstransform.h"
#include "impeller/core/formats.h"
#include "impeller/geometry/color.h"
#include "impeller/geometry/path.h"
#include "impeller/geometry/path_builder.h"
#include "impeller/geometry/point.h"
#include "impeller/geometry/rect.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/core/SkTextBlob.h"

namespace impeller {
namespace skia_conversions {

Rect ToRect(const flutter::DlFRect& rect);

std::optional<Rect> ToRect(const flutter::DlFRect* rect);

std::vector<Rect> ToRects(const flutter::DlFRect tex[], int count);

std::vector<Point> ToPoints(const flutter::DlFPoint points[], int count);

Point ToPoint(const SkPoint& point);

Point ToPoint(const flutter::DlFPoint& point);

Color ToColor(const flutter::DlColor& color);

std::vector<Matrix> ToRSXForms(const flutter::DlRSTransform xform[], int count);

PathBuilder::RoundingRadii ToRoundingRadii(const flutter::DlFRRect& rrect);

Path ToPath(const SkPath& path);

Path ToPath(const flutter::DlPath& path);

Path ToPath(const flutter::DlFRRect& rrect);

Path PathDataFromTextBlob(const sk_sp<SkTextBlob>& blob);

std::optional<impeller::PixelFormat> ToPixelFormat(SkColorType type);

}  // namespace skia_conversions
}  // namespace impeller
