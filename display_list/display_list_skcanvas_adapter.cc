// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_skcanvas_adapter.h"

#include "flutter/display_list/display_list_canvas_dispatcher.h"

namespace flutter {

static sk_sp<SkShader> ToSk(const DlColorSource* source) {
  return source ? source->skia_object() : nullptr;
}

static sk_sp<SkImageFilter> ToSk(const DlImageFilter* filter) {
  return filter ? filter->skia_object() : nullptr;
}

static sk_sp<SkColorFilter> ToSk(const DlColorFilter* filter) {
  return filter ? filter->skia_object() : nullptr;
}

static sk_sp<SkMaskFilter> ToSk(const DlMaskFilter* filter) {
  return filter ? filter->skia_object() : nullptr;
}

static sk_sp<SkPathEffect> ToSk(const DlPathEffect* effect) {
  return effect ? effect->skia_object() : nullptr;
}

static SkPaint ToSk(const DlPaint& paint) {
  SkPaint sk_paint;

  sk_paint.setColor(paint.getColor());
  sk_paint.setBlendMode(ToSk(paint.getBlendMode()));
  sk_paint.setStyle(ToSk(paint.getDrawStyle()));
  sk_paint.setStrokeWidth(paint.getStrokeWidth());
  sk_paint.setStrokeMiter(paint.getStrokeMiter());
  sk_paint.setStrokeCap(ToSk(paint.getStrokeCap()));
  sk_paint.setStrokeJoin(ToSk(paint.getStrokeJoin()));

  sk_paint.setShader(ToSk(paint.getColorSourcePtr()));
  sk_paint.setImageFilter(ToSk(paint.getImageFilterPtr()));
  sk_paint.setColorFilter(ToSk(paint.getColorFilterPtr()));
  sk_paint.setMaskFilter(ToSk(paint.getMaskFilterPtr()));
  sk_paint.setPathEffect(ToSk(paint.getPathEffectPtr()));

  return sk_paint;
}

class SkOptionalPaint {
 public:
  explicit SkOptionalPaint(const DlPaint* paint) {
    if (paint) {
      paint_ = ToSk(*paint);
      ptr_ = &paint_;
    } else {
      ptr_ = nullptr;
    }
  }

  SkPaint* operator()() { return ptr_; }

