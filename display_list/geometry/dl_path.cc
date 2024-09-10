// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_path.h"

#include "flutter/display_list/geometry/dl_geometry_types.h"
#include "flutter/impeller/geometry/path_builder.h"

namespace flutter {

const SkPath& DlPath::GetSkPath() const {
  return sk_path_;
}

impeller::Path DlPath::GetPath() const {
  if (path_.IsEmpty() && !sk_path_.isEmpty()) {
    path_ = ConvertToImpellerPath(sk_path_);
  }

  return path_;
}

bool DlPath::IsInverseFillType() const {
  return sk_path_.isInverseFillType();
}

bool DlPath::IsRect(DlRect* rect, bool* is_closed) const {
  return sk_path_.isRect(ToSkRect(rect), is_closed);
}

bool DlPath::IsOval(DlRect* bounds) const {
  return sk_path_.isOval(ToSkRect(bounds));
}

bool DlPath::IsSkRect(SkRect* rect, bool* is_closed) const {
  return sk_path_.isRect(rect, is_closed);
}

bool DlPath::IsSkOval(SkRect* bounds) const {
  return sk_path_.isOval(bounds);
}

bool DlPath::IsSkRRect(SkRRect* rrect) const {
  return sk_path_.isRRect(rrect);
}

SkRect DlPath::GetSkBounds() const {
  return sk_path_.getBounds();
}

DlRect DlPath::GetBounds() const {
  return ToDlRect(sk_path_.getBounds());
}

bool DlPath::operator==(const DlPath& other) const {
  return sk_path_ == other.sk_path_;
}

using Path = impeller::Path;
using PathBuilder = impeller::PathBuilder;
using FillType = impeller::FillType;
using Convexity = impeller::Convexity;

Path DlPath::ConvertToImpellerPath(const SkPath& path, const DlPoint& shift) {
  auto iterator = SkPath::Iter(path, false);

  struct PathData {
    union {
      SkPoint points[4];
    };
  };

  PathBuilder builder;
  PathData data;
  // Reserve a path size with some arbitrarily additional padding.
  builder.Reserve(path.countPoints() + 8, path.countVerbs() + 8);
  auto verb = SkPath::Verb::kDone_Verb;
  do {
    verb = iterator.next(data.points);
    switch (verb) {
      case SkPath::kMove_Verb:
        builder.MoveTo(ToDlPoint(data.points[0]));
        break;
      case SkPath::kLine_Verb:
        builder.LineTo(ToDlPoint(data.points[1]));
        break;
      case SkPath::kQuad_Verb:
        builder.QuadraticCurveTo(ToDlPoint(data.points[1]),
                                 ToDlPoint(data.points[2]));
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
          builder.QuadraticCurveTo(ToDlPoint(points[point_index + 1]),
                                   ToDlPoint(points[point_index + 2]));
        }
      } break;
      case SkPath::kCubic_Verb:
        builder.CubicCurveTo(ToDlPoint(data.points[1]),
                             ToDlPoint(data.points[2]),
                             ToDlPoint(data.points[3]));
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
  builder.Shift(shift);
  auto sk_bounds = path.getBounds().makeOutset(shift.x, shift.y);
  builder.SetBounds(ToDlRect(sk_bounds));
  return builder.TakePath(fill_type);
}

}  // namespace flutter
