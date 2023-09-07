// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/layer_state_stack.h"

#include "flutter/display_list/utils/dl_matrix_clip_tracker.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/paint_utils.h"
#include "flutter/flow/raster_cache_util.h"

namespace flutter {

// ==============================================================
// Delegate subclasses
// ==============================================================

// The DummyDelegate class implements most Delegate methods as NOP
// but throws errors if the caller starts executing query methods
// that require an active delegate to be tracking. It is specifically
// designed to be immutable, lightweight, and a singleton so that it
// can be substituted into the delegate slot in a LayerStateStack
// quickly and cheaply when no externally supplied delegates are present.
class DummyDelegate : public LayerStateStack::Delegate {
 public:
  static const std::shared_ptr<DummyDelegate> kInstance;

  void decommission() override {}

  DlFRect local_cull_rect() const override {
    error();
    return {};
  }
  DlFRect device_cull_rect() const override {
    error();
    return {};
  }
  DlTransform matrix() const override {
    error();
    return {};
  }
  bool content_culled(const DlFRect& content_bounds) const override {
    error();
    return true;
  }

  void save() override {}
  void saveLayer(const DlFRect& bounds,
                 LayerStateStack::RenderingAttributes& attributes,
                 DlBlendMode blend,
                 const DlImageFilter* backdrop) override {}
  void restore() override {}

  void translate(DlScalar tx, DlScalar ty) override {}
  void transform(const DlTransform& matrix) override {}
  void integralTransform() override {}

  void clipRect(const DlFRect& rect, ClipOp op, bool is_aa) override {}
  void clipRRect(const DlFRRect& rrect, ClipOp op, bool is_aa) override {}
  void clipPath(const DlPath& path, ClipOp op, bool is_aa) override {}

 private:
  static void error() {
    FML_DCHECK(false) << "LayerStateStack state queried without a delegate";
  }
};
const std::shared_ptr<DummyDelegate> DummyDelegate::kInstance =
    std::make_shared<DummyDelegate>();

class DlCanvasDelegate : public LayerStateStack::Delegate {
 public:
  explicit DlCanvasDelegate(DlCanvas* canvas)
      : canvas_(canvas), initial_save_level_(canvas->GetSaveCount()) {}

  void decommission() override { canvas_->RestoreToCount(initial_save_level_); }

  DlCanvas* canvas() const override { return canvas_; }

  DlFRect local_cull_rect() const override {
    return canvas_->GetLocalClipBounds();
  }
  DlFRect device_cull_rect() const override {
    return canvas_->GetDestinationClipBounds();
  }
  DlTransform matrix() const override { return canvas_->GetTransform(); }
  bool content_culled(const DlFRect& content_bounds) const override {
    return canvas_->QuickReject(content_bounds);
  }

  void save() override { canvas_->Save(); }
  void saveLayer(const DlFRect& bounds,
                 LayerStateStack::RenderingAttributes& attributes,
                 DlBlendMode blend_mode,
                 const DlImageFilter* backdrop) override {
    TRACE_EVENT0("flutter", "Canvas::saveLayer");
    DlPaint paint;
    canvas_->SaveLayer(&bounds, attributes.fill(paint, blend_mode), backdrop);
  }
  void restore() override { canvas_->Restore(); }

  void translate(DlScalar tx, DlScalar ty) override {
    canvas_->Translate(tx, ty);
  }
  void transform(const DlTransform& matrix) override {
    canvas_->Transform(matrix);
  }
  void integralTransform() override {
    DlTransform transform = matrix().WithIntegerTranslation();
    canvas_->SetTransform(transform);
  }

  void clipRect(const DlFRect& rect, ClipOp op, bool is_aa) override {
    canvas_->ClipRect(rect, op, is_aa);
  }
  void clipRRect(const DlFRRect& rrect, ClipOp op, bool is_aa) override {
    canvas_->ClipRRect(rrect, op, is_aa);
  }
  void clipPath(const DlPath& path, ClipOp op, bool is_aa) override {
    canvas_->ClipPath(path, op, is_aa);
  }

 private:
  DlCanvas* canvas_;
  const int initial_save_level_;
};

class PrerollDelegate : public LayerStateStack::Delegate {
 public:
  PrerollDelegate(const DlFRect& cull_rect, const DlTransform& matrix)
      : tracker_(cull_rect, matrix) {}

