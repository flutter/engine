// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/transform_layer.h"

#include <optional>

namespace flutter {

TransformLayer::TransformLayer(const SkM44& transform) : transform_(transform) {
  // Checks (in some degree) that SkM44 transform_ is valid and initialized.
  //
  // If transform_ is uninitialized, this assert may look flaky as it doesn't
  // fail all the time, and some rerun may make it pass. But don't ignore it and
  // just rerun the test if this is triggered, since even a flaky failure here
  // may signify a potentially big problem in the code.
  //
  // We have to write this flaky test because there is no reliable way to test
  // whether a variable is initialized or not in C++.
  FML_DCHECK(transform_.isFinite());
  if (!transform_.isFinite()) {
    FML_LOG(ERROR) << "TransformLayer is constructed with an invalid matrix.";
    transform_.setIdentity();
  }
}

void TransformLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  auto* prev = static_cast<const TransformLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    FML_DCHECK(prev);
    if (transform_ != prev->transform_) {
      context->MarkSubtreeDirty(context->GetOldLayerPaintRegion(old_layer));
    }
  }
  context->PushTransform(transform_);
  DiffChildren(context, prev);
  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

static SkRect map_rect_affine(const SkRect& src, const float mat[16]) {
  // When multiplied against vectors of the form <x,y,x,y>, 'flip' allows a
  // single min() to compute both the min and "negated" max between the xy
  // coordinates. Once finished, another multiplication produces the original
  // max.
  const skvx::float4 flip{1.f, 1.f, -1.f, -1.f};

  // Since z = 0 and it's assumed ther's no perspective, only load the upper 2x2
  // and (tx,ty) in c3
  auto c0 = skvx::shuffle<0, 1, 0, 1>(skvx::float2::Load(mat + 0)) * flip;
  auto c1 = skvx::shuffle<0, 1, 0, 1>(skvx::float2::Load(mat + 4)) * flip;
  auto c3 = skvx::shuffle<0, 1, 0, 1>(skvx::float2::Load(mat + 12));

  // Compute the min and max of the four transformed corners pre-translation;
  // then translate once at the end.
  auto minMax = c3 + flip * min(min(c0 * src.fLeft + c1 * src.fTop,
                                    c0 * src.fRight + c1 * src.fTop),
                                min(c0 * src.fLeft + c1 * src.fBottom,
                                    c0 * src.fRight + c1 * src.fBottom));

  // minMax holds (min x, min y, max x, max y) so can be copied into an SkRect
  // expecting l,t,r,b
  SkRect r;
  minMax.store(&r);
  return r;
}

static SkRect map_rect_affine(const SkRect& src, const float mat[16]) {
  // When multiplied against vectors of the form <x,y,x,y>, 'flip' allows a
  // single min() to compute both the min and "negated" max between the xy
  // coordinates. Once finished, another multiplication produces the original
  // max.
  const skvx::float4 flip{1.f, 1.f, -1.f, -1.f};

  // Since z = 0 and it's assumed ther's no perspective, only load the upper 2x2
  // and (tx,ty) in c3
  auto c0 = skvx::shuffle<0, 1, 0, 1>(skvx::float2::Load(mat + 0)) * flip;
  auto c1 = skvx::shuffle<0, 1, 0, 1>(skvx::float2::Load(mat + 4)) * flip;
  auto c3 = skvx::shuffle<0, 1, 0, 1>(skvx::float2::Load(mat + 12));

  // Compute the min and max of the four transformed corners pre-translation;
  // then translate once at the end.
  auto minMax = c3 + flip * min(min(c0 * src.fLeft + c1 * src.fTop,
                                    c0 * src.fRight + c1 * src.fTop),
                                min(c0 * src.fLeft + c1 * src.fBottom,
                                    c0 * src.fRight + c1 * src.fBottom));

  // minMax holds (min x, min y, max x, max y) so can be copied into an SkRect
  // expecting l,t,r,b
  SkRect r;
  minMax.store(&r);
  return r;
}

static SkRect map_rect_perspective(const SkRect& src, const float mat[16]) {
  // Like map_rect_affine, z = 0 so we can skip the 3rd column, but we do need
  // to compute w's for each corner of the src rect.
  auto c0 = skvx::float4::Load(mat + 0);
  auto c1 = skvx::float4::Load(mat + 4);
  auto c3 = skvx::float4::Load(mat + 12);

  // Unlike map_rect_affine, we do not defer the 4th column since we may need to
  // homogeneous coordinates to clip against the w=0 plane
  auto tl = c0 * src.fLeft + c1 * src.fTop + c3;
  auto tr = c0 * src.fRight + c1 * src.fTop + c3;
  auto bl = c0 * src.fLeft + c1 * src.fBottom + c3;
  auto br = c0 * src.fRight + c1 * src.fBottom + c3;

  // After clipping to w>0 and projecting to 2d, 'project' employs the same
  // negation trick to compute min and max at the same time.
  const skvx::float4 flip{1.f, 1.f, -1.f, -1.f};
  auto project = [&flip](const skvx::float4& p0, const skvx::float4& p1,
                         const skvx::float4& p2) {
    float w0 = p0[3];
    if (w0 >= SkPathPriv::kW0PlaneDistance) {
      // Unclipped, just divide by w
      return flip * skvx::shuffle<0, 1, 0, 1>(p0) / w0;
    } else {
      auto clip = [&](const skvx::float4& p) {
        float w = p[3];
        if (w >= SkPathPriv::kW0PlaneDistance) {
          float t = (SkPathPriv::kW0PlaneDistance - w0) / (w - w0);
          auto c = (t * skvx::shuffle<0, 1>(p) +
                    (1.f - t) * skvx::shuffle<0, 1>(p0)) /
                   SkPathPriv::kW0PlaneDistance;

          return flip * skvx::shuffle<0, 1, 0, 1>(c);
        } else {
          return skvx::float4(SK_ScalarInfinity);
        }
      };
      // Clip both edges leaving p0, and return the min/max of the two clipped
      // points (since clip returns infinity when both p0 and 2nd vertex have
      // w<0, it'll automatically be ignored).
      return min(clip(p1), clip(p2));
    }
  };

  // Project all 4 corners, and pass in their adjacent vertices for clipping if
  // it has w < 0, then accumulate the min and max xy's.
  auto minMax = flip * min(min(project(tl, tr, bl), project(tr, br, tl)),
                           min(project(br, bl, tr), project(bl, tl, br)));

  SkRect r;
  minMax.store(&r);
  return r;
}

static SkRect MapRect(const SkM44& m, const SkRect& src) {
  const bool hasPerspective =
      m.fMat[3] != 0 || m.fMat[7] != 0 || m.fMat[11] != 0 || m.fMat[15] != 1;
  if (hasPerspective) {
    return map_rect_perspective(src, m.fMat);
  } else {
    return map_rect_affine(src, m.fMat);
  }
}

void TransformLayer::Preroll(PrerollContext* context) {
  auto mutator = context->state_stack.save();
  mutator.transform(transform_);

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, &child_paint_bounds);
  MapRect(transform_, child_paint_bounds);
  set_paint_bounds(child_paint_bounds);
}

void TransformLayer::Paint(PaintContext& context) const {
  FML_DCHECK(needs_painting(context));

  auto mutator = context.state_stack.save();
  mutator.transform(transform_);

  PaintChildren(context);
}
}  // namespace flutter
