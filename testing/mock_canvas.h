// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_MOCK_CANVAS_H_
#define TESTING_MOCK_CANVAS_H_

#include <ostream>
#include <variant>
#include <vector>

#include "flutter/display_list/dl_canvas.h"
#include "flutter/display_list/utils/dl_matrix_clip_tracker.h"
#include "gtest/gtest.h"
#include "third_party/skia/include/core/SkData.h"
#include "third_party/skia/include/core/SkTextBlob.h"

namespace flutter {
namespace testing {

// Mock |DlCanvas|, useful for writing tests that use DlCanvas but do not
// interact with the GPU.
//
// The |MockCanvas| stores a list of |DrawCall| that the test can later verify
// against the expected list of primitives to be drawn.
class MockCanvas final : public DlCanvas {
 public:
  enum ClipEdgeStyle {
    kHard_ClipEdgeStyle,
    kSoft_ClipEdgeStyle,
  };

  struct SaveData {
    int save_to_layer;
  };

  struct SaveLayerData {
    DlFRect save_bounds;
    DlPaint restore_paint;
    std::shared_ptr<DlImageFilter> backdrop_filter;
    int save_to_layer;
  };

  struct RestoreData {
    int restore_to_layer;
  };

  struct ConcatMatrixData {
    DlTransform matrix;
  };

  struct SetMatrixData {
    DlTransform matrix;
  };

  struct DrawRectData {
    DlFRect rect;
    DlPaint paint;
  };

  struct DrawPathData {
    DlPath path;
    DlPaint paint;
  };

  struct DrawTextData {
    sk_sp<SkData> serialized_text;
    DlPaint paint;
    DlFPoint offset;
  };

  struct DrawImageDataNoPaint {
    sk_sp<DlImage> image;
    DlScalar x;
    DlScalar y;
    DlImageSampling options;
  };

  struct DrawImageData {
    sk_sp<DlImage> image;
    DlScalar x;
    DlScalar y;
    DlImageSampling options;
    DlPaint paint;
  };

  struct DrawDisplayListData {
    sk_sp<DisplayList> display_list;
    DlScalar opacity;
  };

  struct DrawShadowData {
    DlPath path;
    DlColor color;
    DlScalar elevation;
    bool transparent_occluder;
    DlScalar dpr;
  };

  struct ClipRectData {
    DlFRect rect;
    ClipOp clip_op;
    ClipEdgeStyle style;
  };

  struct ClipRRectData {
    DlFRRect rrect;
    ClipOp clip_op;
    ClipEdgeStyle style;
  };

  struct ClipPathData {
    DlPath path;
    ClipOp clip_op;
    ClipEdgeStyle style;
  };

  struct DrawPaintData {
    DlPaint paint;
  };

  // Discriminated union of all the different |DrawCall| types.  It is roughly
  // equivalent to the different methods in |DlCanvas|' public API.
  using DrawCallData = std::variant<SaveData,
                                    SaveLayerData,
                                    RestoreData,
                                    ConcatMatrixData,
                                    SetMatrixData,
                                    DrawRectData,
                                    DrawPathData,
                                    DrawTextData,
                                    DrawImageDataNoPaint,
                                    DrawImageData,
                                    DrawDisplayListData,
                                    DrawShadowData,
                                    ClipRectData,
                                    ClipRRectData,
                                    ClipPathData,
                                    DrawPaintData>;

  // A single call made against this canvas.
  struct DrawCall {
    int layer;
    DrawCallData data;
  };

  MockCanvas();
  MockCanvas(int width, int height);
  ~MockCanvas();

  const std::vector<DrawCall>& draw_calls() const { return draw_calls_; }
  void reset_draw_calls() { draw_calls_.clear(); }

  DlISize GetBaseLayerSize() const override;
  SkImageInfo GetImageInfo() const override;

  void Save() override;
  void SaveLayer(const DlFRect* bounds,
                 const DlPaint* paint = nullptr,
                 const DlImageFilter* backdrop = nullptr) override;
  void Restore() override;
  int GetSaveCount() const { return current_layer_; }
  void RestoreToCount(int restore_count) {
    while (current_layer_ > restore_count) {
      Restore();
    }
  }

  // clang-format off

  // 2x3 2D affine subset of a 4x4 transform in row major order
  void Transform2DAffine(DlScalar mxx, DlScalar mxy, DlScalar mxt,
                         DlScalar myx, DlScalar myy, DlScalar myt) override;
  // full 4x4 transform in row major order
  void TransformFullPerspective(
      DlScalar mxx, DlScalar mxy, DlScalar mxz, DlScalar mxt,
      DlScalar myx, DlScalar myy, DlScalar myz, DlScalar myt,
      DlScalar mzx, DlScalar mzy, DlScalar mzz, DlScalar mzt,
      DlScalar mwx, DlScalar mwy, DlScalar mwz, DlScalar mwt) override;
  // clang-format on

  void Translate(DlScalar tx, DlScalar ty) override;
  void Scale(DlScalar sx, DlScalar sy) override;
  void Rotate(DlAngle angle) override;
  void Skew(DlScalar sx, DlScalar sy) override;
  void TransformReset() override;
  void Transform(const DlTransform& matrix) override;
  void SetTransform(const DlTransform& matrix) override;
  using DlCanvas::SetTransform;
  using DlCanvas::Transform;

  DlTransform GetTransform() const override;

  void ClipRect(const DlFRect& rect, ClipOp clip_op, bool is_aa) override;
  void ClipRRect(const DlFRRect& rrect, ClipOp clip_op, bool is_aa) override;
  void ClipPath(const DlPath& path, ClipOp clip_op, bool is_aa) override;

