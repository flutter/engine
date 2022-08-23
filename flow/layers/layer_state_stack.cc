// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/layer_state_stack.h"

namespace flutter {
  
LayerStateStack::LayerStateStack() {
  state_stack_.emplace_back(SkM44());
}

void LayerStateStack::setCanvasDelegate(SkCanvas* canvas) {
  if (canvas_) {
    canvas_->restoreToCount(canvas_restore_count_);
    canvas_ = nullptr;
  }
  if (canvas) {
    canvas_restore_count_ = canvas->getSaveCount();
    canvas_ = canvas;
    for (auto& state : state_stack_) {
      if (state.is_layer) {
        SkPaint paint;
        canvas->saveLayer(state.save_bounds(), state.save_skpaint(paint));
      } else {
        canvas->save();
      }
      canvas->setMatrix(state.matrix);
      for (auto& clip : state.clip_ops) {
        clip->apply(*canvas);
      }
    }
  }
}

void LayerStateStack::setBuilderDelegate(DisplayListBuilder* builder) {
  if (builder_) {
    builder_->restoreToCount(builder_restore_count_);
    builder_ = nullptr;
  }
  if (builder) {
    builder_restore_count_ = builder->getSaveCount();
    builder_ = builder;
    for (auto& state : state_stack_) {
      if (state.is_layer) {
        // We do not save the backdrop filter for later playback because
        // that filter should only be used to populate the temporary layer
        // of the first invocation of saveLayer. Since the filtered backdrop
        // appears behind any of the content of the saveLayer, subsequent
        // builder objects that are being populated from this state stack
        // should performa normal saveLayer with the properties that clip
        // or modulate that layers contents.
        builder->saveLayer(state.save_bounds(), state.save_dlpaint());
      } else {
        builder->save();
      }
      builder->transformReset();
      builder->transform(state.matrix);
      for (auto& clip : state.clip_ops) {
        clip->apply(*builder);
      }
    }
  }
}

void LayerStateStack::setMutatorDelegate(MutatorsStack* mutators) {
  // Does a MutatorsStack do restoreToCount?
  if (mutators_) {
    // builder_->restoreToCount(builder_restore_count_);
    mutators_ = nullptr;
  }
  if (mutators) {
    // builder_restore_count_ = mutators->getSaveCount();
    mutators_ = mutators;
  }
}

LayerStateStack::AutoRestore::AutoRestore(LayerStateStack* stack)
    : stack_(stack), stack_restore_count_(stack->getStackCount()) {}

LayerStateStack::AutoRestore::~AutoRestore() {
  stack_->restoreToCount(stack_restore_count_);
}

void LayerStateStack::save() {
  state_stack_.emplace_back(state_stack_.back().matrix);
  RenderState& state = state_stack_.back();
  state.is_layer = false;
  if (canvas_) {
    canvas_->save();
  }
  if (builder_) {
    builder_->save();
  }
}

LayerStateStack::AutoRestore LayerStateStack::autoSave() {
  save();
  return AutoRestore(this);
}

void LayerStateStack::saveLayer(const SkRect* bounds,
                                const DlPaint* paint,
                                const DlImageFilter* backdrop_filter) {
  state_stack_.emplace_back(state_stack_.back().matrix);
  RenderState& state = state_stack_.back();
  state.is_layer = true;
  if ((state.layer_has_bounds = (bounds != nullptr))) {
    state.layer_bounds = *bounds;
  }
  if ((state.layer_has_paint = (paint != nullptr))) {
    state.layer_paint = *paint;
  }
  if (canvas_) {
    SkPaint paint;
    if (backdrop_filter) {
      sk_sp<SkImageFilter> sk_filter = backdrop_filter->skia_object();
      canvas_->saveLayer(SkCanvas::SaveLayerRec(
          state.save_bounds(), state.save_skpaint(paint), sk_filter.get(), 0));
    } else {
      canvas_->saveLayer(state.save_bounds(), state.save_skpaint(paint));
    }
  }
  if (builder_) {
    builder_->saveLayer(bounds, paint, backdrop_filter);
  }
}

LayerStateStack::AutoRestore LayerStateStack::autoSaveLayer(
    const SkRect* bounds,
    const DlPaint* paint,
    const DlImageFilter* backdrop_filter) {
  saveLayer(bounds, paint, backdrop_filter);
  return AutoRestore(this);
}

void LayerStateStack::translate(SkScalar tx, SkScalar ty) {
  state_stack_.back().matrix.preTranslate(tx, ty);
  if (canvas_) {
    canvas_->translate(tx, ty);
  }
  if (builder_) {
    builder_->translate(tx, ty);
  }
}

void LayerStateStack::scale(SkScalar sx, SkScalar sy) {
  state_stack_.back().matrix.preScale(sx, sy);
  if (canvas_) {
    canvas_->scale(sx, sy);
  }
  if (builder_) {
    builder_->scale(sx, sy);
  }
}

void LayerStateStack::skew(SkScalar sx, SkScalar sy) {
  SkMatrix m;
  m.setSkew(sx, sy);
  state_stack_.back().matrix.preConcat(SkM44(m));
  if (canvas_) {
    canvas_->skew(sx, sy);
  }
  if (builder_) {
    builder_->skew(sx, sy);
  }
}

void LayerStateStack::rotate(SkScalar degrees) {
  SkMatrix m;
  m.setRotate(degrees);
  state_stack_.back().matrix.preConcat(SkM44(m));
  if (canvas_) {
    canvas_->rotate(degrees);
  }
  if (builder_) {
    builder_->rotate(degrees);
  }
}

void LayerStateStack::transform(SkMatrix& matrix) {
  state_stack_.back().matrix.preConcat(SkM44(matrix));
  if (canvas_) {
    canvas_->concat(matrix);
  }
  if (builder_) {
    builder_->transform(matrix);
  }
}

void LayerStateStack::transform(SkM44& matrix) {
  state_stack_.back().matrix.preConcat(matrix);
  if (canvas_) {
    canvas_->concat(matrix);
  }
  if (builder_) {
    builder_->transform(matrix);
  }
}

void LayerStateStack::clipRect(const SkRect& rect, SkClipOp op, bool is_aa) {
  state_stack_.back().clip_ops.push_back(
      std::make_unique<ClipRectEntry>(rect, op, is_aa));
  if (canvas_) {
    canvas_->clipRect(rect, op, is_aa);
  }
  if (builder_) {
    builder_->clipRect(rect, op, is_aa);
  }
}

void LayerStateStack::clipRRect(const SkRRect& rrect, SkClipOp op, bool is_aa) {
  state_stack_.back().clip_ops.push_back(
      std::make_unique<ClipRRectEntry>(rrect, op, is_aa));
  if (canvas_) {
    canvas_->clipRRect(rrect, op, is_aa);
  }
  if (builder_) {
    builder_->clipRRect(rrect, op, is_aa);
  }
}

void LayerStateStack::clipPath(const SkPath& path, SkClipOp op, bool is_aa) {
  state_stack_.back().clip_ops.push_back(
      std::make_unique<ClipPathEntry>(path, op, is_aa));
  if (canvas_) {
    canvas_->clipPath(path, op, is_aa);
  }
  if (builder_) {
    builder_->clipPath(path, op, is_aa);
  }
}

LayerStateStack::ClipEntry::ClipEntry(SkClipOp op, bool is_aa)
    : clip_op_(op), is_aa_(is_aa) {}

LayerStateStack::ClipRectEntry::ClipRectEntry(const SkRect& rect,
                                              SkClipOp op,
                                              bool is_aa)
    : ClipEntry(op, is_aa), rect_(rect) {}

void LayerStateStack::ClipRectEntry::apply(SkCanvas& canvas) const {
  canvas.clipRect(rect_, clip_op_, is_aa_);
}

void LayerStateStack::ClipRectEntry::apply(DisplayListBuilder& builder) const {
  builder.clipRect(rect_, clip_op_, is_aa_);
}

LayerStateStack::ClipRRectEntry::ClipRRectEntry(const SkRRect& rrect,
                                                SkClipOp op,
                                                bool is_aa)
    : ClipEntry(op, is_aa), rrect_(rrect) {}

void LayerStateStack::ClipRRectEntry::apply(SkCanvas& canvas) const {
  canvas.clipRRect(rrect_, clip_op_, is_aa_);
}

void LayerStateStack::ClipRRectEntry::apply(DisplayListBuilder& builder) const {
  builder.clipRRect(rrect_, clip_op_, is_aa_);
}

LayerStateStack::ClipPathEntry::ClipPathEntry(const SkPath& path,
                                              SkClipOp op,
                                              bool is_aa)
    : ClipEntry(op, is_aa), path_(path) {}

void LayerStateStack::ClipPathEntry::apply(SkCanvas& canvas) const {
  canvas.clipPath(path_, clip_op_, is_aa_);
}

void LayerStateStack::ClipPathEntry::apply(DisplayListBuilder& builder) const {
  builder.clipPath(path_, clip_op_, is_aa_);
}

LayerStateStack::RenderState::RenderState(SkM44& incoming_matrix)
    : matrix(incoming_matrix) {}

}  // namespace flutter
