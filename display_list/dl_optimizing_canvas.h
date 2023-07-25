// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/display_list/dl_canvas.h"

namespace flutter {

class DlOptimizingCanvas : public virtual DlCanvas {
 public:
  DlOptimizingCanvas(const SkRect& cull_rect, bool prepare_rtree = false);
  virtual ~DlOptimizingCanvas() = default;

  virtual SkISize GetBaseLayerSize() const override;

  virtual void Save() override;
  virtual void SaveLayer(const SkRect* bounds,
                         const DlPaint* paint = nullptr,
                         const DlImageFilter* backdrop = nullptr) override;
  virtual void Restore() override;
  virtual void RestoreToCount(int restore_count) override;

  virtual void Translate(SkScalar tx, SkScalar ty) override;
  virtual void Scale(SkScalar sx, SkScalar sy) override;
  virtual void Rotate(SkScalar degrees) override;
  virtual void Skew(SkScalar sx, SkScalar sy) override;

  // clang-format off

  // 2x3 2D affine subset of a 4x4 transform in row major order
  virtual void Transform2DAffine(SkScalar mxx, SkScalar mxy, SkScalar mxt,
                                 SkScalar myx, SkScalar myy, SkScalar myt) override;
  // full 4x4 transform in row major order
  virtual void TransformFullPerspective(
      SkScalar mxx, SkScalar mxy, SkScalar mxz, SkScalar mxt,
      SkScalar myx, SkScalar myy, SkScalar myz, SkScalar myt,
      SkScalar mzx, SkScalar mzy, SkScalar mzz, SkScalar mzt,
      SkScalar mwx, SkScalar mwy, SkScalar mwz, SkScalar mwt) override;
  // clang-format on
  virtual void TransformReset() override;
  virtual void Transform(const SkMatrix* matrix) override;
  virtual void Transform(const SkM44* matrix44) override;
  virtual void Transform(const SkMatrix& matrix) { Transform(&matrix); }
  virtual void Transform(const SkM44& matrix44) { Transform(&matrix44); }
  virtual void SetTransform(const SkMatrix* matrix) override;
  virtual void SetTransform(const SkM44* matrix44) override;
  virtual void SetTransform(const SkMatrix& matrix) { SetTransform(&matrix); }
  virtual void SetTransform(const SkM44& matrix44) { SetTransform(&matrix44); }

  /// Returns the 4x4 full perspective transform representing all transform
  /// operations executed so far in this DisplayList within the enclosing
  /// save stack.
  virtual SkM44 GetTransformFullPerspective() const override;
  /// Returns the 3x3 partial perspective transform representing all transform
  /// operations executed so far in this DisplayList within the enclosing
  /// save stack.
  virtual SkMatrix GetTransform() const override;

  virtual void ClipRect(const SkRect& rect,
                        ClipOp clip_op = ClipOp::kIntersect,
                        bool is_aa = false) override;
  virtual void ClipRRect(const SkRRect& rrect,
                         ClipOp clip_op = ClipOp::kIntersect,
                         bool is_aa = false) override;
  virtual void ClipPath(const SkPath& path,
                        ClipOp clip_op = ClipOp::kIntersect,
                        bool is_aa = false) override;

  /// Conservative estimate of the bounds of all outstanding clip operations
  /// measured in the coordinate space within which this DisplayList will
  /// be rendered.
  virtual SkRect GetDestinationClipBounds() const override;
  /// Conservative estimate of the bounds of all outstanding clip operations
  /// transformed into the local coordinate space in which currently
  /// recorded rendering operations are interpreted.
  virtual SkRect GetLocalClipBounds() const override;

  /// Return true iff the supplied bounds are easily shown to be outside
  /// of the current clip bounds. This method may conservatively return
  /// false if it cannot make the determination.
  virtual bool QuickReject(const SkRect& bounds) const override;

  virtual void DrawPaint(const DlPaint& paint) override;
  virtual void DrawColor(DlColor color,
                         DlBlendMode mode = DlBlendMode::kSrcOver) override;

