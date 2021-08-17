// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_DISPLAY_LIST_H_
#define FLUTTER_FLOW_DISPLAY_LIST_H_

#include "third_party/skia/include/core/SkBlender.h"
#include "third_party/skia/include/core/SkBlurTypes.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/core/SkPathEffect.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/core/SkVertices.h"

// The Flutter DisplayList mechanism encapsulates a persistent sequence of
// rendering operations.
//
// This file contains the definitions for:
// DisplayList: the base class that holds the information about the
//              sequence of operations and can dispatch them to a Dispatcher
// Dispatcher: a pure virtual interface which can be implemented to field
//             the requests for purposes such as sending them to an SkCanvas
//             or detecting various rendering optimization scenarios
// DisplayListBuilder: a class for constructing a DisplayList from the same
//                     calls defined in the Dispatcher
//
// Other files include various class definitions for dealing with display
// lists, such as:
// display_list_canvas.h: classes to interact between SkCanvas and DisplayList
//                        (SkCanvas->DisplayList adapter and vice versa)
//
// display_list_utils.h: various utility classes to ease implementing
//                       a Dispatcher, including NOP implementations of
//                       the attribute, clip, and transform methods,
//                       classes to track attributes, clips, and transforms
//                       and a class to compute the bounds of a DisplayList
//                       Any class implementing Dispatcher can inherit from
//                       these utility classes to simplify its creation
//
// The Flutter DisplayList mechanism can be used in place of the Skia
// SkPicture mechanism. The primary means of communication into and out
// of the DisplayList is through the Dispatcher virtual class which
// provides a nearly 1:1 translation between the records of the DisplayList
// to method calls.
//
// A DisplayList can be created directly using a DisplayListBuilder and
// the Dispatcher methods that it implements, or it can be created from
// a sequence of SkCanvas calls using the DisplayListCanvasRecorder class.
//
// A DisplayList can be read back by implementing the Dispatcher virtual
// methods (with help from some of the classes in the utils file) and
// passing an instance to the dispatch() method, or it can be rendered
// to Skia using a DisplayListCanvasDispatcher or simply by passing an
// SkCanvas pointer to its renderTo() method.
//
// The mechanism is inspired by the SkLiteDL class that is not directly
// supported by Skia, but has been recommended as a basis for custom
// display lists for a number of their customers.

