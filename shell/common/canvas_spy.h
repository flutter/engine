
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include "flutter/fml/macros.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkCanvasVirtualEnforcer.h"
#include "third_party/skia/include/utils/SkNWayCanvas.h"
#include "third_party/skia/include/utils/SkNoDrawCanvas.h"

#ifndef FLUTTER_SHELL_COMMON_CANVAS_SPY_H_
#define FLUTTER_SHELL_COMMON_CANVAS_SPY_H_

namespace flutter {

class DidDrawCanvas;

// Facilitates spying on drawing commands to an SkCanvas.
//
// This is used to determine whether anything was drawn into
// a canvas so it is possible to implement optimizations that
// are specific to empty canvases.
class CanvasSpy {
 public:
  CanvasSpy(SkCanvas* target_canvas);

  // Returns true if any non trasnparent content has been drawn into
  // the spying canvas. Note that this class does tries to detect
  // empty canvases but in some cases may return true even for empty
  // canvases (e.g when a transparent image is drawn into the canvas).
  bool DidDrawIntoCanvas();

  // The returned canvas delegate all operations to the target canvas
  // while spying on them.
  SkCanvas* GetSpyingCanvas();

 private:
  std::unique_ptr<SkNWayCanvas> n_way_canvas_;
  std::unique_ptr<DidDrawCanvas> did_draw_canvas_;

  FML_DISALLOW_COPY_AND_ASSIGN(CanvasSpy);
};

class DidDrawCanvas : public SkCanvasVirtualEnforcer<SkNoDrawCanvas> {
 public:
  DidDrawCanvas(int width, int height);
  ~DidDrawCanvas() override;
  bool DidDrawIntoCanvas();

 protected:
  bool did_draw_;

  void willSave() override;
  SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
  bool onDoSaveBehind(const SkRect*) override;
  void willRestore() override;

  void didConcat(const SkMatrix&) override;
  void didSetMatrix(const SkMatrix&) override;

  void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
  virtual void onDrawTextBlob(const SkTextBlob* blob,
                              SkScalar x,
                              SkScalar y,
                              const SkPaint& paint) override;
  virtual void onDrawPatch(const SkPoint cubics[12],
                           const SkColor colors[4],
                           const SkPoint texCoords[4],
                           SkBlendMode,
                           const SkPaint& paint) override;

  void onDrawPaint(const SkPaint&) override;
  void onDrawBehind(const SkPaint&) override;
  void onDrawPoints(PointMode,
                    size_t count,
                    const SkPoint pts[],
                    const SkPaint&) override;
  void onDrawRect(const SkRect&, const SkPaint&) override;
  void onDrawRegion(const SkRegion&, const SkPaint&) override;
  void onDrawOval(const SkRect&, const SkPaint&) override;
  void onDrawArc(const SkRect&,
                 SkScalar,
                 SkScalar,
                 bool,
                 const SkPaint&) override;
  void onDrawRRect(const SkRRect&, const SkPaint&) override;
  void onDrawPath(const SkPath&, const SkPaint&) override;
  void onDrawBitmap(const SkBitmap&,
                    SkScalar left,
                    SkScalar top,
                    const SkPaint*) override;
  void onDrawBitmapRect(const SkBitmap&,
                        const SkRect* src,
                        const SkRect& dst,
                        const SkPaint*,
                        SrcRectConstraint) override;
  void onDrawImage(const SkImage*,
                   SkScalar left,
                   SkScalar top,
                   const SkPaint*) override;
  void onDrawImageRect(const SkImage*,
                       const SkRect* src,
                       const SkRect& dst,
                       const SkPaint*,
                       SrcRectConstraint) override;
  void onDrawBitmapLattice(const SkBitmap&,
                           const Lattice&,
                           const SkRect&,
                           const SkPaint*) override;
  void onDrawImageLattice(const SkImage*,
                          const Lattice&,
                          const SkRect&,
                          const SkPaint*) override;
  void onDrawImageNine(const SkImage*,
                       const SkIRect& center,
                       const SkRect& dst,
                       const SkPaint*) override;
  void onDrawBitmapNine(const SkBitmap&,
                        const SkIRect& center,
                        const SkRect& dst,
                        const SkPaint*) override;
  void onDrawVerticesObject(const SkVertices*,
                            const SkVertices::Bone bones[],
                            int boneCount,
                            SkBlendMode,
                            const SkPaint&) override;
  void onDrawAtlas(const SkImage*,
                   const SkRSXform[],
                   const SkRect[],
                   const SkColor[],
                   int,
                   SkBlendMode,
                   const SkRect*,
                   const SkPaint*) override;
  void onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) override;

  void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
  void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
  void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;
  void onClipRegion(const SkRegion&, SkClipOp) override;

  void onDrawPicture(const SkPicture*,
                     const SkMatrix*,
                     const SkPaint*) override;
  void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
  void onDrawAnnotation(const SkRect&, const char[], SkData*) override;

  void onDrawEdgeAAQuad(const SkRect&,
                        const SkPoint[4],
                        QuadAAFlags,
                        SkColor,
                        SkBlendMode) override;
  void onDrawEdgeAAImageSet(const ImageSetEntry[],
                            int count,
                            const SkPoint[],
                            const SkMatrix[],
                            const SkPaint*,
                            SrcRectConstraint) override;

  void onFlush() override;

 private:
  void MarkDrawIfNonTransparentPaint(const SkPaint& paint);
  typedef SkCanvasVirtualEnforcer<SkNoDrawCanvas> INHERITED;

  FML_DISALLOW_COPY_AND_ASSIGN(DidDrawCanvas);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_SKIA_EVENT_TRACER_IMPL_H_