  virtual void DrawLine(const SkPoint& p0,
                        const SkPoint& p1,
                        const DlPaint& paint) override;
  virtual void DrawRect(const SkRect& rect, const DlPaint& paint) override;
  virtual void DrawOval(const SkRect& bounds, const DlPaint& paint) override;
  virtual void DrawCircle(const SkPoint& center,
                          SkScalar radius,
                          const DlPaint& paint);
  virtual void DrawRRect(const SkRRect& rrect, const DlPaint& paint) override;
  virtual void DrawDRRect(const SkRRect& outer,
                          const SkRRect& inner,
                          const DlPaint& paint) override;
  virtual void DrawPath(const SkPath& path, const DlPaint& paint) override;
  virtual void DrawArc(const SkRect& bounds,
                       SkScalar start,
                       SkScalar sweep,
                       bool useCenter,
                       const DlPaint& paint) override;
  virtual void DrawPoints(PointMode mode,
                          uint32_t count,
                          const SkPoint pts[],
                          const DlPaint& paint) override;
  virtual void DrawVertices(const DlVertices* vertices,
                            DlBlendMode mode,
                            const DlPaint& paint) override;
  virtual void DrawImage(const sk_sp<DlImage>& image,
                         const SkPoint point,
                         DlImageSampling sampling,
                         const DlPaint* paint = nullptr) override;
  virtual void DrawImageRect(
      const sk_sp<DlImage>& image,
      const SkRect& src,
      const SkRect& dst,
      DlImageSampling sampling,
      const DlPaint* paint = nullptr,
      SrcRectConstraint constraint = SrcRectConstraint::kFast) override;
  virtual void DrawImageNine(const sk_sp<DlImage>& image,
                             const SkIRect& center,
                             const SkRect& dst,
                             DlFilterMode filter,
                             const DlPaint* paint = nullptr) override;
  virtual void DrawAtlas(const sk_sp<DlImage>& atlas,
                         const SkRSXform xform[],
                         const SkRect tex[],
                         const DlColor colors[],
                         int count,
                         DlBlendMode mode,
                         DlImageSampling sampling,
                         const SkRect* cullRect,
                         const DlPaint* paint = nullptr) override;
  virtual void DrawDisplayList(const sk_sp<DisplayList> display_list,
                               SkScalar opacity = SK_Scalar1) override;
  virtual void DrawImpellerPicture(
      const std::shared_ptr<const impeller::Picture>& picture,
      SkScalar opacity = SK_Scalar1) override;
  virtual void DrawTextBlob(const sk_sp<SkTextBlob>& blob,
                            SkScalar x,
                            SkScalar y,
                            const DlPaint& paint) override;
  virtual void DrawShadow(const SkPath& path,
                          const DlColor color,
                          const SkScalar elevation,
                          bool transparent_occluder,
                          SkScalar dpr) override;

 private:
  std::vector<LayerInfo> layer_stack_;
  LayerInfo* current_layer_;
  DisplayListMatrixClipTracker tracker_;
  std::unique_ptr<BoundsAccumulator> accumulator_;
  BoundsAccumulator* accumulator() { return accumulator_.get(); }

  // This flag indicates whether or not the current rendering attributes
  // are compatible with rendering ops applying an inherited opacity.
  bool current_opacity_compatibility_ = true;

  // Returns the compatibility of a given blend mode for applying an
  // inherited opacity value to modulate the visibility of the op.
  // For now we only accept SrcOver blend modes but this could be expanded
  // in the future to include other (rarely used) modes that also modulate
  // the opacity of a rendering operation at the cost of a switch statement
  // or lookup table.
  static bool IsOpacityCompatible(DlBlendMode mode) {
    return (mode == DlBlendMode::kSrcOver);
  }

  void UpdateCurrentOpacityCompatibility() {
    current_opacity_compatibility_ =             //
        current_.getColorFilter() == nullptr &&  //
        !current_.isInvertColors() &&            //
        IsOpacityCompatible(current_.getBlendMode());
  }