namespace flutter {

#define FOR_EACH_DISPLAY_LIST_OP(V) \
  V(SetAA)                          \
  V(SetDither)                      \
  V(SetInvertColors)                \
                                    \
  V(SetCaps)                        \
  V(SetJoins)                       \
                                    \
  V(SetDrawStyle)                   \
  V(SetStrokeWidth)                 \
  V(SetMiterLimit)                  \
                                    \
  V(SetColor)                       \
  V(SetBlendMode)                   \
                                    \
  V(SetBlender)                     \
  V(ClearBlender)                   \
  V(SetShader)                      \
  V(ClearShader)                    \
  V(SetColorFilter)                 \
  V(ClearColorFilter)               \
  V(SetImageFilter)                 \
  V(ClearImageFilter)               \
  V(SetPathEffect)                  \
  V(ClearPathEffect)                \
                                    \
  V(ClearMaskFilter)                \
  V(SetMaskFilter)                  \
  V(SetMaskBlurFilterNormal)        \
  V(SetMaskBlurFilterSolid)         \
  V(SetMaskBlurFilterOuter)         \
  V(SetMaskBlurFilterInner)         \
                                    \
  V(Save)                           \
  V(SaveLayer)                      \
  V(SaveLayerBounds)                \
  V(Restore)                        \
                                    \
  V(Translate)                      \
  V(Scale)                          \
  V(Rotate)                         \
  V(Skew)                           \
  V(Transform2x3)                   \
  V(Transform3x3)                   \
                                    \
  V(ClipIntersectRect)              \
  V(ClipIntersectRRect)             \
  V(ClipIntersectPath)              \
  V(ClipDifferenceRect)             \
  V(ClipDifferenceRRect)            \
  V(ClipDifferencePath)             \
                                    \
  V(DrawPaint)                      \
  V(DrawColor)                      \
                                    \
  V(DrawLine)                       \
  V(DrawRect)                       \
  V(DrawOval)                       \
  V(DrawCircle)                     \
  V(DrawRRect)                      \
  V(DrawDRRect)                     \
  V(DrawArc)                        \
  V(DrawPath)                       \
                                    \
  V(DrawPoints)                     \
  V(DrawLines)                      \
  V(DrawPolygon)                    \
  V(DrawVertices)                   \
                                    \
  V(DrawImage)                      \
  V(DrawImageWithAttr)              \
  V(DrawImageRect)                  \
  V(DrawImageNine)                  \
  V(DrawImageNineWithAttr)          \
  V(DrawImageLattice)               \
  V(DrawAtlas)                      \
  V(DrawAtlasCulled)                \
                                    \
  V(DrawSkPicture)                  \
  V(DrawSkPictureMatrix)            \
  V(DrawDisplayList)                \
  V(DrawTextBlob)                   \
                                    \
  V(DrawShadow)                     \
  V(DrawShadowOccludes)

#define DL_OP_TO_ENUM_VALUE(name) k##name,
enum class DisplayListOpType { FOR_EACH_DISPLAY_LIST_OP(DL_OP_TO_ENUM_VALUE) };
#undef DL_OP_TO_ENUM_VALUE

class Dispatcher;
class DisplayListBuilder;

// The base class that contains a sequence of rendering operations
// for dispatch to a Dispatcher. These objects must be instantiated
// through an instance of DisplayListBuilder::build().
class DisplayList : public SkRefCnt {
 public:
  static const SkSamplingOptions NearestSampling;
  static const SkSamplingOptions LinearSampling;
  static const SkSamplingOptions MipmapSampling;
  static const SkSamplingOptions CubicSampling;

  DisplayList()
      : used_(0),
        op_count_(0),
        unique_id_(0),
        bounds_({0, 0, 0, 0}),
        bounds_cull_({0, 0, 0, 0}) {}

  ~DisplayList();

  void Dispatch(Dispatcher& ctx) const {
    uint8_t* ptr = storage_.get();
    Dispatch(ctx, ptr, ptr + used_);
  }

  void RenderTo(SkCanvas* canvas) const;

  size_t bytes() const { return used_; }
  int op_count() const { return op_count_; }
  uint32_t unique_id() const { return unique_id_; }

  const SkRect& bounds() {
    if (bounds_.width() < 0.0) {
      // ComputeBounds() will leave the variable with a
      // non-negative width and height
      ComputeBounds();
    }
    return bounds_;
  }

  bool Equals(const DisplayList& other) const;

 private:
  DisplayList(uint8_t* ptr, size_t used, int op_count, const SkRect& cull_rect);

  std::unique_ptr<uint8_t, SkFunctionWrapper<void(void*), sk_free>> storage_;
  size_t used_;
  int op_count_;

  uint32_t unique_id_;
  SkRect bounds_;

  // Only used for drawPaint() and drawColor()
  SkRect bounds_cull_;

  void ComputeBounds();
  void Dispatch(Dispatcher& ctx, uint8_t* ptr, uint8_t* end) const;

  friend class DisplayListBuilder;
};

// The pure virtual interface for interacting with a display list.
// This interface represents the methods used to build a list
// through the DisplayListBuilder and also the methods that will
// be invoked through the DisplayList::dispatch() method.
class Dispatcher {
 public:
  // MaxDrawPointsCount * sizeof(SkPoint) must be less than 1 << 32
  static constexpr int kMaxDrawPointsCount = ((1 << 29) - 1);