  DlFRect GetDestinationClipBounds() const override;
  DlFRect GetLocalClipBounds() const override;
  bool QuickReject(const DlFRect& bounds) const override;

  void DrawPaint(const DlPaint& paint) override;
  void DrawColor(DlColor color, DlBlendMode mode) override;
  void DrawLine(const DlFPoint& p0,
                const DlFPoint& p1,
                const DlPaint& paint) override;
  void DrawRect(const DlFRect& rect, const DlPaint& paint) override;
  void DrawOval(const DlFRect& bounds, const DlPaint& paint) override;
  void DrawCircle(const DlFPoint& center,
                  DlScalar radius,
                  const DlPaint& paint) override;
  void DrawRRect(const DlFRRect& rrect, const DlPaint& paint) override;
  void DrawDRRect(const DlFRRect& outer,
                  const DlFRRect& inner,
                  const DlPaint& paint) override;
  void DrawPath(const DlPath& path, const DlPaint& paint) override;
  void DrawArc(const DlFRect& bounds,
               DlScalar start,
               DlScalar sweep,
               bool useCenter,
               const DlPaint& paint) override;
  void DrawPoints(PointMode mode,
                  uint32_t count,
                  const DlFPoint pts[],
                  const DlPaint& paint) override;
  void DrawVertices(const DlVertices* vertices,
                    DlBlendMode mode,
                    const DlPaint& paint) override;

  void DrawImage(const sk_sp<DlImage>& image,
                 const DlFPoint point,
                 DlImageSampling sampling,
                 const DlPaint* paint = nullptr) override;
  void DrawImageRect(
      const sk_sp<DlImage>& image,
      const DlFRect& src,
      const DlFRect& dst,
      DlImageSampling sampling,
      const DlPaint* paint = nullptr,
      SrcRectConstraint constraint = SrcRectConstraint::kFast) override;
  void DrawImageNine(const sk_sp<DlImage>& image,
                     const DlIRect& center,
                     const DlFRect& dst,
                     DlFilterMode filter,
                     const DlPaint* paint = nullptr) override;
  void DrawAtlas(const sk_sp<DlImage>& atlas,
                 const DlRSTransform xform[],
                 const DlFRect tex[],
                 const DlColor colors[],
                 int count,
                 DlBlendMode mode,
                 DlImageSampling sampling,
                 const DlFRect* cullRect,
                 const DlPaint* paint = nullptr) override;

  void DrawDisplayList(const sk_sp<DisplayList> display_list,
                       DlScalar opacity) override;
  void DrawTextBlob(const sk_sp<SkTextBlob>& blob,
                    DlScalar x,
                    DlScalar y,
                    const DlPaint& paint) override;
  void DrawShadow(const DlPath& path,
                  const DlColor color,
                  const DlScalar elevation,
                  bool transparent_occluder,
                  DlScalar dpr) override;

  void Flush() override;

 private:
  DisplayListMatrixClipTracker tracker_;
  std::vector<DrawCall> draw_calls_;
  int current_layer_;
};

extern bool operator==(const MockCanvas::SaveData& a,
                       const MockCanvas::SaveData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::SaveData& data);
extern bool operator==(const MockCanvas::SaveLayerData& a,
                       const MockCanvas::SaveLayerData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::SaveLayerData& data);
extern bool operator==(const MockCanvas::RestoreData& a,
                       const MockCanvas::RestoreData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::RestoreData& data);
extern bool operator==(const MockCanvas::ConcatMatrixData& a,
                       const MockCanvas::ConcatMatrixData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::ConcatMatrixData& data);
extern bool operator==(const MockCanvas::SetMatrixData& a,
                       const MockCanvas::SetMatrixData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::SetMatrixData& data);
extern bool operator==(const MockCanvas::DrawRectData& a,
                       const MockCanvas::DrawRectData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawRectData& data);
extern bool operator==(const MockCanvas::DrawPathData& a,
                       const MockCanvas::DrawPathData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawPathData& data);
extern bool operator==(const MockCanvas::DrawTextData& a,
                       const MockCanvas::DrawTextData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawTextData& data);
extern bool operator==(const MockCanvas::DrawImageData& a,
                       const MockCanvas::DrawImageData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawImageData& data);
extern bool operator==(const MockCanvas::DrawImageDataNoPaint& a,
                       const MockCanvas::DrawImageDataNoPaint& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawImageDataNoPaint& data);
extern bool operator==(const MockCanvas::DrawDisplayListData& a,
                       const MockCanvas::DrawDisplayListData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawDisplayListData& data);
extern bool operator==(const MockCanvas::DrawShadowData& a,
                       const MockCanvas::DrawShadowData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawShadowData& data);
extern bool operator==(const MockCanvas::ClipRectData& a,
                       const MockCanvas::ClipRectData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::ClipRectData& data);
extern bool operator==(const MockCanvas::ClipRRectData& a,
                       const MockCanvas::ClipRRectData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::ClipRRectData& data);
extern bool operator==(const MockCanvas::ClipPathData& a,
                       const MockCanvas::ClipPathData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::ClipPathData& data);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawCallData& data);
extern bool operator==(const MockCanvas::DrawCall& a,
                       const MockCanvas::DrawCall& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawCall& draw);
extern bool operator==(const MockCanvas::DrawPaintData& a,
                       const MockCanvas::DrawPaintData& b);
extern std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::DrawPaintData& data);
}  // namespace testing
}  // namespace flutter

#endif  // TESTING_MOCK_CANVAS_H_
