// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/mock_canvas.h"

#include "flutter/fml/logging.h"
#include "flutter/testing/display_list_testing.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkSerialProcs.h"
#include "third_party/skia/include/core/SkTextBlob.h"

namespace flutter {
namespace testing {

MockCanvas::MockCanvas()
    : tracker_(DlFRect::MakeWH(64, 64), DlTransform()), current_layer_(0) {}

MockCanvas::MockCanvas(int width, int height)
    : tracker_(DlFRect::MakeWH(width, height), DlTransform()),
      current_layer_(0) {}

MockCanvas::~MockCanvas() {
  EXPECT_EQ(current_layer_, 0);
}

DlISize MockCanvas::GetBaseLayerSize() const {
  return DlIRect::MakeRoundedOut(tracker_.base_device_cull_rect()).size();
}

SkImageInfo MockCanvas::GetImageInfo() const {
  DlISize size = GetBaseLayerSize();
  return SkImageInfo::MakeUnknown(size.width(), size.height());
}

void MockCanvas::Save() {
  draw_calls_.emplace_back(
      DrawCall{current_layer_, SaveData{current_layer_ + 1}});
  tracker_.save();
  current_layer_++;  // Must go here; func params order of eval is undefined
}

void MockCanvas::SaveLayer(const DlFRect* bounds,
                           const DlPaint* paint,
                           const DlImageFilter* backdrop) {
  // saveLayer calls this prior to running, so we use it to track saveLayer
  // calls
  draw_calls_.emplace_back(DrawCall{
      current_layer_,
      SaveLayerData{bounds ? *bounds : DlFRect(), paint ? *paint : DlPaint(),
                    backdrop ? backdrop->shared() : nullptr,
                    current_layer_ + 1}});
  tracker_.save();
  current_layer_++;  // Must go here; func params order of eval is undefined
}

void MockCanvas::Restore() {
  FML_DCHECK(current_layer_ > 0);

  draw_calls_.emplace_back(
      DrawCall{current_layer_, RestoreData{current_layer_ - 1}});
  tracker_.restore();
  current_layer_--;  // Must go here; func params order of eval is undefined
}

// clang-format off

// 2x3 2D affine subset of a 4x4 transform in row major order
void MockCanvas::Transform2DAffine(DlScalar mxx, DlScalar mxy, DlScalar mxt,
                                   DlScalar myx, DlScalar myy, DlScalar myt) {
  Transform(DlTransform::MakeAffine2D(mxx, mxy, mxt, myx, myy, myt));
}

// full 4x4 transform in row major order
void MockCanvas::TransformFullPerspective(
    DlScalar mxx, DlScalar mxy, DlScalar mxz, DlScalar mxt,
    DlScalar myx, DlScalar myy, DlScalar myz, DlScalar myt,
    DlScalar mzx, DlScalar mzy, DlScalar mzz, DlScalar mzt,
    DlScalar mwx, DlScalar mwy, DlScalar mwz, DlScalar mwt) {
  Transform(DlTransform::MakeRowMajor(mxx, mxy, mxz, mxt,
                                      myx, myy, myz, myt,
                                      mzx, mzy, mzz, mzt,
                                      mwx, mwy, mwz, mwt));
}

// clang-format on

void MockCanvas::Transform(const DlTransform& matrix) {
  draw_calls_.emplace_back(DrawCall{current_layer_, ConcatMatrixData{matrix}});
  tracker_.transform(matrix);
}

void MockCanvas::SetTransform(const DlTransform& matrix) {
  draw_calls_.emplace_back(DrawCall{current_layer_, SetMatrixData{matrix}});
  tracker_.setTransform(matrix);
}

void MockCanvas::TransformReset() {
  draw_calls_.emplace_back(
      DrawCall{current_layer_, SetMatrixData{DlTransform()}});
  tracker_.setIdentity();
}

void MockCanvas::Translate(DlScalar x, DlScalar y) {
  this->Transform(DlTransform::MakeTranslate(x, y));
}

void MockCanvas::Scale(DlScalar x, DlScalar y) {
  this->Transform(DlTransform::MakeScale(x, y));
}

void MockCanvas::Rotate(DlAngle angle) {
  this->Transform(DlTransform::MakeRotate(angle));
}

void MockCanvas::Skew(DlScalar sx, DlScalar sy) {
  this->Transform(DlTransform::MakeSkew(sx, sy));
}

DlTransform MockCanvas::GetTransform() const {
  return tracker_.matrix();
}

void MockCanvas::DrawTextBlob(const sk_sp<SkTextBlob>& text,
                              DlScalar x,
                              DlScalar y,
                              const DlPaint& paint) {
  // This duplicates existing logic in SkCanvas::onDrawPicture
  // that should probably be split out so it doesn't need to be here as well.
  // SkRect storage;
  // if (paint.canComputeFastBounds()) {
  //   storage = text->bounds().makeOffset(x, y);
  //   SkRect tmp;
  //   if (this->quickReject(paint.computeFastBounds(storage, &tmp))) {
  //     return;
  //   }
  // }

  draw_calls_.emplace_back(DrawCall{
      current_layer_, DrawTextData{text ? text->serialize(SkSerialProcs{})
                                        : SkData::MakeUninitialized(0),
                                   paint, DlFPoint(x, y)}});
}

void MockCanvas::DrawRect(const DlFRect& rect, const DlPaint& paint) {
  draw_calls_.emplace_back(DrawCall{current_layer_, DrawRectData{rect, paint}});
}

void MockCanvas::DrawPath(const DlPath& path, const DlPaint& paint) {
  draw_calls_.emplace_back(DrawCall{current_layer_, DrawPathData{path, paint}});
}

void MockCanvas::DrawShadow(const DlPath& path,
                            const DlColor color,
                            const DlScalar elevation,
                            bool transparent_occluder,
                            DlScalar dpr) {
  draw_calls_.emplace_back(DrawCall{
      current_layer_,
      DrawShadowData{path, color, elevation, transparent_occluder, dpr}});
}

void MockCanvas::DrawImage(const sk_sp<DlImage>& image,
                           DlFPoint point,
                           const DlImageSampling options,
                           const DlPaint* paint) {
  if (paint) {
    draw_calls_.emplace_back(
        DrawCall{current_layer_,
                 DrawImageData{image, point.x(), point.y(), options, *paint}});
  } else {
    draw_calls_.emplace_back(
        DrawCall{current_layer_,
                 DrawImageDataNoPaint{image, point.x(), point.y(), options}});
  }
}

void MockCanvas::DrawDisplayList(const sk_sp<DisplayList> display_list,
                                 DlScalar opacity) {
  draw_calls_.emplace_back(
      DrawCall{current_layer_, DrawDisplayListData{display_list, opacity}});
}

void MockCanvas::ClipRect(const DlFRect& rect, ClipOp op, bool is_aa) {
  ClipEdgeStyle style = is_aa ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
  draw_calls_.emplace_back(
      DrawCall{current_layer_, ClipRectData{rect, op, style}});
  tracker_.clipRect(rect, op, is_aa);
}

void MockCanvas::ClipRRect(const DlFRRect& rrect, ClipOp op, bool is_aa) {
  ClipEdgeStyle style = is_aa ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
  draw_calls_.emplace_back(
      DrawCall{current_layer_, ClipRRectData{rrect, op, style}});
  tracker_.clipRRect(rrect, op, is_aa);
}

void MockCanvas::ClipPath(const DlPath& path, ClipOp op, bool is_aa) {
  ClipEdgeStyle style = is_aa ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
  draw_calls_.emplace_back(
      DrawCall{current_layer_, ClipPathData{path, op, style}});
  tracker_.clipPath(path, op, is_aa);
}

DlFRect MockCanvas::GetDestinationClipBounds() const {
  return tracker_.device_cull_rect();
}

DlFRect MockCanvas::GetLocalClipBounds() const {
  return tracker_.local_cull_rect();
}

bool MockCanvas::QuickReject(const DlFRect& bounds) const {
  return tracker_.content_culled(bounds);
}

void MockCanvas::DrawDRRect(const DlFRRect&, const DlFRRect&, const DlPaint&) {
  FML_DCHECK(false);
}

void MockCanvas::DrawPaint(const DlPaint& paint) {
  draw_calls_.emplace_back(DrawCall{current_layer_, DrawPaintData{paint}});
}

void MockCanvas::DrawColor(DlColor color, DlBlendMode mode) {
  DrawPaint(DlPaint(color).setBlendMode(mode));
}

void MockCanvas::DrawLine(const DlFPoint& p0,
                          const DlFPoint& p1,
                          const DlPaint& paint) {
  FML_DCHECK(false);
}

void MockCanvas::DrawPoints(PointMode,
                            uint32_t,
                            const DlFPoint[],
                            const DlPaint&) {
  FML_DCHECK(false);
}

void MockCanvas::DrawOval(const DlFRect&, const DlPaint&) {
  FML_DCHECK(false);
}

void MockCanvas::DrawCircle(const DlFPoint& center,
                            DlScalar radius,
                            const DlPaint& paint) {
  FML_DCHECK(false);
}

void MockCanvas::DrawArc(const DlFRect&,
                         DlScalar,
                         DlScalar,
                         bool,
                         const DlPaint&) {
  FML_DCHECK(false);
}

void MockCanvas::DrawRRect(const DlFRRect&, const DlPaint&) {
  FML_DCHECK(false);
}

void MockCanvas::DrawImageRect(const sk_sp<DlImage>&,
                               const DlFRect&,
                               const DlFRect&,
                               const DlImageSampling,
                               const DlPaint*,
                               SrcRectConstraint constraint) {
  FML_DCHECK(false);
}

void MockCanvas::DrawImageNine(const sk_sp<DlImage>& image,
                               const DlIRect& center,
                               const DlFRect& dst,
                               DlFilterMode filter,
                               const DlPaint* paint) {
  FML_DCHECK(false);
}

void MockCanvas::DrawVertices(const DlVertices*, DlBlendMode, const DlPaint&) {
  FML_DCHECK(false);
}

void MockCanvas::DrawAtlas(const sk_sp<DlImage>&,
                           const DlRSTransform[],
                           const DlFRect[],
                           const DlColor[],
                           int,
                           DlBlendMode,
                           const DlImageSampling,
                           const DlFRect*,
                           const DlPaint*) {
  FML_DCHECK(false);
}

void MockCanvas::Flush() {
  FML_DCHECK(false);
}

// --------------------------------------------------------
// A few ostream operators duplicated from assertions_skia.cc
// In the short term, there are issues trying to include that file
// here because it appears in a skia-targeted testing source set
// and in the long term, DlCanvas, and therefore this file will
// eventually be cleaned of these SkObject dependencies and these
// ostream operators will be converted to their DL equivalents.
static std::ostream& operator<<(std::ostream& os, const DlFPoint& r) {
  return os << "XY: " << r.x() << ", " << r.y();
}

static std::ostream& operator<<(std::ostream& os, const DlFRect& r) {
  return os << "LTRB: "                             //
            << r.left() << ", " << r.top() << ", "  //
            << r.right() << ", " << r.bottom();
}

static std::ostream& operator<<(std::ostream& os, const DlPath& r) {
  return os << "Valid: " << r.is_valid()
            << ", FillType: " << static_cast<int>(r.fill_type())
            << ", Bounds: " << r.Bounds();
}
// --------------------------------------------------------

bool operator==(const MockCanvas::SaveData& a, const MockCanvas::SaveData& b) {
  return a.save_to_layer == b.save_to_layer;
}

std::ostream& operator<<(std::ostream& os, const MockCanvas::SaveData& data) {
  return os << data.save_to_layer;
}

bool operator==(const MockCanvas::SaveLayerData& a,
                const MockCanvas::SaveLayerData& b) {
  return a.save_bounds == b.save_bounds && a.restore_paint == b.restore_paint &&
         Equals(a.backdrop_filter, b.backdrop_filter) &&
         a.save_to_layer == b.save_to_layer;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::SaveLayerData& data) {
  return os << data.save_bounds << " " << data.restore_paint << " "
            << data.backdrop_filter << " " << data.save_to_layer;
}

bool operator==(const MockCanvas::RestoreData& a,
                const MockCanvas::RestoreData& b) {
  return a.restore_to_layer == b.restore_to_layer;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::RestoreData& data) {
  return os << data.restore_to_layer;
}

bool operator==(const MockCanvas::ConcatMatrixData& a,
                const MockCanvas::ConcatMatrixData& b) {
  return a.matrix == b.matrix;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::ConcatMatrixData& data) {
  return os << data.matrix;
}

bool operator==(const MockCanvas::SetMatrixData& a,
                const MockCanvas::SetMatrixData& b) {
  return a.matrix == b.matrix;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::SetMatrixData& data) {
  return os << data.matrix;
}

bool operator==(const MockCanvas::DrawRectData& a,
                const MockCanvas::DrawRectData& b) {
  return a.rect == b.rect && a.paint == b.paint;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::DrawRectData& data) {
  return os << data.rect << " " << data.paint;
}

bool operator==(const MockCanvas::DrawPathData& a,
                const MockCanvas::DrawPathData& b) {
  return a.path == b.path && a.paint == b.paint;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::DrawPathData& data) {
  return os << data.path << " " << data.paint;
}

bool operator==(const MockCanvas::DrawTextData& a,
                const MockCanvas::DrawTextData& b) {
  return a.serialized_text->equals(b.serialized_text.get()) &&
         a.paint == b.paint && a.offset == b.offset;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::DrawTextData& data) {
  return os << data.serialized_text << " " << data.paint << " " << data.offset;
}

bool operator==(const MockCanvas::DrawImageData& a,
                const MockCanvas::DrawImageData& b) {
  return a.image == b.image && a.x == b.x && a.y == b.y &&
         a.options == b.options && a.paint == b.paint;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::DrawImageData& data) {
  return os << data.image << " " << data.x << " " << data.y << " "
            << data.options << " " << data.paint;
}

bool operator==(const MockCanvas::DrawImageDataNoPaint& a,
                const MockCanvas::DrawImageDataNoPaint& b) {
  return a.image == b.image && a.x == b.x && a.y == b.y &&
         a.options == b.options;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::DrawImageDataNoPaint& data) {
  return os << data.image << " " << data.x << " " << data.y << " "
            << data.options;
}

bool operator==(const MockCanvas::DrawDisplayListData& a,
                const MockCanvas::DrawDisplayListData& b) {
  return a.display_list->Equals(b.display_list) && a.opacity == b.opacity;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::DrawDisplayListData& data) {
  auto dl = data.display_list;
  return os << "[" << dl->unique_id() << " " << dl->op_count() << " "
            << dl->bytes() << "] " << data.opacity;
}

bool operator==(const MockCanvas::DrawShadowData& a,
                const MockCanvas::DrawShadowData& b) {
  return a.path == b.path && a.color == b.color && a.elevation == b.elevation &&
         a.transparent_occluder == b.transparent_occluder && a.dpr == b.dpr;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::DrawShadowData& data) {
  return os << data.path << " " << data.color << " " << data.elevation << " "
            << data.transparent_occluder << " " << data.dpr;
}

bool operator==(const MockCanvas::ClipRectData& a,
                const MockCanvas::ClipRectData& b) {
  return a.rect == b.rect && a.clip_op == b.clip_op && a.style == b.style;
}

static std::ostream& operator<<(std::ostream& os,
                                const MockCanvas::ClipEdgeStyle& style) {
  return os << (style == MockCanvas::kSoft_ClipEdgeStyle ? "kSoftEdges"
                                                         : "kHardEdges");
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::ClipRectData& data) {
  return os << data.rect << " " << data.clip_op << " " << data.style;
}

bool operator==(const MockCanvas::ClipRRectData& a,
                const MockCanvas::ClipRRectData& b) {
  return a.rrect == b.rrect && a.clip_op == b.clip_op && a.style == b.style;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::ClipRRectData& data) {
  return os << data.rrect << " " << data.clip_op << " " << data.style;
}

bool operator==(const MockCanvas::ClipPathData& a,
                const MockCanvas::ClipPathData& b) {
  return a.path == b.path && a.clip_op == b.clip_op && a.style == b.style;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::ClipPathData& data) {
  return os << data.path << " " << data.clip_op << " " << data.style;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::DrawCallData& data) {
  std::visit([&os](auto& d) { os << d; }, data);
  return os;
}

bool operator==(const MockCanvas::DrawCall& a, const MockCanvas::DrawCall& b) {
  return a.layer == b.layer && a.data == b.data;
}

std::ostream& operator<<(std::ostream& os, const MockCanvas::DrawCall& draw) {
  return os << "[Layer: " << draw.layer << ", Data: " << draw.data << "]";
}

bool operator==(const MockCanvas::DrawPaintData& a,
                const MockCanvas::DrawPaintData& b) {
  return a.paint == b.paint;
}

std::ostream& operator<<(std::ostream& os,
                         const MockCanvas::DrawPaintData& data) {
  return os << data.paint;
}

}  // namespace testing
}  // namespace flutter