  virtual void setAA(bool aa) = 0;
  virtual void setDither(bool dither) = 0;
  virtual void setInvertColors(bool invert) = 0;
  virtual void setCaps(SkPaint::Cap cap) = 0;
  virtual void setJoins(SkPaint::Join join) = 0;
  virtual void setDrawStyle(SkPaint::Style style) = 0;
  virtual void setStrokeWidth(SkScalar width) = 0;
  virtual void setMiterLimit(SkScalar limit) = 0;
  virtual void setColor(SkColor color) = 0;
  virtual void setBlendMode(SkBlendMode mode) = 0;
  virtual void setBlender(sk_sp<SkBlender> blender) = 0;
  virtual void setShader(sk_sp<SkShader> shader) = 0;
  virtual void setImageFilter(sk_sp<SkImageFilter> filter) = 0;
  virtual void setColorFilter(sk_sp<SkColorFilter> filter) = 0;
  virtual void setPathEffect(sk_sp<SkPathEffect> effect) = 0;
  virtual void setMaskFilter(sk_sp<SkMaskFilter> filter) = 0;
  virtual void setMaskBlurFilter(SkBlurStyle style, SkScalar sigma) = 0;

  virtual void save() = 0;
  virtual void restore() = 0;
  virtual void saveLayer(const SkRect* bounds, bool restore_with_paint) = 0;

  virtual void translate(SkScalar tx, SkScalar ty) = 0;
  virtual void scale(SkScalar sx, SkScalar sy) = 0;
  virtual void rotate(SkScalar degrees) = 0;
  virtual void skew(SkScalar sx, SkScalar sy) = 0;
  virtual void transform2x3(SkScalar mxx,
                            SkScalar mxy,
                            SkScalar mxt,
                            SkScalar myx,
                            SkScalar myy,
                            SkScalar myt) = 0;
  virtual void transform3x3(SkScalar mxx,
                            SkScalar mxy,
                            SkScalar mxt,
                            SkScalar myx,
                            SkScalar myy,
                            SkScalar myt,
                            SkScalar px,
                            SkScalar py,
                            SkScalar pt) = 0;

  virtual void clipRect(const SkRect& rect, SkClipOp clip_op, bool is_aa) = 0;
  virtual void clipRRect(const SkRRect& rrect,
                         SkClipOp clip_op,
                         bool is_aa) = 0;
  virtual void clipPath(const SkPath& path, SkClipOp clip_op, bool is_aa) = 0;

