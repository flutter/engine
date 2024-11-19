// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/effects/dl_image_filter.h"

#include "flutter/display_list/effects/dl_local_matrix_image_filter.h"

namespace flutter {

DlVector2 DlImageFilter::map_vectors_affine(const DlMatrix& ctm,
                                            DlScalar x,
                                            DlScalar y) {
  FML_DCHECK(std::isfinite(x) && x >= 0);
  FML_DCHECK(std::isfinite(y) && y >= 0);
  FML_DCHECK(ctm.IsFinite() && !ctm.HasPerspective2D());

  // The x and y scalars would have been used to expand a local space
  // rectangle which is then transformed by ctm. In order to do the
  // expansion correctly, we should look at the relevant math. The
  // 4 corners will be moved outward by the following vectors:
  //     (UL,UR,LR,LL) = ((-x, -y), (+x, -y), (+x, +y), (-x, +y))
  // After applying the transform, each of these vectors could be
  // pointing in any direction so we need to examine each transformed
  // delta vector and how it affected the bounds.
  // Looking at just the affine 2x3 entries of the CTM we can delta
  // transform these corner offsets and get the following:
  //     UL = dCTM(-x, -y) = (- x*m00 - y*m01, - x*m10 - y*m11)
  //     UR = dCTM(+x, -y) = (  x*m00 - y*m01,   x*m10 - y*m11)
  //     LR = dCTM(+x, +y) = (  x*m00 + y*m01,   x*m10 + y*m11)
  //     LL = dCTM(-x, +y) = (- x*m00 + y*m01, - x*m10 + y*m11)
  // The X vectors are all some variation of adding or subtracting
  // the sum of x*m00 and y*m01 or their difference. Similarly the Y
  // vectors are +/- the associated sum/difference of x*m10 and y*m11.
  // The largest displacements, both left/right or up/down, will
  // happen when the signs of the m00/m01/m10/m11 matrix entries
  // coincide with the signs of the scalars, i.e. are all positive.
  return {x * abs(ctm.m[0]) + y * abs(ctm.m[4]),
          x * abs(ctm.m[1]) + y * abs(ctm.m[5])};
}

DlIRect* DlImageFilter::inset_device_bounds(const DlIRect& input_bounds,
                                            DlScalar radius_x,
                                            DlScalar radius_y,
                                            const DlMatrix& ctm,
                                            DlIRect& output_bounds) {
  if (ctm.IsFinite() && ctm.IsInvertible()) {
    if (ctm.HasPerspective2D()) {
      DlRect local_bounds = DlRect::Make(input_bounds)
                                .TransformAndClipBounds(ctm.Invert())
                                .Expand(-radius_x, -radius_y);
      output_bounds =
          DlIRect::RoundOut(local_bounds.TransformAndClipBounds(ctm));
      return &output_bounds;
    } else {
      DlVector2 device_radius = map_vectors_affine(ctm, radius_x, radius_y);
      output_bounds =
          input_bounds.Expand(-floor(device_radius.x), -floor(device_radius.y));
      return &output_bounds;
    }
  }
  output_bounds = input_bounds;
  return nullptr;
}

DlIRect* DlImageFilter::outset_device_bounds(const DlIRect& input_bounds,
                                             DlScalar radius_x,
                                             DlScalar radius_y,
                                             const DlMatrix& ctm,
                                             DlIRect& output_bounds) {
  if (ctm.IsFinite() && ctm.IsInvertible()) {
    if (ctm.HasPerspective2D()) {
      DlRect local_bounds = DlRect::Make(input_bounds)
                                .TransformAndClipBounds(ctm.Invert())
                                .Expand(radius_x, radius_y);
      output_bounds =
          DlIRect::RoundOut(local_bounds.TransformAndClipBounds(ctm));
      return &output_bounds;
    } else {
      DlVector2 device_radius = map_vectors_affine(ctm, radius_x, radius_y);
      output_bounds =
          input_bounds.Expand(ceil(device_radius.x), ceil(device_radius.y));
      return &output_bounds;
    }
  }
  output_bounds = input_bounds;
  return nullptr;
}

std::shared_ptr<DlImageFilter> DlImageFilter::makeWithLocalMatrix(
    const DlMatrix& matrix) const {
  if (matrix.IsIdentity()) {
    return shared();
  }
  // Matrix
  switch (this->matrix_capability()) {
    case MatrixCapability::kTranslate: {
      if (!matrix.IsTranslationOnly()) {
        // Nothing we can do at this point
        return nullptr;
      }
      break;
    }
    case MatrixCapability::kScaleTranslate: {
      if (!matrix.IsTranslationScaleOnly()) {
        // Nothing we can do at this point
        return nullptr;
      }
      break;
    }
    default:
      break;
  }
  return DlLocalMatrixImageFilter::Make(matrix, shared());
}

}  // namespace flutter