  void decommission() override {}

  DlTransform matrix() const override { return tracker_.matrix(); }
  DlFRect local_cull_rect() const override {
    return tracker_.local_cull_rect();
  }
  DlFRect device_cull_rect() const override {
    return tracker_.device_cull_rect();
  }
  bool content_culled(const DlFRect& content_bounds) const override {
    return tracker_.content_culled(content_bounds);
  }

  void save() override { tracker_.save(); }
  void saveLayer(const DlFRect& bounds,
                 LayerStateStack::RenderingAttributes& attributes,
                 DlBlendMode blend,
                 const DlImageFilter* backdrop) override {
    tracker_.save();
  }
  void restore() override { tracker_.restore(); }

  void translate(DlScalar tx, DlScalar ty) override {
    tracker_.translate(tx, ty);
  }
  void transform(const DlTransform& matrix) override {
    tracker_.transform(matrix);
  }
  void integralTransform() override {
    tracker_.setTransform(tracker_.matrix().WithIntegerTranslation());
  }

  void clipRect(const DlFRect& rect, ClipOp op, bool is_aa) override {
    tracker_.clipRect(rect, op, is_aa);
  }
  void clipRRect(const DlFRRect& rrect, ClipOp op, bool is_aa) override {
    tracker_.clipRRect(rrect, op, is_aa);
  }
  void clipPath(const DlPath& path, ClipOp op, bool is_aa) override {
    tracker_.clipPath(path, op, is_aa);
  }

 private:
  DisplayListMatrixClipTracker tracker_;
};

// ==============================================================
// StateEntry subclasses
// ==============================================================

class SaveEntry : public LayerStateStack::StateEntry {
 public:
  SaveEntry() = default;

  void apply(LayerStateStack* stack) const override {
    stack->delegate_->save();
  }
  void restore(LayerStateStack* stack) const override {
    stack->delegate_->restore();
  }

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(SaveEntry);
};

class SaveLayerEntry : public LayerStateStack::StateEntry {
 public:
  SaveLayerEntry(const DlFRect& bounds,
                 DlBlendMode blend_mode,
                 const LayerStateStack::RenderingAttributes& prev)
      : bounds_(bounds), blend_mode_(blend_mode), old_attributes_(prev) {}

  void apply(LayerStateStack* stack) const override {
    stack->delegate_->saveLayer(bounds_, stack->outstanding_, blend_mode_,
                                nullptr);
    stack->outstanding_ = {};
  }
  void restore(LayerStateStack* stack) const override {
    if (stack->checkerboard_func_) {
      DlCanvas* canvas = stack->canvas_delegate();
      if (canvas != nullptr) {
        (*stack->checkerboard_func_)(canvas, bounds_);
      }
    }
    stack->delegate_->restore();
    stack->outstanding_ = old_attributes_;
  }

 protected:
  const DlFRect bounds_;
  const DlBlendMode blend_mode_;
  const LayerStateStack::RenderingAttributes old_attributes_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(SaveLayerEntry);
};

class OpacityEntry : public LayerStateStack::StateEntry {
 public:
  OpacityEntry(const DlFRect& bounds,
               DlScalar opacity,
               const LayerStateStack::RenderingAttributes& prev)
      : bounds_(bounds),
        opacity_(opacity),
        old_opacity_(prev.opacity),
        old_bounds_(prev.save_layer_bounds) {}

  void apply(LayerStateStack* stack) const override {
    stack->outstanding_.save_layer_bounds = bounds_;
    stack->outstanding_.opacity *= opacity_;
  }
  void restore(LayerStateStack* stack) const override {
    stack->outstanding_.save_layer_bounds = old_bounds_;
    stack->outstanding_.opacity = old_opacity_;
  }
  void update_mutators(MutatorsStack* mutators_stack) const override {
    mutators_stack->PushOpacity(DlColor::toAlpha(opacity_));
  }

 private:
  const DlFRect bounds_;
  const DlScalar opacity_;
  const DlScalar old_opacity_;
  const DlFRect old_bounds_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(OpacityEntry);
};

class ImageFilterEntry : public LayerStateStack::StateEntry {
 public:
  ImageFilterEntry(const DlFRect& bounds,
                   const std::shared_ptr<const DlImageFilter>& filter,
                   const LayerStateStack::RenderingAttributes& prev)
      : bounds_(bounds),
        filter_(filter),
        old_filter_(prev.image_filter),
        old_bounds_(prev.save_layer_bounds) {}
  ~ImageFilterEntry() override = default;

