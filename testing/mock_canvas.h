// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_MOCK_CANVAS_H_
#define TESTING_MOCK_CANVAS_H_

#include <variant>
#include <vector>

#include "flutter/testing/assertions_skia.h"
#include "gtest/gtest.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkCanvasVirtualEnforcer.h"
#include "third_party/skia/include/core/SkClipOp.h"
#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/utils/SkNWayCanvas.h"

namespace flutter {
namespace testing {

class MockCanvas : public SkCanvasVirtualEnforcer<SkCanvas> {
 public:
  using SkCanvas::kHard_ClipEdgeStyle;
  using SkCanvas::kSoft_ClipEdgeStyle;

  struct SaveData {
    int save_to_layer;
  };

  struct SaveLayerData {
    SkRect save_bounds;
    SkPaint restore_paint;
    sk_sp<SkImageFilter> backdrop_filter;
    int save_to_layer;
  };

  struct RestoreData {
    int restore_to_layer;
  };

  struct ConcatMatrixData {
    SkMatrix matrix;
  };

  struct SetMatrixData {
    SkMatrix matrix;
  };

  struct DrawRectData {
    SkRect rect;
    SkPaint paint;
  };

  struct DrawPathData {
    SkPath path;
    SkPaint paint;
  };

  struct DrawPictureData {
    const SkMatrix* matrix;
    const SkPaint* paint;
    const SkPicture* picture;
  };

  struct DrawShadowData {
    SkPath path;
  };

  struct ClipRectData {
    SkRect rect;
    SkClipOp clip_op;
    ClipEdgeStyle style;
  };

  struct ClipRRectData {
    SkRRect rrect;
    SkClipOp clip_op;
    ClipEdgeStyle style;
  };

  struct ClipPathData {
    SkPath path;
    SkClipOp clip_op;
    ClipEdgeStyle style;
  };

  using DrawCallData = std::variant<SaveData,
                                    SaveLayerData,
                                    RestoreData,
                                    ConcatMatrixData,
                                    SetMatrixData,
                                    DrawRectData,
                                    DrawPathData,
                                    DrawPictureData,
                                    DrawShadowData,
                                    ClipRectData,
                                    ClipRRectData,
                                    ClipPathData>;

  // A single call made against this canvas.
  struct DrawCall {
    int layer;
    DrawCallData data;
  };

  MockCanvas();
  virtual ~MockCanvas() = default;

  SkNWayCanvas* internal_canvas() { return &internal_canvas_; }

  void ExpectDrawCalls(std::vector<DrawCall> expected_calls);
  const std::vector<DrawCall>& draw_calls() const { return draw_calls_; }

 protected:
  // Save/restore/set operations that we track.
  void willSave() override;
  SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override;
  void willRestore() override;
  void didRestore() override {}
  void didConcat(const SkMatrix& matrix) override;
  void didSetMatrix(const SkMatrix& matrix) override;

  // Draw and clip operations that we track.
  void onDrawRect(const SkRect& rect, const SkPaint& paint) override;
  void onDrawPath(const SkPath& path, const SkPaint& paint) override;
  void onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) override;
  void onDrawPicture(const SkPicture* picture,
                     const SkMatrix* matrix,
                     const SkPaint* paint) override;
  void onClipRect(const SkRect& rect,
                  SkClipOp op,
                  ClipEdgeStyle style) override;
  void onClipRRect(const SkRRect& rrect,
                   SkClipOp op,
                   ClipEdgeStyle style) override;
  void onClipPath(const SkPath& path,
                  SkClipOp op,
                  ClipEdgeStyle style) override;

