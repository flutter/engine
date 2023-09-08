// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/skia/dl_sk_canvas.h"

#include "flutter/display_list/skia/dl_sk_conversions.h"
#include "flutter/display_list/skia/dl_sk_dispatcher.h"
#include "flutter/fml/trace_event.h"

#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"
#include "third_party/skia/include/gpu/GrRecordingContext.h"

namespace flutter {

// clang-format off
constexpr float kInvertColorMatrix[20] = {
  -1.0,    0,    0, 1.0, 0,
     0, -1.0,    0, 1.0, 0,
     0,    0, -1.0, 1.0, 0,
   1.0,  1.0,  1.0, 1.0, 0
};
// clang-format on

static SkPaint ToSk(const DlPaint& paint, bool force_stroke = false) {
  SkPaint sk_paint;

  sk_paint.setAntiAlias(paint.isAntiAlias());
  sk_paint.setDither(paint.isDither());

  sk_paint.setColor(paint.getColor());
  sk_paint.setBlendMode(ToSk(paint.getBlendMode()));
  sk_paint.setStyle(force_stroke ? SkPaint::kStroke_Style
                                 : ToSk(paint.getDrawStyle()));
  sk_paint.setStrokeWidth(paint.getStrokeWidth());
  sk_paint.setStrokeMiter(paint.getStrokeMiter());
  sk_paint.setStrokeCap(ToSk(paint.getStrokeCap()));
  sk_paint.setStrokeJoin(ToSk(paint.getStrokeJoin()));

  sk_paint.setShader(ToSk(paint.getColorSourcePtr()));
  sk_paint.setImageFilter(ToSk(paint.getImageFilterPtr()));
  auto color_filter = ToSk(paint.getColorFilterPtr());
  if (paint.isInvertColors()) {
    auto invert_filter = SkColorFilters::Matrix(kInvertColorMatrix);
    if (color_filter) {
      invert_filter = invert_filter->makeComposed(color_filter);
    }
    color_filter = invert_filter;
  }
  sk_paint.setColorFilter(color_filter);
  sk_paint.setMaskFilter(ToSk(paint.getMaskFilterPtr()));
  sk_paint.setPathEffect(ToSk(paint.getPathEffectPtr()));

  return sk_paint;
}

class SkOptionalPaint {
 public:
  explicit SkOptionalPaint(const DlPaint* dl_paint) {
    if (dl_paint && !dl_paint->isDefault()) {
      sk_paint_ = ToSk(*dl_paint);
      ptr_ = &sk_paint_;
    } else {
      ptr_ = nullptr;
    }
  }

  SkPaint* operator()() { return ptr_; }

