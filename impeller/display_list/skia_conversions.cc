// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_point.h"
#include "flutter/display_list/geometry/dl_rect.h"
#include "impeller/display_list/skia_conversions.h"
#include "third_party/skia/modules/skparagraph/include/Paragraph.h"

namespace impeller {
namespace skia_conversions {

Rect ToRect(const flutter::DlFRect& rect) {
  return Rect::MakeLTRB(rect.left(), rect.top(), rect.right(), rect.bottom());
}

std::optional<Rect> ToRect(const flutter::DlFRect* rect) {
  if (rect == nullptr) {
    return std::nullopt;
  }
  return ToRect(*rect);
}

std::vector<Rect> ToRects(const flutter::DlFRect tex[], int count) {
  auto result = std::vector<Rect>();
  for (int i = 0; i < count; i++) {
    result.push_back(ToRect(tex[i]));
  }
  return result;
}

std::vector<Point> ToPoints(const flutter::DlFPoint points[], int count) {
  std::vector<Point> result(count);
  for (auto i = 0; i < count; i++) {
    result[i] = ToPoint(points[i]);
  }
  return result;
}

PathBuilder::RoundingRadii ToRoundingRadii(const flutter::DlFRRect& rrect) {
  PathBuilder::RoundingRadii radii;
  radii.bottom_left = ToPoint(rrect.lower_left_radii());
  radii.bottom_right = ToPoint(rrect.lower_right_radii());
  radii.top_left = ToPoint(rrect.upper_left_radii());
  radii.top_right = ToPoint(rrect.upper_right_radii());
  return radii;
}

Path ToPath(const flutter::DlPath& dl_path) {
  return ToPath(dl_path.GetSkiaPath());
}

Path ToPath(const SkPath& path) {
  auto iterator = SkPath::Iter(path, false);

  struct PathData {
    union {
      SkPoint points[4];
    };
  };

  PathBuilder builder;
  PathData data;
  auto verb = SkPath::Verb::kDone_Verb;
  do {
    verb = iterator.next(data.points);
    switch (verb) {
      case SkPath::kMove_Verb:
        builder.MoveTo(ToPoint(data.points[0]));
        break;
      case SkPath::kLine_Verb:
        builder.LineTo(ToPoint(data.points[1]));
        break;
      case SkPath::kQuad_Verb:
        builder.QuadraticCurveTo(ToPoint(data.points[1]),
                                 ToPoint(data.points[2]));
        break;
      case SkPath::kConic_Verb: {
        constexpr auto kPow2 = 1;  // Only works for sweeps up to 90 degrees.
        constexpr auto kQuadCount = 1 + (2 * (1 << kPow2));
        SkPoint points[kQuadCount];
        const auto curve_count =
            SkPath::ConvertConicToQuads(data.points[0],          //
                                        data.points[1],          //
                                        data.points[2],          //
                                        iterator.conicWeight(),  //
                                        points,                  //
                                        kPow2                    //
            );

        for (int curve_index = 0, point_index = 0;  //
             curve_index < curve_count;             //
             curve_index++, point_index += 2        //
        ) {
          builder.QuadraticCurveTo(ToPoint(points[point_index + 1]),
                                   ToPoint(points[point_index + 2]));
        }
      } break;
      case SkPath::kCubic_Verb:
        builder.CubicCurveTo(ToPoint(data.points[1]), ToPoint(data.points[2]),
                             ToPoint(data.points[3]));
        break;
      case SkPath::kClose_Verb:
        builder.Close();
        break;
      case SkPath::kDone_Verb:
        break;
    }
  } while (verb != SkPath::Verb::kDone_Verb);

  FillType fill_type;
  switch (path.getFillType()) {
    case SkPathFillType::kWinding:
      fill_type = FillType::kNonZero;
      break;
    case SkPathFillType::kEvenOdd:
      fill_type = FillType::kOdd;
      break;
    case SkPathFillType::kInverseWinding:
    case SkPathFillType::kInverseEvenOdd:
      // Flutter doesn't expose these path fill types. These are only visible
      // via the receiver interface. We should never get here.
      fill_type = FillType::kNonZero;
      break;
  }
  builder.SetConvexity(path.isConvex() ? Convexity::kConvex
                                       : Convexity::kUnknown);
  return builder.TakePath(fill_type);
}

Path ToPath(const flutter::DlFRRect& rrect) {
  return PathBuilder{}
      .AddRoundedRect(ToRect(rrect.Bounds()), ToRoundingRadii(rrect))
      .SetConvexity(Convexity::kConvex)
      .TakePath();
}

Point ToPoint(const SkPoint& point) {
  return Point::MakeXY(point.x(), point.y());
}

Point ToPoint(const flutter::DlFPoint& point) {
  return Point::MakeXY(point.x(), point.y());
}

Color ToColor(const flutter::DlColor& color) {
  return {
      color.getRedF(),    //
      color.getGreenF(),  //
      color.getBlueF(),   //
      color.getAlphaF()   //
  };
}

std::vector<Matrix> ToRSXForms(const flutter::DlRSTransform xform[], int count) {
  auto result = std::vector<Matrix>();
  for (int i = 0; i < count; i++) {
    auto form = xform[i];
    auto scos = form.scaled_cos();
    auto ssin = form.scaled_sin();
    auto tx = form.translate_x();
    auto ty = form.translate_y();
    // clang-format off
    auto matrix = Matrix{
      scos, ssin, 0, 0,
     -ssin, scos, 0, 0,
      0,    0,    1, 0,
      tx,   ty,   0, 1
    };
    // clang-format on
    result.push_back(matrix);
  }
  return result;
}

Path PathDataFromTextBlob(const sk_sp<SkTextBlob>& blob) {
  if (!blob) {
    return {};
  }

  return ToPath(skia::textlayout::Paragraph::GetPath(blob.get()));
}

std::optional<impeller::PixelFormat> ToPixelFormat(SkColorType type) {
  switch (type) {
    case kRGBA_8888_SkColorType:
      return impeller::PixelFormat::kR8G8B8A8UNormInt;
    case kBGRA_8888_SkColorType:
      return impeller::PixelFormat::kB8G8R8A8UNormInt;
    case kRGBA_F16_SkColorType:
      return impeller::PixelFormat::kR16G16B16A16Float;
    case kBGR_101010x_XR_SkColorType:
      return impeller::PixelFormat::kB10G10R10XR;
    default:
      return std::nullopt;
  }
  return std::nullopt;
}

}  // namespace skia_conversions
}  // namespace impeller