  // Operations that we don't track.
  bool onDoSaveBehind(const SkRect*) override;
  void onDrawAnnotation(const SkRect&, const char[], SkData*) override;
  void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
  void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
  void onDrawTextBlob(const SkTextBlob*,
                      SkScalar,
                      SkScalar,
                      const SkPaint&) override;
  void onDrawPatch(const SkPoint[12],
                   const SkColor[4],
                   const SkPoint[4],
                   SkBlendMode,
                   const SkPaint&) override;
  void onDrawPaint(const SkPaint&) override;
  void onDrawBehind(const SkPaint&) override;
  void onDrawPoints(PointMode,
                    size_t,
                    const SkPoint[],
                    const SkPaint&) override;
  void onDrawRegion(const SkRegion&, const SkPaint&) override;
  void onDrawOval(const SkRect&, const SkPaint&) override;
  void onDrawArc(const SkRect&,
                 SkScalar,
                 SkScalar,
                 bool,
                 const SkPaint&) override;
  void onDrawRRect(const SkRRect&, const SkPaint&) override;
  void onDrawBitmap(const SkBitmap&,
                    SkScalar,
                    SkScalar,
                    const SkPaint*) override;
  void onDrawBitmapRect(const SkBitmap&,
                        const SkRect*,
                        const SkRect&,
                        const SkPaint*,
                        SrcRectConstraint) override;
  void onDrawImage(const SkImage*, SkScalar, SkScalar, const SkPaint*) override;
  void onDrawImageRect(const SkImage*,
                       const SkRect*,
                       const SkRect&,
                       const SkPaint*,
                       SrcRectConstraint) override;
  void onDrawImageNine(const SkImage*,
                       const SkIRect&,
                       const SkRect&,
                       const SkPaint*) override;
  void onDrawBitmapNine(const SkBitmap&,
                        const SkIRect&,
                        const SkRect&,
                        const SkPaint*) override;
  void onDrawImageLattice(const SkImage*,
                          const Lattice&,
                          const SkRect&,
                          const SkPaint*) override;
  void onDrawBitmapLattice(const SkBitmap&,
                           const Lattice&,
                           const SkRect&,
                           const SkPaint*) override;
  void onDrawVerticesObject(const SkVertices*,
                            const SkVertices::Bone[],
                            int,
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
  void onDrawEdgeAAQuad(const SkRect&,
                        const SkPoint[4],
                        QuadAAFlags,
                        const SkColor4f&,
                        SkBlendMode) override;
  void onDrawEdgeAAImageSet(const ImageSetEntry[],
                            int,
                            const SkPoint[],
                            const SkMatrix[],
                            const SkPaint*,
                            SrcRectConstraint) override;
  void onClipRegion(const SkRegion&, SkClipOp) override;

 private:
  SkNWayCanvas internal_canvas_;

  std::vector<DrawCall> draw_calls_;
  int current_layer_;
};

inline bool operator==(const MockCanvas::SaveData& a,
                       const MockCanvas::SaveData& b) {
  return a.save_to_layer == b.save_to_layer;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::SaveData& data) {
  return os << data.save_to_layer;
}

inline bool operator==(const MockCanvas::SaveLayerData& a,
                       const MockCanvas::SaveLayerData& b) {
  return a.save_bounds == b.save_bounds && a.restore_paint == b.restore_paint &&
         a.backdrop_filter == b.backdrop_filter &&
         a.save_to_layer == b.save_to_layer;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::SaveLayerData& data) {
  return os << data.save_bounds << " " << data.restore_paint << " "
            << data.backdrop_filter << " " << data.save_to_layer;
}

inline bool operator==(const MockCanvas::RestoreData& a,
                       const MockCanvas::RestoreData& b) {
  return a.restore_to_layer == b.restore_to_layer;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::RestoreData& data) {
  return os << data.restore_to_layer;
}

inline bool operator==(const MockCanvas::ConcatMatrixData& a,
                       const MockCanvas::ConcatMatrixData& b) {
  return a.matrix == b.matrix;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::ConcatMatrixData& data) {
  return os << data.matrix;
}

inline bool operator==(const MockCanvas::SetMatrixData& a,
                       const MockCanvas::SetMatrixData& b) {
  return a.matrix == b.matrix;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::SetMatrixData& data) {
  return os << data.matrix;
}

inline bool operator==(const MockCanvas::DrawRectData& a,
                       const MockCanvas::DrawRectData& b) {
  return a.rect == b.rect && a.paint == b.paint;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawRectData& data) {
  return os << data.rect << " " << data.paint;
}

inline bool operator==(const MockCanvas::DrawPathData& a,
                       const MockCanvas::DrawPathData& b) {
  return a.path == b.path && a.paint == b.paint;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawPathData& data) {
  return os << data.path << " " << data.paint;
}

inline bool operator==(const MockCanvas::DrawPictureData& a,
                       const MockCanvas::DrawPictureData& b) {
  return a.picture == b.picture && a.matrix == b.matrix && a.paint == b.paint;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawPictureData& data) {
  os << data.picture << " ";
  if (data.matrix) {
    os << *data.matrix << " ";
  } else {
    os << "(null)"
       << " ";
  }
  if (data.paint) {
    os << *data.paint << " ";
  } else {
    os << "(null)"
       << " ";
  }
  return os;
}

inline bool operator==(const MockCanvas::DrawShadowData& a,
                       const MockCanvas::DrawShadowData& b) {
  return a.path == b.path;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawShadowData& data) {
  return os << data.path;
}

inline bool operator==(const MockCanvas::ClipRectData& a,
                       const MockCanvas::ClipRectData& b) {
  return a.rect == b.rect && a.clip_op == b.clip_op && a.style == b.style;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::ClipRectData& data) {
  return os << data.rect << " " << data.clip_op << " " << data.style;
}

inline bool operator==(const MockCanvas::ClipRRectData& a,
                       const MockCanvas::ClipRRectData& b) {
  return a.rrect == b.rrect && a.clip_op == b.clip_op && a.style == b.style;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::ClipRRectData& data) {
  return os << data.rrect << " " << data.clip_op << " " << data.style;
}

inline bool operator==(const MockCanvas::ClipPathData& a,
                       const MockCanvas::ClipPathData& b) {
  return a.path == b.path && a.clip_op == b.clip_op && a.style == b.style;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::ClipPathData& data) {
  return os << data.path << " " << data.clip_op << " " << data.style;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawCallData& data) {
  std::visit([&os](auto& d) { os << d; }, data);
  return os;
}

inline bool operator==(const MockCanvas::DrawCall& a,
                       const MockCanvas::DrawCall& b) {
  return a.layer == b.layer && a.data == b.data;
}

inline std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawCall& draw) {
  return os << "[Layer: " << draw.layer << ", Data: " << draw.data << "]";
}

}  // namespace testing
}  // namespace flutter

#endif  // TESTING_MOCK_CANVAS_H_