 private:
  SkPaint sk_paint_;
  SkPaint* ptr_;
};

void DlSkCanvasAdapter::set_canvas(SkCanvas* canvas) {
  delegate_ = canvas;
}

DlISize DlSkCanvasAdapter::GetBaseLayerSize() const {
  return ToDl(delegate_->getBaseLayerSize());
}

SkImageInfo DlSkCanvasAdapter::GetImageInfo() const {
  return delegate_->imageInfo();
}

void DlSkCanvasAdapter::Save() {
  delegate_->save();
}

void DlSkCanvasAdapter::SaveLayer(const DlFRect* bounds,
                                  const DlPaint* paint,
                                  const DlImageFilter* backdrop) {
  sk_sp<SkImageFilter> sk_backdrop = ToSk(backdrop);
  SkOptionalPaint sk_paint(paint);
  TRACE_EVENT0("flutter", "Canvas::saveLayer");
  SkRect scratch;
  delegate_->saveLayer(SkCanvas::SaveLayerRec{
      ToSk(&scratch, bounds), sk_paint(), sk_backdrop.get(), 0});
}

void DlSkCanvasAdapter::Restore() {
  delegate_->restore();
}

int DlSkCanvasAdapter::GetSaveCount() const {
  return delegate_->getSaveCount();
}

void DlSkCanvasAdapter::RestoreToCount(int restore_count) {
  delegate_->restoreToCount(restore_count);
}

void DlSkCanvasAdapter::Translate(DlScalar tx, DlScalar ty) {
  delegate_->translate(tx, ty);
}

void DlSkCanvasAdapter::Scale(DlScalar sx, DlScalar sy) {
  delegate_->scale(sx, sy);
}

void DlSkCanvasAdapter::Rotate(DlAngle angle) {
  delegate_->rotate(angle.degrees());
}

void DlSkCanvasAdapter::Skew(DlScalar sx, DlScalar sy) {
  delegate_->skew(sx, sy);
}

// clang-format off

// 2x3 2D affine subset of a 4x4 transform in row major order
void DlSkCanvasAdapter::Transform2DAffine(
    DlScalar mxx, DlScalar mxy, DlScalar mxt,
    DlScalar myx, DlScalar myy, DlScalar myt) {
  delegate_->concat(SkMatrix::MakeAll(mxx, mxy, mxt, myx, myy, myt, 0, 0, 1));
}

// full 4x4 transform in row major order
void DlSkCanvasAdapter::TransformFullPerspective(
    DlScalar mxx, DlScalar mxy, DlScalar mxz, DlScalar mxt,
    DlScalar myx, DlScalar myy, DlScalar myz, DlScalar myt,
    DlScalar mzx, DlScalar mzy, DlScalar mzz, DlScalar mzt,
    DlScalar mwx, DlScalar mwy, DlScalar mwz, DlScalar mwt) {
  delegate_->concat(SkM44(mxx, mxy, mxz, mxt,
                          myx, myy, myz, myt,
                          mzx, mzy, mzz, mzt,
                          mwx, mwy, mwz, mwt));
}

// clang-format on

void DlSkCanvasAdapter::TransformReset() {
  delegate_->resetMatrix();
}

void DlSkCanvasAdapter::Transform(const DlTransform& matrix) {
  delegate_->concat(ToSk(matrix));
}

void DlSkCanvasAdapter::SetTransform(const DlTransform& matrix) {
  delegate_->setMatrix(ToSk(matrix));
}

/// Returns the 4x4 full perspective transform representing all transform
/// operations executed so far in this DisplayList within the enclosing
/// save stack.
DlTransform DlSkCanvasAdapter::GetTransform() const {
  return ToDl(delegate_->getLocalToDevice());
}

void DlSkCanvasAdapter::ClipRect(const DlFRect& rect,
                                 ClipOp clip_op,
                                 bool is_aa) {
  delegate_->clipRect(ToSk(rect), ToSk(clip_op), is_aa);
}

void DlSkCanvasAdapter::ClipRRect(const DlFRRect& rrect,
                                  ClipOp clip_op,
                                  bool is_aa) {
  delegate_->clipRRect(ToSk(rrect), ToSk(clip_op), is_aa);
}

void DlSkCanvasAdapter::ClipPath(const DlPath& path,
                                 ClipOp clip_op,
                                 bool is_aa) {
  delegate_->clipPath(path.GetSkiaPath(), ToSk(clip_op), is_aa);
}

/// Conservative estimate of the bounds of all outstanding clip operations
/// measured in the coordinate space within which this DisplayList will
/// be rendered.
DlFRect DlSkCanvasAdapter::GetDestinationClipBounds() const {
  SkIRect sk_bounds = delegate_->getDeviceClipBounds();
  return DlFRect::MakeLTRB(sk_bounds.fLeft, sk_bounds.fTop,  //
                           sk_bounds.fRight, sk_bounds.fBottom);
}

/// Conservative estimate of the bounds of all outstanding clip operations
/// transformed into the local coordinate space in which currently
/// recorded rendering operations are interpreted.
DlFRect DlSkCanvasAdapter::GetLocalClipBounds() const {
  return ToDl(delegate_->getLocalClipBounds());
}

/// Return true iff the supplied bounds are easily shown to be outside
/// of the current clip bounds. This method may conservatively return
/// false if it cannot make the determination.
bool DlSkCanvasAdapter::QuickReject(const DlFRect& bounds) const {
  return delegate_->quickReject(ToSk(bounds));
}

void DlSkCanvasAdapter::DrawPaint(const DlPaint& paint) {
  delegate_->drawPaint(ToSk(paint));
}

void DlSkCanvasAdapter::DrawColor(DlColor color, DlBlendMode mode) {
  delegate_->drawColor(color, ToSk(mode));
}

void DlSkCanvasAdapter::DrawLine(const DlFPoint& p0,
                                 const DlFPoint& p1,
                                 const DlPaint& paint) {
  delegate_->drawLine(ToSk(p0), ToSk(p1), ToSk(paint, true));
}

void DlSkCanvasAdapter::DrawRect(const DlFRect& rect, const DlPaint& paint) {
  delegate_->drawRect(ToSk(rect), ToSk(paint));
}

void DlSkCanvasAdapter::DrawOval(const DlFRect& bounds, const DlPaint& paint) {
  delegate_->drawOval(ToSk(bounds), ToSk(paint));
}

void DlSkCanvasAdapter::DrawCircle(const DlFPoint& center,
                                   DlScalar radius,
                                   const DlPaint& paint) {
  delegate_->drawCircle(ToSk(center), radius, ToSk(paint));
}

void DlSkCanvasAdapter::DrawRRect(const DlFRRect& rrect, const DlPaint& paint) {
  delegate_->drawRRect(ToSk(rrect), ToSk(paint));
}

void DlSkCanvasAdapter::DrawDRRect(const DlFRRect& outer,
                                   const DlFRRect& inner,
                                   const DlPaint& paint) {
  delegate_->drawDRRect(ToSk(outer), ToSk(inner), ToSk(paint));
}

void DlSkCanvasAdapter::DrawPath(const DlPath& path, const DlPaint& paint) {
  delegate_->drawPath(path.GetSkiaPath(), ToSk(paint));
}

void DlSkCanvasAdapter::DrawArc(const DlFRect& bounds,
                                DlScalar start,
                                DlScalar sweep,
                                bool useCenter,
                                const DlPaint& paint) {
  delegate_->drawArc(ToSk(bounds), start, sweep, useCenter, ToSk(paint));
}

void DlSkCanvasAdapter::DrawPoints(PointMode mode,
                                   uint32_t count,
                                   const DlFPoint pts[],
                                   const DlPaint& paint) {
  delegate_->drawPoints(ToSk(mode), count, ToSk(pts), ToSk(paint, true));
}

void DlSkCanvasAdapter::DrawVertices(const DlVertices* vertices,
                                     DlBlendMode mode,
                                     const DlPaint& paint) {
  delegate_->drawVertices(ToSk(vertices), ToSk(mode), ToSk(paint));
}

void DlSkCanvasAdapter::DrawImage(const sk_sp<DlImage>& image,
                                  const DlFPoint point,
                                  DlImageSampling sampling,
                                  const DlPaint* paint) {
  SkOptionalPaint sk_paint(paint);
  sk_sp<SkImage> sk_image = image->skia_image();
  delegate_->drawImage(sk_image.get(), point.x(), point.y(), ToSk(sampling),
                       sk_paint());
}

void DlSkCanvasAdapter::DrawImageRect(const sk_sp<DlImage>& image,
                                      const DlFRect& src,
                                      const DlFRect& dst,
                                      DlImageSampling sampling,
                                      const DlPaint* paint,
                                      SrcRectConstraint constraint) {
  SkOptionalPaint sk_paint(paint);
  sk_sp<SkImage> sk_image = image->skia_image();
  delegate_->drawImageRect(sk_image.get(), ToSk(src), ToSk(dst), ToSk(sampling),
                           sk_paint(), ToSk(constraint));
}

void DlSkCanvasAdapter::DrawImageNine(const sk_sp<DlImage>& image,
                                      const DlIRect& center,
                                      const DlFRect& dst,
                                      DlFilterMode filter,
                                      const DlPaint* paint) {
  SkOptionalPaint sk_paint(paint);
  sk_sp<SkImage> sk_image = image->skia_image();
  delegate_->drawImageNine(sk_image.get(), ToSk(center), ToSk(dst),
                           ToSk(filter), sk_paint());
}

void DlSkCanvasAdapter::DrawAtlas(const sk_sp<DlImage>& atlas,
                                  const DlRSTransform xform[],
                                  const DlFRect tex[],
                                  const DlColor colors[],
                                  int count,
                                  DlBlendMode mode,
                                  DlImageSampling sampling,
                                  const DlFRect* cullRect,
                                  const DlPaint* paint) {
  SkOptionalPaint sk_paint(paint);
  sk_sp<SkImage> sk_image = atlas->skia_image();
  const SkColor* sk_colors = reinterpret_cast<const SkColor*>(colors);
  delegate_->drawAtlas(sk_image.get(), ToSk(xform), ToSk(tex), sk_colors, count,
                       ToSk(mode), ToSk(sampling), ToSk(cullRect), sk_paint());
}

void DlSkCanvasAdapter::DrawDisplayList(const sk_sp<DisplayList> display_list,
                                        DlScalar opacity) {
  const int restore_count = delegate_->getSaveCount();

  // Figure out whether we can apply the opacity during dispatch or
  // if we need a saveLayer.
  if (opacity < SK_Scalar1 && !display_list->can_apply_group_opacity()) {
    TRACE_EVENT0("flutter", "Canvas::saveLayer");
    SkRect bounds = ToSk(display_list->bounds());
    delegate_->saveLayerAlphaf(&bounds, opacity);
    opacity = SK_Scalar1;
  } else {
    delegate_->save();
  }

  DlSkCanvasDispatcher dispatcher(delegate_, opacity);
  if (display_list->has_rtree()) {
    display_list->Dispatch(dispatcher, ToDl(delegate_->getLocalClipBounds()));
  } else {
    display_list->Dispatch(dispatcher);
  }

  delegate_->restoreToCount(restore_count);
}

void DlSkCanvasAdapter::DrawTextBlob(const sk_sp<SkTextBlob>& blob,
                                     DlScalar x,
                                     DlScalar y,
                                     const DlPaint& paint) {
  delegate_->drawTextBlob(blob, x, y, ToSk(paint));
}

void DlSkCanvasAdapter::DrawShadow(const DlPath& path,
                                   const DlColor color,
                                   const DlScalar elevation,
                                   bool transparent_occluder,
                                   DlScalar dpr) {
  DlSkCanvasDispatcher::DrawShadow(delegate_, path, color, elevation,
                                   transparent_occluder, dpr);
}

void DlSkCanvasAdapter::Flush() {
  auto dContext = GrAsDirectContext(delegate_->recordingContext());

  if (dContext) {
    dContext->flushAndSubmit();
  }
}

}  // namespace flutter