  void apply(LayerStateStack* stack) const override {
    stack->outstanding_.save_layer_bounds = bounds_;
    stack->outstanding_.image_filter = filter_;
  }
  void restore(LayerStateStack* stack) const override {
    stack->outstanding_.save_layer_bounds = old_bounds_;
    stack->outstanding_.image_filter = old_filter_;
  }

  // There is no ImageFilter mutator currently
  // void update_mutators(MutatorsStack* mutators_stack) const override;

 private:
  const DlFRect bounds_;
  const std::shared_ptr<const DlImageFilter> filter_;
  const std::shared_ptr<const DlImageFilter> old_filter_;
  const DlFRect old_bounds_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(ImageFilterEntry);
};

class ColorFilterEntry : public LayerStateStack::StateEntry {
 public:
  ColorFilterEntry(const DlFRect& bounds,
                   const std::shared_ptr<const DlColorFilter>& filter,
                   const LayerStateStack::RenderingAttributes& prev)
      : bounds_(bounds),
        filter_(filter),
        old_filter_(prev.color_filter),
        old_bounds_(prev.save_layer_bounds) {}
  ~ColorFilterEntry() override = default;

  void apply(LayerStateStack* stack) const override {
    stack->outstanding_.save_layer_bounds = bounds_;
    stack->outstanding_.color_filter = filter_;
  }
  void restore(LayerStateStack* stack) const override {
    stack->outstanding_.save_layer_bounds = old_bounds_;
    stack->outstanding_.color_filter = old_filter_;
  }

  // There is no ColorFilter mutator currently
  // void update_mutators(MutatorsStack* mutators_stack) const override;

 private:
  const DlFRect bounds_;
  const std::shared_ptr<const DlColorFilter> filter_;
  const std::shared_ptr<const DlColorFilter> old_filter_;
  const DlFRect old_bounds_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(ColorFilterEntry);
};

class BackdropFilterEntry : public SaveLayerEntry {
 public:
  BackdropFilterEntry(const DlFRect& bounds,
                      const std::shared_ptr<const DlImageFilter>& filter,
                      DlBlendMode blend_mode,
                      const LayerStateStack::RenderingAttributes& prev)
      : SaveLayerEntry(bounds, blend_mode, prev), filter_(filter) {}
  ~BackdropFilterEntry() override = default;

  void apply(LayerStateStack* stack) const override {
    stack->delegate_->saveLayer(bounds_, stack->outstanding_, blend_mode_,
                                filter_.get());
    stack->outstanding_ = {};
  }

  void reapply(LayerStateStack* stack) const override {
    // On the reapply for subsequent overlay layers, we do not
    // want to reapply the backdrop filter, but we do need to
    // do a saveLayer to encapsulate the contents and match the
    // restore that will be forthcoming. Note that this is not
    // perfect if the BlendMode is not associative as we will be
    // compositing multiple parts of the content in batches.
    // Luckily the most common SrcOver is associative.
    SaveLayerEntry::apply(stack);
  }

 private:
  const std::shared_ptr<const DlImageFilter> filter_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(BackdropFilterEntry);
};

class TranslateEntry : public LayerStateStack::StateEntry {
 public:
  TranslateEntry(DlScalar tx, DlScalar ty) : tx_(tx), ty_(ty) {}

  void apply(LayerStateStack* stack) const override {
    stack->delegate_->translate(tx_, ty_);
  }
  void update_mutators(MutatorsStack* mutators_stack) const override {
    mutators_stack->PushTransform(DlTransform::MakeTranslate(tx_, ty_));
  }

 private:
  const DlScalar tx_;
  const DlScalar ty_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(TranslateEntry);
};

class TransformEntry : public LayerStateStack::StateEntry {
 public:
  explicit TransformEntry(const DlTransform& transform)
      : transform_(transform) {}

  void apply(LayerStateStack* stack) const override {
    stack->delegate_->transform(transform_);
  }
  void update_mutators(MutatorsStack* mutators_stack) const override {
    mutators_stack->PushTransform(transform_);
  }