 private:
  SkPaint paint_;
  SkPaint* ptr_;
};

void DlSkCanvasAdapter::save() {
  delegate_->save();
}

void DlSkCanvasAdapter::onSaveLayer(const SkRect* bounds,
                                    const DlPaint* paint,
                                    const DlImageFilter* backdrop) {
  sk_sp<SkImageFilter> sk_filter = backdrop ? backdrop->skia_object() : nullptr;
  SkOptionalPaint sk_paint(paint);
  delegate_->saveLayer(
      SkCanvas::SaveLayerRec{bounds, sk_paint(), sk_filter.get(), 0});
}

void DlSkCanvasAdapter::restore() {
  delegate_->restore();
}

int DlSkCanvasAdapter::getSaveCount() {
  return delegate_->getSaveCount();
}

void DlSkCanvasAdapter::restoreToCount(int restore_count) {
  delegate_->restoreToCount(restore_count);
}

void DlSkCanvasAdapter::translate(SkScalar tx, SkScalar ty) {
  delegate_->translate(tx, ty);
}

void DlSkCanvasAdapter::scale(SkScalar sx, SkScalar sy) {
  delegate_->scale(sx, sy);
}

void DlSkCanvasAdapter::rotate(SkScalar degrees) {
  delegate_->rotate(degrees);
}

void DlSkCanvasAdapter::skew(SkScalar sx, SkScalar sy) {
  delegate_->skew(sx, sy);
}

// clang-format off
void DlSkCanvasAdapter::transform2DAffine(
    SkScalar mxx, SkScalar mxy, SkScalar mxt,
    SkScalar myx, SkScalar myy, SkScalar myt) {
  delegate_->concat(SkMatrix::MakeAll(mxx, mxy, mxt, myx, myy, myt, 0, 0, 1));
}

void DlSkCanvasAdapter::transformFullPerspective(
    SkScalar mxx, SkScalar mxy, SkScalar mxz, SkScalar mxt,
    SkScalar myx, SkScalar myy, SkScalar myz, SkScalar myt,
    SkScalar mzx, SkScalar mzy, SkScalar mzz, SkScalar mzt,
    SkScalar mwx, SkScalar mwy, SkScalar mwz, SkScalar mwt) {
  delegate_->concat(SkM44(mxx, mxy, mxz, mxt,
                          myx, myy, myz, myt,
                          mzx, mzy, mzz, mzt,
                          mwx, mwy, mwz, mwt));
}
// clang-format on

void DlSkCanvasAdapter::transformReset() {
  delegate_->resetMatrix();
}

void DlSkCanvasAdapter::transform(const SkMatrix* matrix) {
  if (matrix) {
    delegate_->concat(*matrix);
  }
}
void DlSkCanvasAdapter::transform(const SkM44* matrix44) {
  if (matrix44) {
    delegate_->concat(*matrix44);
  }
}

SkM44 DlSkCanvasAdapter::getTransformFullPerspective() const {
  return delegate_->getLocalToDevice();
}

SkMatrix DlSkCanvasAdapter::getTransform() const {
  return delegate_->getTotalMatrix();
}

void DlSkCanvasAdapter::clipRect(const SkRect& rect,
                                 SkClipOp clip_op,
                                 bool is_aa) {
  delegate_->clipRect(rect, clip_op, is_aa);
}

void DlSkCanvasAdapter::clipRRect(const SkRRect& rrect,
                                  SkClipOp clip_op,
                                  bool is_aa) {
  delegate_->clipRRect(rrect, clip_op, is_aa);
}

void DlSkCanvasAdapter::clipPath(const SkPath& path,
                                 SkClipOp clip_op,
                                 bool is_aa) {
  delegate_->clipPath(path, clip_op, is_aa);
}

SkRect DlSkCanvasAdapter::getDestinationClipBounds() const {
  return SkRect::Make(delegate_->getDeviceClipBounds());
}

SkRect DlSkCanvasAdapter::getLocalClipBounds() const {
  return delegate_->getLocalClipBounds();
}

void DlSkCanvasAdapter::drawPaint(const DlPaint& paint) {
  delegate_->drawPaint(ToSk(paint));
}

void DlSkCanvasAdapter::drawColor(DlColor color, DlBlendMode mode) {
  delegate_->drawColor(color, ToSk(mode));
}

void DlSkCanvasAdapter::drawLine(const SkPoint& p0,
                                 const SkPoint& p1,
                                 const DlPaint& paint) {
  delegate_->drawLine(p0, p1, ToSk(paint));
}

void DlSkCanvasAdapter::drawRect(const SkRect& rect, const DlPaint& paint) {
  delegate_->drawRect(rect, ToSk(paint));
}

void DlSkCanvasAdapter::drawOval(const SkRect& bounds, const DlPaint& paint) {
  delegate_->drawOval(bounds, ToSk(paint));
}

void DlSkCanvasAdapter::drawCircle(const SkPoint& center,
                                   SkScalar radius,
                                   const DlPaint& paint) {
  delegate_->drawCircle(center, radius, ToSk(paint));
}

void DlSkCanvasAdapter::drawRRect(const SkRRect& rrect, const DlPaint& paint) {
  delegate_->drawRRect(rrect, ToSk(paint));
}

void DlSkCanvasAdapter::drawDRRect(const SkRRect& outer,
                                   const SkRRect& inner,
                                   const DlPaint& paint) {
  delegate_->drawDRRect(outer, inner, ToSk(paint));
}

void DlSkCanvasAdapter::drawPath(const SkPath& path, const DlPaint& paint) {
  delegate_->drawPath(path, ToSk(paint));
}

void DlSkCanvasAdapter::drawArc(const SkRect& bounds,
                                SkScalar start,
                                SkScalar sweep,
                                bool useCenter,
                                const DlPaint& paint) {
  delegate_->drawArc(bounds, start, sweep, useCenter, ToSk(paint));
}

void DlSkCanvasAdapter::drawPoints(SkCanvas::PointMode mode,
                                   uint32_t count,
                                   const SkPoint pts[],
                                   const DlPaint& paint) {
  delegate_->drawPoints(mode, count, pts, ToSk(paint));
}

void DlSkCanvasAdapter::drawVertices(const DlVertices* vertices,
                                     DlBlendMode mode,
                                     const DlPaint& paint) {
  auto sk_vertices = vertices->skia_object();
  delegate_->drawVertices(sk_vertices, ToSk(mode), ToSk(paint));
}

void DlSkCanvasAdapter::onDrawImage(const sk_sp<DlImage> image,
                                    const SkPoint point,
                                    DlImageSampling sampling,
                                    const DlPaint* paint) {
  SkOptionalPaint sk_paint(paint);
  sk_sp<SkImage> sk_image = image->skia_image();
  delegate_->drawImage(sk_image.get(), point.fX, point.fY, ToSk(sampling),
                       sk_paint());
}

void DlSkCanvasAdapter::onDrawImageRect(
    const sk_sp<DlImage> image,
    const SkRect& src,
    const SkRect& dst,
    DlImageSampling sampling,
    const DlPaint* paint,
    SkCanvas::SrcRectConstraint constraint) {
  SkOptionalPaint sk_paint(paint);
  sk_sp<SkImage> sk_image = image->skia_image();
  delegate_->drawImageRect(sk_image.get(), src, dst, ToSk(sampling), sk_paint(),
                           constraint);
}

void DlSkCanvasAdapter::onDrawImageNine(const sk_sp<DlImage> image,
                                        const SkIRect& center,
                                        const SkRect& dst,
                                        DlFilterMode filter,
                                        const DlPaint* paint) {
  SkOptionalPaint sk_paint(paint);
  sk_sp<SkImage> sk_image = image->skia_image();
  delegate_->drawImageNine(sk_image.get(), center, dst, ToSk(filter),
                           sk_paint());
}

void DlSkCanvasAdapter::onDrawAtlas(const sk_sp<DlImage> atlas,
                                    const SkRSXform xform[],
                                    const SkRect tex[],
                                    const DlColor colors[],
                                    int count,
                                    DlBlendMode mode,
                                    DlImageSampling sampling,
                                    const SkRect* cullRect,
                                    const DlPaint* paint) {
  SkOptionalPaint sk_paint(paint);
  sk_sp<SkImage> sk_image = atlas->skia_image();
  const SkColor* sk_colors = reinterpret_cast<const SkColor*>(colors);
  delegate_->drawAtlas(sk_image.get(), xform, tex, sk_colors, count, ToSk(mode),
                       ToSk(sampling), cullRect, sk_paint());
}

void DlSkCanvasAdapter::drawDisplayList(const sk_sp<DisplayList> display_list) {
  display_list->RenderTo(delegate_);
}

void DlSkCanvasAdapter::drawTextBlob(const sk_sp<SkTextBlob> blob,
                                     SkScalar x,
                                     SkScalar y,
                                     const DlPaint& paint) {
  delegate_->drawTextBlob(blob, x, y, ToSk(paint));
}

void DlSkCanvasAdapter::drawShadow(const SkPath& path,
                                   const DlColor color,
                                   const SkScalar elevation,
                                   bool transparent_occluder,
                                   SkScalar dpr) {
  DisplayListCanvasDispatcher::DrawShadow(delegate_, path, color, elevation,
                                          SkColorGetA(color) != 0xff, dpr);
}

}  // namespace flutter
