// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/mock_canvas.h"

#include "flutter/fml/logging.h"
#include "third_party/skia/include/core/SkImageInfo.h"

namespace flutter {
namespace testing {

MockCanvas::MockCanvas(int width, int height)
    : SkCanvasVirtualEnforcer<SkCanvas>(width, height),
      internal_canvas_(imageInfo().width(), imageInfo().height()) {}

SkCanvas::SaveLayerStrategy MockCanvas::getSaveLayerStrategy(
    const SaveLayerRec& rec) {
  (void)SkCanvasVirtualEnforcer<SkCanvas>::getSaveLayerStrategy(rec);
  return kNoLayer_SaveLayerStrategy;
}

bool MockCanvas::onDoSaveBehind(const SkRect*) {
  FML_DCHECK(false);
  return false;
}

void MockCanvas::onDrawAnnotation(const SkRect&, const char[], SkData*) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawDrawable(SkDrawable*, const SkMatrix*) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawTextBlob(const SkTextBlob*,
                                SkScalar,
                                SkScalar,
                                const SkPaint&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawPatch(const SkPoint[12],
                             const SkColor[4],
                             const SkPoint[4],
                             SkBlendMode,
                             const SkPaint&) {
  FML_DCHECK(false);
}

void MockCanvas::onDrawPaint(const SkPaint&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawBehind(const SkPaint&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawPoints(PointMode,
                              size_t,
                              const SkPoint[],
                              const SkPaint&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawRect(const SkRect&, const SkPaint&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawRegion(const SkRegion&, const SkPaint&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawOval(const SkRect&, const SkPaint&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawArc(const SkRect&,
                           SkScalar,
                           SkScalar,
                           bool,
                           const SkPaint&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawRRect(const SkRRect&, const SkPaint&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawPath(const SkPath&, const SkPaint&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawBitmap(const SkBitmap&,
                              SkScalar,
                              SkScalar,
                              const SkPaint*) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawBitmapRect(const SkBitmap&,
                                  const SkRect*,
                                  const SkRect&,
                                  const SkPaint*,
                                  SrcRectConstraint) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawImage(const SkImage*,
                             SkScalar,
                             SkScalar,
                             const SkPaint*) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawImageRect(const SkImage*,
                                 const SkRect*,
                                 const SkRect&,
                                 const SkPaint*,
                                 SrcRectConstraint) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawImageNine(const SkImage*,
                                 const SkIRect&,
                                 const SkRect&,
                                 const SkPaint*) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawBitmapNine(const SkBitmap&,
                                  const SkIRect&,
                                  const SkRect&,
                                  const SkPaint*) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawImageLattice(const SkImage*,
                                    const Lattice&,
                                    const SkRect&,
                                    const SkPaint*) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawBitmapLattice(const SkBitmap&,
                                     const Lattice&,
                                     const SkRect&,
                                     const SkPaint*) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawVerticesObject(const SkVertices*,
                                      const SkVertices::Bone[],
                                      int,
                                      SkBlendMode,
                                      const SkPaint&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawAtlas(const SkImage*,
                             const SkRSXform[],
                             const SkRect[],
                             const SkColor[],
                             int,
                             SkBlendMode,
                             const SkRect*,
                             const SkPaint*) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawPicture(const SkPicture*,
                               const SkMatrix*,
                               const SkPaint*) {
  FML_DCHECK(false);
}

void MockCanvas::onDrawEdgeAAQuad(const SkRect&,
                                  const SkPoint[4],
                                  QuadAAFlags,
                                  const SkColor4f&,
                                  SkBlendMode) {
  FML_DCHECK(false);
}
void MockCanvas::onDrawEdgeAAImageSet(const ImageSetEntry[],
                                      int,
                                      const SkPoint[],
                                      const SkMatrix[],
                                      const SkPaint*,
                                      SrcRectConstraint) {
  FML_DCHECK(false);
}

}  // namespace testing
}  // namespace flutter