 private:
  const DlTransform transform_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(TransformEntry);
};

class IntegralTransformEntry : public LayerStateStack::StateEntry {
 public:
  IntegralTransformEntry() = default;

  void apply(LayerStateStack* stack) const override {
    stack->delegate_->integralTransform();
  }

 private:
  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(IntegralTransformEntry);
};

class ClipRectEntry : public LayerStateStack::StateEntry {
 public:
  ClipRectEntry(const DlFRect& clip_rect, bool is_aa)
      : clip_rect_(clip_rect), is_aa_(is_aa) {}

  void apply(LayerStateStack* stack) const override {
    stack->delegate_->clipRect(clip_rect_, DlCanvas::ClipOp::kIntersect,
                               is_aa_);
  }
  void update_mutators(MutatorsStack* mutators_stack) const override {
    mutators_stack->PushClipRect(clip_rect_);
  }

 private:
  const DlFRect clip_rect_;
  const bool is_aa_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(ClipRectEntry);
};

class ClipRRectEntry : public LayerStateStack::StateEntry {
 public:
  ClipRRectEntry(const DlFRRect& clip_rrect, bool is_aa)
      : clip_rrect_(clip_rrect), is_aa_(is_aa) {}

  void apply(LayerStateStack* stack) const override {
    stack->delegate_->clipRRect(clip_rrect_, DlCanvas::ClipOp::kIntersect,
                                is_aa_);
  }
  void update_mutators(MutatorsStack* mutators_stack) const override {
    mutators_stack->PushClipRRect(clip_rrect_);
  }

 private:
  const DlFRRect clip_rrect_;
  const bool is_aa_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(ClipRRectEntry);
};

class ClipPathEntry : public LayerStateStack::StateEntry {
 public:
  ClipPathEntry(const DlPath& clip_path, bool is_aa)
      : clip_path_(clip_path), is_aa_(is_aa) {}
  ~ClipPathEntry() override = default;

  void apply(LayerStateStack* stack) const override {
    stack->delegate_->clipPath(clip_path_, DlCanvas::ClipOp::kIntersect,
                               is_aa_);
  }
  void update_mutators(MutatorsStack* mutators_stack) const override {
    mutators_stack->PushClipPath(clip_path_);
  }