  virtual void drawPaint() = 0;
  virtual void drawColor(SkColor color, SkBlendMode mode) = 0;
  virtual void drawLine(const SkPoint& p0, const SkPoint& p1) = 0;
  virtual void drawRect(const SkRect& rect) = 0;
  virtual void drawOval(const SkRect& bounds) = 0;
  virtual void drawCircle(const SkPoint& center, SkScalar radius) = 0;
  virtual void drawRRect(const SkRRect& rrect) = 0;
  virtual void drawDRRect(const SkRRect& outer, const SkRRect& inner) = 0;
  virtual void drawPath(const SkPath& path) = 0;
  virtual void drawArc(const SkRect& bounds,
                       SkScalar start,
                       SkScalar sweep,
                       bool useCenter) = 0;
  virtual void drawPoints(SkCanvas::PointMode mode,
                          uint32_t count,
                          const SkPoint pts[]) = 0;
  virtual void drawVertices(const sk_sp<SkVertices> vertices,
                            SkBlendMode mode) = 0;
  virtual void drawImage(const sk_sp<SkImage> image,
                         const SkPoint point,
                         const SkSamplingOptions& sampling,
                         bool render_with_attributes) = 0;
  virtual void drawImageRect(const sk_sp<SkImage> image,
                             const SkRect& src,
                             const SkRect& dst,
                             const SkSamplingOptions& sampling,
                             bool render_with_attributes,
                             SkCanvas::SrcRectConstraint constraint) = 0;
  virtual void drawImageNine(const sk_sp<SkImage> image,
                             const SkIRect& center,
                             const SkRect& dst,
                             SkFilterMode filter,
                             bool render_with_attributes) = 0;
  virtual void drawImageLattice(const sk_sp<SkImage> image,
                                const SkCanvas::Lattice& lattice,
                                const SkRect& dst,
                                SkFilterMode filter,
                                bool render_with_attributes) = 0;
  virtual void drawAtlas(const sk_sp<SkImage> atlas,
                         const SkRSXform xform[],
                         const SkRect tex[],
                         const SkColor colors[],
                         int count,
                         SkBlendMode mode,
                         const SkSamplingOptions& sampling,
                         const SkRect* cullRect,
                         bool render_with_attributes) = 0;
  virtual void drawPicture(const sk_sp<SkPicture> picture,
                           const SkMatrix* matrix,
                           bool render_with_attributes) = 0;
  virtual void drawDisplayList(const sk_sp<DisplayList> display_list) = 0;
  virtual void drawTextBlob(const sk_sp<SkTextBlob> blob,
                            SkScalar x,
                            SkScalar y) = 0;
  virtual void drawShadow(const SkPath& path,
                          const SkColor color,
                          const SkScalar elevation,
                          bool occludes,
                          SkScalar dpr) = 0;
};

// The primary class used to build a display list. The list of methods
// here matches the list of methods invoked during dispatch().
// If there is some code that already renders to an SkCanvas object,
// those rendering commands can be captured into a DisplayList using
// the DisplayListCanvasRecorder class.
class DisplayListBuilder final : public virtual Dispatcher, public SkRefCnt {
 public:
  DisplayListBuilder(const SkRect& cull = kMaxCull_);
  ~DisplayListBuilder();

  void setAA(bool aa) override {
    if (current_aa_ != aa) {
      onSetAA(aa);
    }
  }
  void setDither(bool dither) override {
    if (current_dither_ != dither) {
      onSetDither(dither);
    }
  }
  void setInvertColors(bool invert) override {
    if (current_invert_colors_ != invert) {
      onSetInvertColors(invert);
    }
  }
  void setCaps(SkPaint::Cap cap) override {
    if (current_cap_ != cap) {
      onSetCaps(cap);
    }
  }
  void setJoins(SkPaint::Join join) override {
    if (current_join_ != join) {
      onSetJoins(join);
    }
  }
  void setDrawStyle(SkPaint::Style style) override {
    if (current_style_ != style) {
      onSetDrawStyle(style);
    }
  }
  void setStrokeWidth(SkScalar width) override {
    if (current_stroke_width_ != width) {
      onSetStrokeWidth(width);
    }
  }
  void setMiterLimit(SkScalar limit) override {
    if (current_miter_limit_ != limit) {
      onSetMiterLimit(limit);
    }
  }
  void setColor(SkColor color) override {
    if (current_color_ != color) {
      onSetColor(color);
    }
  }
  void setBlendMode(SkBlendMode mode) override {
    if (current_blender_ || current_blend_ != mode) {
      onSetBlendMode(mode);
    }
  }
  void setBlender(sk_sp<SkBlender> blender) override {
    if (!blender) {
      setBlendMode(SkBlendMode::kSrcOver);
    } else if (current_blender_ != blender) {
      onSetBlender(std::move(blender));
    }
  }
  void setShader(sk_sp<SkShader> shader) override {
    if (current_shader_ != shader) {
      onSetShader(std::move(shader));
    }
  }
  void setImageFilter(sk_sp<SkImageFilter> filter) override {
    if (current_image_filter_ != filter) {
      onSetImageFilter(std::move(filter));
    }
  }
  void setColorFilter(sk_sp<SkColorFilter> filter) override {
    if (current_color_filter_ != filter) {
      onSetColorFilter(std::move(filter));
    }
  }
  void setPathEffect(sk_sp<SkPathEffect> effect) override {
    if (current_path_effect_ != effect) {
      onSetPathEffect(std::move(effect));
    }
  }
  void setMaskFilter(sk_sp<SkMaskFilter> filter) override {
    if (mask_sigma_valid(current_mask_sigma_) || current_mask_filter_ != filter) {
      onSetMaskFilter(std::move(filter));
    }
  }
  void setMaskBlurFilter(SkBlurStyle style, SkScalar sigma) override {
    if (mask_sigma_valid(sigma) &&
        (current_mask_sigma_ != sigma || current_mask_style_ != style)) {
      onSetMaskBlurFilter(style, sigma);
    }
  }