  // Update the opacity compatibility flags of the current layer for an op
  // that has determined its compatibility as indicated by |compatible|.
  void UpdateLayerOpacityCompatibility(bool compatible) {
    if (compatible) {
      current_layer_->add_compatible_op();
    } else {
      current_layer_->mark_incompatible();
    }
  }

  // Check for opacity compatibility for an op that may or may not use the
  // current rendering attributes as indicated by |uses_blend_attribute|.
  // If the flag is false then the rendering op will be able to substitute
  // a default Paint object with the opacity applied using the default SrcOver
  // blend mode which is always compatible with applying an inherited opacity.
  void CheckLayerOpacityCompatibility(bool uses_blend_attribute = true) {
    UpdateLayerOpacityCompatibility(!uses_blend_attribute ||
                                    current_opacity_compatibility_);
  }

  void CheckLayerOpacityHairlineCompatibility() {
    UpdateLayerOpacityCompatibility(
        current_opacity_compatibility_ &&
        (current_.getDrawStyle() == DlDrawStyle::kFill ||
         current_.getStrokeWidth() > 0));
  }

  // Check for opacity compatibility for an op that ignores the current
  // attributes and uses the indicated blend |mode| to render to the layer.
  // This is only used by |drawColor| currently.
  void CheckLayerOpacityCompatibility(DlBlendMode mode) {
    UpdateLayerOpacityCompatibility(IsOpacityCompatible(mode));
  }

  // The DisplayList had an unbounded call with no cull rect or clip
  // to contain it. Should only be called after the stream is fully
  // built.
  // Unbounded operations are calls like |drawColor| which are defined
  // to flood the entire surface, or calls that relied on a rendering
  // attribute which is unable to compute bounds (should be rare).
  // In those cases the bounds will represent only the accumulation
  // of the bounded calls and this flag will be set to indicate that
  // condition.
  bool is_unbounded() const {
    FML_DCHECK(layer_stack_.size() == 1);
    return layer_stack_.front().is_unbounded();
  }

  SkRect bounds() const {
    FML_DCHECK(layer_stack_.size() == 1);
    if (is_unbounded()) {
      FML_LOG(INFO) << "returning partial bounds for unbounded DisplayList";
    }

    return accumulator_->bounds();
  }

  sk_sp<DlRTree> rtree() {
    FML_DCHECK(layer_stack_.size() == 1);
    if (is_unbounded()) {
      FML_LOG(INFO) << "returning partial rtree for unbounded DisplayList";
    }

    return accumulator_->rtree();
  }

  enum class OpResult {
    kNoEffect,
    kPreservesTransparency,
    kAffectsAll,
  };

  bool paint_nops_on_transparency();
  OpResult PaintResult(const DlPaint& paint,
                       DisplayListAttributeFlags flags = kDrawPaintFlags);

  void UpdateLayerResult(OpResult result) {
    switch (result) {
      case OpResult::kNoEffect:
      case OpResult::kPreservesTransparency:
        break;
      case OpResult::kAffectsAll:
        current_layer_->add_visible_op();
        break;
    }
  }

  // Adjusts the indicated bounds for the given flags and returns true if
  // the calculation was possible, or false if it could not be estimated.
  bool AdjustBoundsForPaint(SkRect& bounds, DisplayListAttributeFlags flags);

  // Records the fact that we encountered an op that either could not
  // estimate its bounds or that fills all of the destination space.
  bool AccumulateUnbounded();

  // Records the bounds for an op after modifying them according to the
  // supplied attribute flags and transforming by the current matrix.
  bool AccumulateOpBounds(const SkRect& bounds,
                          DisplayListAttributeFlags flags) {
    SkRect safe_bounds = bounds;
    return AccumulateOpBounds(safe_bounds, flags);
  }

  // Records the bounds for an op after modifying them according to the
  // supplied attribute flags and transforming by the current matrix
  // and clipping against the current clip.
  bool AccumulateOpBounds(SkRect& bounds, DisplayListAttributeFlags flags);

  // Records the given bounds after transforming by the current matrix
  // and clipping against the current clip.
  bool AccumulateBounds(SkRect& bounds);
};

}  // namespace flutter