 private:
  const DlPath clip_path_;
  const bool is_aa_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(ClipPathEntry);
};

// ==============================================================
// RenderingAttributes methods
// ==============================================================

DlPaint* LayerStateStack::RenderingAttributes::fill(DlPaint& paint,
                                                    DlBlendMode mode) const {
  DlPaint* ret = nullptr;
  if (opacity < SK_Scalar1) {
    paint.setOpacity(std::max(opacity, 0.0f));
    ret = &paint;
  } else {
    paint.setOpacity(SK_Scalar1);
  }
  paint.setColorFilter(color_filter);
  if (color_filter) {
    ret = &paint;
  }
  paint.setImageFilter(image_filter);
  if (image_filter) {
    ret = &paint;
  }
  paint.setBlendMode(mode);
  if (mode != DlBlendMode::kSrcOver) {
    ret = &paint;
  }
  return ret;
}

// ==============================================================
// MutatorContext methods
// ==============================================================

using MutatorContext = LayerStateStack::MutatorContext;

void MutatorContext::saveLayer(const DlFRect& bounds) {
  layer_state_stack_->save_layer(bounds);
}

void MutatorContext::applyOpacity(const DlFRect& bounds, DlScalar opacity) {
  if (opacity < SK_Scalar1) {
    layer_state_stack_->push_opacity(bounds, opacity);
  }
}

void MutatorContext::applyImageFilter(
    const DlFRect& bounds,
    const std::shared_ptr<const DlImageFilter>& filter) {
  if (filter) {
    layer_state_stack_->push_image_filter(bounds, filter);
  }
}

void MutatorContext::applyColorFilter(
    const DlFRect& bounds,
    const std::shared_ptr<const DlColorFilter>& filter) {
  if (filter) {
    layer_state_stack_->push_color_filter(bounds, filter);
  }
}

void MutatorContext::applyBackdropFilter(
    const DlFRect& bounds,
    const std::shared_ptr<const DlImageFilter>& filter,
    DlBlendMode blend_mode) {
  layer_state_stack_->push_backdrop(bounds, filter, blend_mode);
}

void MutatorContext::translate(DlScalar tx, DlScalar ty) {
  if (!(tx == 0 && ty == 0)) {
    layer_state_stack_->maybe_save_layer_for_transform(save_needed_);
    save_needed_ = false;
    layer_state_stack_->push_translate(tx, ty);
  }
}

void MutatorContext::transform(const DlTransform& matrix) {
  if (matrix.IsTranslate()) {
    translate(matrix.rc(0, 3), matrix.rc(1, 3));
  } else if (!matrix.IsIdentity()) {
    layer_state_stack_->maybe_save_layer_for_transform(save_needed_);
    save_needed_ = false;
    layer_state_stack_->push_transform(matrix);
  }
}

void MutatorContext::integralTransform() {
  layer_state_stack_->maybe_save_layer_for_transform(save_needed_);
  save_needed_ = false;
  layer_state_stack_->push_integral_transform();
}

void MutatorContext::clipRect(const DlFRect& rect, bool is_aa) {
  layer_state_stack_->maybe_save_layer_for_clip(save_needed_);
  save_needed_ = false;
  layer_state_stack_->push_clip_rect(rect, is_aa);
}

void MutatorContext::clipRRect(const DlFRRect& rrect, bool is_aa) {
  layer_state_stack_->maybe_save_layer_for_clip(save_needed_);
  save_needed_ = false;
  layer_state_stack_->push_clip_rrect(rrect, is_aa);
}

void MutatorContext::clipPath(const DlPath& path, bool is_aa) {
  layer_state_stack_->maybe_save_layer_for_clip(save_needed_);
  save_needed_ = false;
  layer_state_stack_->push_clip_path(path, is_aa);
}

// ==============================================================
// LayerStateStack methods
// ==============================================================

LayerStateStack::LayerStateStack() : delegate_(DummyDelegate::kInstance) {}

void LayerStateStack::clear_delegate() {
  delegate_->decommission();
  delegate_ = DummyDelegate::kInstance;
}

void LayerStateStack::set_delegate(DlCanvas* canvas) {
  if (delegate_) {
    if (canvas == delegate_->canvas()) {
      return;
    }
    clear_delegate();
  }
  if (canvas) {
    delegate_ = std::make_shared<DlCanvasDelegate>(canvas);
    reapply_all();
  }
}

void LayerStateStack::set_preroll_delegate(const DlFRect& cull_rect) {
  set_preroll_delegate(cull_rect, DlTransform());
}
void LayerStateStack::set_preroll_delegate(const DlTransform& matrix) {
  set_preroll_delegate(kMaxCullRect, matrix);
}
void LayerStateStack::set_preroll_delegate(const DlFRect& cull_rect,
                                           const DlTransform& matrix) {
  clear_delegate();
  delegate_ = std::make_shared<PrerollDelegate>(cull_rect, matrix);
  reapply_all();
}

void LayerStateStack::reapply_all() {
  // We use a local RenderingAttributes instance so that it can track the
  // necessary state changes independently as they occur in the stack.
  // Reusing |outstanding_| would wreak havoc on the current state of
  // the stack. When we are finished, though, the local attributes
  // contents should match the current outstanding_ values;
  RenderingAttributes attributes = outstanding_;
  outstanding_ = {};
  for (auto& state : state_stack_) {
    state->reapply(this);
  }
  FML_DCHECK(attributes == outstanding_);
}

void LayerStateStack::fill(MutatorsStack* mutators) {
  for (auto& state : state_stack_) {
    state->update_mutators(mutators);
  }
}

void LayerStateStack::restore_to_count(size_t restore_count) {
  while (state_stack_.size() > restore_count) {
    state_stack_.back()->restore(this);
    state_stack_.pop_back();
  }
}

void LayerStateStack::push_opacity(const DlFRect& bounds, DlScalar opacity) {
  maybe_save_layer(opacity);
  state_stack_.emplace_back(
      std::make_unique<OpacityEntry>(bounds, opacity, outstanding_));
  apply_last_entry();
}

void LayerStateStack::push_color_filter(
    const DlFRect& bounds,
    const std::shared_ptr<const DlColorFilter>& filter) {
  maybe_save_layer(filter);
  state_stack_.emplace_back(
      std::make_unique<ColorFilterEntry>(bounds, filter, outstanding_));
  apply_last_entry();
}

void LayerStateStack::push_image_filter(
    const DlFRect& bounds,
    const std::shared_ptr<const DlImageFilter>& filter) {
  maybe_save_layer(filter);
  state_stack_.emplace_back(
      std::make_unique<ImageFilterEntry>(bounds, filter, outstanding_));
  apply_last_entry();
}

void LayerStateStack::push_backdrop(
    const DlFRect& bounds,
    const std::shared_ptr<const DlImageFilter>& filter,
    DlBlendMode blend_mode) {
  state_stack_.emplace_back(std::make_unique<BackdropFilterEntry>(
      bounds, filter, blend_mode, outstanding_));
  apply_last_entry();
}

void LayerStateStack::push_translate(DlScalar tx, DlScalar ty) {
  state_stack_.emplace_back(std::make_unique<TranslateEntry>(tx, ty));
  apply_last_entry();
}

void LayerStateStack::push_transform(const DlTransform& transform) {
  state_stack_.emplace_back(std::make_unique<TransformEntry>(transform));
  apply_last_entry();
}

void LayerStateStack::push_integral_transform() {
  state_stack_.emplace_back(std::make_unique<IntegralTransformEntry>());
  apply_last_entry();
}

void LayerStateStack::push_clip_rect(const DlFRect& rect, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipRectEntry>(rect, is_aa));
  apply_last_entry();
}