  void setAttributesFromPaint(const SkPaint* paint, int attribute_mask);

  void save() override;
  void restore() override;
  void saveLayer(const SkRect* bounds, bool restore_with_paint) override;
  int getSaveCount() const { return save_level_ + 1; }

  void translate(SkScalar tx, SkScalar ty) override;
  void scale(SkScalar sx, SkScalar sy) override;
  void rotate(SkScalar degrees) override;
  void skew(SkScalar sx, SkScalar sy) override;
  void transform2x3(SkScalar mxx,
                    SkScalar mxy,
                    SkScalar mxt,
                    SkScalar myx,
                    SkScalar myy,
                    SkScalar myt) override;
  void transform3x3(SkScalar mxx,
                    SkScalar mxy,
                    SkScalar mxt,
                    SkScalar myx,
                    SkScalar myy,
                    SkScalar myt,
                    SkScalar px,
                    SkScalar py,
                    SkScalar pt) override;

  void clipRect(const SkRect& rect, SkClipOp clip_op, bool is_aa) override;
  void clipRRect(const SkRRect& rrect, SkClipOp clip_op, bool is_aa) override;
  void clipPath(const SkPath& path, SkClipOp clip_op, bool is_aa) override;

  void drawPaint() override;
  void drawColor(SkColor color, SkBlendMode mode) override;
  void drawLine(const SkPoint& p0, const SkPoint& p1) override;
  void drawRect(const SkRect& rect) override;
  void drawOval(const SkRect& bounds) override;
  void drawCircle(const SkPoint& center, SkScalar radius) override;
  void drawRRect(const SkRRect& rrect) override;
  void drawDRRect(const SkRRect& outer, const SkRRect& inner) override;
  void drawPath(const SkPath& path) override;
  void drawArc(const SkRect& bounds,
               SkScalar start,
               SkScalar sweep,
               bool useCenter) override;
  void drawPoints(SkCanvas::PointMode mode,
                  uint32_t count,
                  const SkPoint pts[]) override;
  void drawVertices(const sk_sp<SkVertices> vertices,
                    SkBlendMode mode) override;
  void drawImage(const sk_sp<SkImage> image,
                 const SkPoint point,
                 const SkSamplingOptions& sampling,
                 bool render_with_attributes) override;
  void drawImageRect(const sk_sp<SkImage> image,
                     const SkRect& src,
                     const SkRect& dst,
                     const SkSamplingOptions& sampling,
                     bool render_with_attributes,
                     SkCanvas::SrcRectConstraint constraint) override;
  void drawImageNine(const sk_sp<SkImage> image,
                     const SkIRect& center,
                     const SkRect& dst,
                     SkFilterMode filter,
                     bool render_with_attributes) override;
  void drawImageLattice(const sk_sp<SkImage> image,
                        const SkCanvas::Lattice& lattice,
                        const SkRect& dst,
                        SkFilterMode filter,
                        bool render_with_attributes) override;
  void drawAtlas(const sk_sp<SkImage> atlas,
                 const SkRSXform xform[],
                 const SkRect tex[],
                 const SkColor colors[],
                 int count,
                 SkBlendMode mode,
                 const SkSamplingOptions& sampling,
                 const SkRect* cullRect,
                 bool render_with_attributes) override;
  void drawPicture(const sk_sp<SkPicture> picture,
                   const SkMatrix* matrix,
                   bool render_with_attributes) override;
  void drawDisplayList(const sk_sp<DisplayList> display_list) override;
  void drawTextBlob(const sk_sp<SkTextBlob> blob,
                    SkScalar x,
                    SkScalar y) override;
  void drawShadow(const SkPath& path,
                  const SkColor color,
                  const SkScalar elevation,
                  bool occludes,
                  SkScalar dpr) override;