void LayerStateStack::push_clip_rrect(const DlFRRect& rrect, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipRRectEntry>(rrect, is_aa));
  apply_last_entry();
}

void LayerStateStack::push_clip_path(const DlPath& path, bool is_aa) {
  state_stack_.emplace_back(std::make_unique<ClipPathEntry>(path, is_aa));
  apply_last_entry();
}

bool LayerStateStack::needs_save_layer(int flags) const {
  if (outstanding_.opacity < SK_Scalar1 &&
      (flags & LayerStateStack::kCallerCanApplyOpacity) == 0) {
    return true;
  }
  if (outstanding_.image_filter &&
      (flags & LayerStateStack::kCallerCanApplyImageFilter) == 0) {
    return true;
  }
  if (outstanding_.color_filter &&
      (flags & LayerStateStack::kCallerCanApplyColorFilter) == 0) {
    return true;
  }
  return false;
}

void LayerStateStack::do_save() {
  state_stack_.emplace_back(std::make_unique<SaveEntry>());
  apply_last_entry();
}

void LayerStateStack::save_layer(const DlFRect& bounds) {
  state_stack_.emplace_back(std::make_unique<SaveLayerEntry>(
      bounds, DlBlendMode::kSrcOver, outstanding_));
  apply_last_entry();
}

void LayerStateStack::maybe_save_layer_for_transform(bool save_needed) {
  // Alpha and ColorFilter don't care about transform
  if (outstanding_.image_filter) {
    save_layer(outstanding_.save_layer_bounds);
  } else if (save_needed) {
    do_save();
  }
}

void LayerStateStack::maybe_save_layer_for_clip(bool save_needed) {
  // Alpha and ColorFilter don't care about clipping
  // - Alpha of clipped content == clip of alpha content
  // - Color-filtering of clipped content == clip of color-filtered content
  if (outstanding_.image_filter) {
    save_layer(outstanding_.save_layer_bounds);
  } else if (save_needed) {
    do_save();
  }
}

void LayerStateStack::maybe_save_layer(int apply_flags) {
  if (needs_save_layer(apply_flags)) {
    save_layer(outstanding_.save_layer_bounds);
  }
}

void LayerStateStack::maybe_save_layer(DlScalar opacity) {
  if (outstanding_.image_filter) {
    save_layer(outstanding_.save_layer_bounds);
  }
}

void LayerStateStack::maybe_save_layer(
    const std::shared_ptr<const DlColorFilter>& filter) {
  if (outstanding_.color_filter || outstanding_.image_filter ||
      (outstanding_.opacity < SK_Scalar1 &&
       !filter->can_commute_with_opacity())) {
    // TBD: compose the 2 color filters together.
    save_layer(outstanding_.save_layer_bounds);
  }
}

void LayerStateStack::maybe_save_layer(
    const std::shared_ptr<const DlImageFilter>& filter) {
  if (outstanding_.image_filter) {
    // TBD: compose the 2 image filters together.
    save_layer(outstanding_.save_layer_bounds);
  }
}

}  // namespace flutter