  sk_sp<DisplayList> Build();

  // Mask bits for the various attributes that might be needed for a given
  // operation.
  // clang-format off
  static constexpr int kAaNeeded            = 1 << 0;
  static constexpr int kColorNeeded         = 1 << 1;
  static constexpr int kBlendNeeded         = 1 << 2;
  static constexpr int kInvertColorsNeeded  = 1 << 3;
  static constexpr int kPaintStyleNeeded    = 1 << 4;
  static constexpr int kStrokeStyleNeeded   = 1 << 5;
  static constexpr int kShaderNeeded        = 1 << 6;
  static constexpr int kColorFilterNeeded   = 1 << 7;
  static constexpr int kImageFilterNeeded   = 1 << 8;
  static constexpr int kPathEffectNeeded    = 1 << 9;
  static constexpr int kMaskFilterNeeded    = 1 << 10;
  static constexpr int kDitherNeeded        = 1 << 11;
  // clang-format on

  static constexpr int kAllAttributesMask =
      kAaNeeded | kColorNeeded | kBlendNeeded | kInvertColorsNeeded |
      kPaintStyleNeeded | kStrokeStyleNeeded | kShaderNeeded |
      kColorFilterNeeded | kImageFilterNeeded | kPathEffectNeeded |
      kMaskFilterNeeded | kDitherNeeded;

 private:
  // Combinations of the above mask bits that are common to typical "draw"
  // calls.
  // Note that the strokeStyle_ is handled conditionally depending on whether
  // the paintStyle_ attribute value is synchronized. It can also be manually
  // specified for operations that will be always stroking, like [drawLine].
  static constexpr int kPaintMask_ =
      kAaNeeded | kColorNeeded | kBlendNeeded | kInvertColorsNeeded |
      kColorFilterNeeded | kShaderNeeded | kDitherNeeded | kImageFilterNeeded;
  static constexpr int kDrawMask_ =
      kPaintMask_ | kPaintStyleNeeded | kMaskFilterNeeded | kPathEffectNeeded;
  static constexpr int kStrokeMask_ =
      kPaintMask_ | kStrokeStyleNeeded | kMaskFilterNeeded | kPathEffectNeeded;
  static constexpr int kImageMask_ =
      kColorNeeded | kBlendNeeded | kInvertColorsNeeded | kColorFilterNeeded |
      kDitherNeeded | kImageFilterNeeded | kMaskFilterNeeded;
  static constexpr int kImageRectMask_ = kImageMask_ | kAaNeeded;
  static constexpr int kSaveLayerMask_ =
      kColorNeeded | kBlendNeeded | kInvertColorsNeeded | kColorFilterNeeded |
      kImageFilterNeeded;

 public:
  static constexpr int kSaveLayerMask = kSaveLayerMask_;

  static constexpr int kDrawPaintMask = kPaintMask_;
  // static constexpr int kDrawColorMask = 0;

  static constexpr int kDrawLineMask = kStrokeMask_;
  static constexpr int kDrawRectMask = kDrawMask_;
  static constexpr int kDrawOvalMask = kDrawMask_;
  static constexpr int kDrawCircleMask = kDrawMask_;
  static constexpr int kDrawRRectMask = kDrawMask_;
  static constexpr int kDrawDRRectMask = kDrawMask_;
  static constexpr int kDrawArcMask = kDrawMask_;
  static constexpr int kDrawPathMask = kDrawMask_;

  static constexpr int kDrawPointsMask = kStrokeMask_;
  static constexpr int kDrawLinesMask = kStrokeMask_;
  static constexpr int kDrawPolygonMask = kStrokeMask_;
  static constexpr int kDrawVerticesMask = kDrawMask_;

  static constexpr int kDrawImageMask = kImageMask_;
  static constexpr int kDrawImageRectMask = kImageRectMask_;
  static constexpr int kDrawImageNineMask = kImageMask_;
  static constexpr int kDrawImageLatticeMask = kImageMask_;
  static constexpr int kDrawAtlasMask = kImageMask_;

  static constexpr int kDrawSkPictureMask = kSaveLayerMask_;
  // static constexpr int kDrawDisplayListMask = 0;
  static constexpr int kDrawTextBlobMask = kDrawMask_;

  // static constexpr int kDrawShadowMask = 0;
  // static constexpr int kDrawShadowOccludesMask = 0;

 private:
  SkAutoTMalloc<uint8_t> storage_;
  size_t used_ = 0;
  size_t allocated_ = 0;
  int op_count_ = 0;
  int save_level_ = 0;

  SkRect cull_;
  static constexpr SkRect kMaxCull_ =
      SkRect::MakeLTRB(-1E9F, -1E9F, 1E9F, 1E9F);

  template <typename T, typename... Args>
  void* Push(size_t extra, Args&&... args);

  // kInvalidSigma is used to indicate that no MaskBlur is currently set.
  static constexpr SkScalar kInvalidSigma = 0.0;
  bool mask_sigma_valid(SkScalar sigma) {
    return SkScalarIsFinite(sigma) && sigma > 0.0;
  }

  void onSetAA(bool aa);
  void onSetDither(bool dither);
  void onSetInvertColors(bool invert);
  void onSetCaps(SkPaint::Cap cap);
  void onSetJoins(SkPaint::Join join);
  void onSetDrawStyle(SkPaint::Style style);
  void onSetStrokeWidth(SkScalar width);
  void onSetMiterLimit(SkScalar limit);
  void onSetColor(SkColor color);
  void onSetBlendMode(SkBlendMode mode);
  void onSetBlender(sk_sp<SkBlender> blender);
  void onSetShader(sk_sp<SkShader> shader);
  void onSetImageFilter(sk_sp<SkImageFilter> filter);
  void onSetColorFilter(sk_sp<SkColorFilter> filter);
  void onSetPathEffect(sk_sp<SkPathEffect> effect);
  void onSetMaskFilter(sk_sp<SkMaskFilter> filter);
  void onSetMaskBlurFilter(SkBlurStyle style, SkScalar sigma);

  // These values should match the defaults of the Dart Paint object.
  bool current_aa_ = false;
  bool current_dither_ = false;
  bool current_invert_colors_ = false;
  SkColor current_color_ = 0xFF000000;
  SkBlendMode current_blend_ = SkBlendMode::kSrcOver;
  SkPaint::Style current_style_ = SkPaint::Style::kFill_Style;
  SkScalar current_stroke_width_ = 0.0;
  SkScalar current_miter_limit_ = 4.0;
  SkPaint::Cap current_cap_ = SkPaint::Cap::kButt_Cap;
  SkPaint::Join current_join_ = SkPaint::Join::kMiter_Join;
  sk_sp<SkBlender> current_blender_;
  sk_sp<SkShader> current_shader_;
  sk_sp<SkColorFilter> current_color_filter_;
  sk_sp<SkImageFilter> current_image_filter_;
  sk_sp<SkPathEffect> current_path_effect_;
  int current_mask_style_;
  sk_sp<SkMaskFilter> current_mask_filter_;
  SkScalar current_mask_sigma_ = kInvalidSigma;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_DISPLAY_LIST_H_
