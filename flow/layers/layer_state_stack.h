// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_LAYER_STATE_STACK_H_
#define FLUTTER_FLOW_LAYERS_LAYER_STATE_STACK_H_

#include "flutter/display_list/display_list_builder.h"
#include "layer.h"

namespace flutter {

class LayerStateStack {
 public:
  LayerStateStack();

  void setCanvasDelegate(SkCanvas* canvas);
  void setBuilderDelegate(DisplayListBuilder* canvas);
  void setMutatorDelegate(MutatorsStack* mutators);

  class AutoRestore {
   public:
    ~AutoRestore();

   private:
    AutoRestore(LayerStateStack* stack);
    friend class LayerStateStack;

    LayerStateStack* stack_;
    const int stack_restore_count_;
  };

  [[nodiscard]] AutoRestore autoSave();
  [[nodiscard]] AutoRestore pushImageFilter(const SkRect* bounds,
                                            const DlImageFilter* filter);
  [[nodiscard]] AutoRestore pushColorFilter(const SkRect* bounds,
                                            const DlColorFilter* filter);
  [[nodiscard]] AutoRestore pushBackdropFilter(const SkRect* bounds,
                                               const DlImageFilter* backdrop);
  void translate(SkScalar tx, SkScalar ty);
  void scale(SkScalar sx, SkScalar sy);
  void skew(SkScalar sx, SkScalar sy);
  void rotate(SkScalar degrees);
  void transform(SkM44& matrix);
  void transform(SkMatrix& matrix);

  void clipRect(const SkRect& rect, SkClipOp op, bool is_aa);
  void clipRRect(const SkRRect& rect, SkClipOp op, bool is_aa);
  void clipPath(const SkPath& rect, SkClipOp op, bool is_aa);

 private:
  int getStackCount();
  void restoreToCount(int restore_count);

  class StateEntry {
   public:
    virtual void apply(SkCanvas& canvas) const = 0;
    virtual void apply(DisplayListBuilder& canvas) const = 0;
    virtual void apply(MutatorsStack& mutators) const = 0;
  };

  class TransformEntry : public StateEntry {
    
  };

  class ClipEntry : public StateEntry {
   protected:
    ClipEntry(SkClipOp op, bool is_aa);

    const SkClipOp clip_op_;
    const bool is_aa_;
  };

  class ClipRectEntry : public ClipEntry {
    ClipRectEntry(const SkRect& rect, SkClipOp op, bool is_aa);

    void apply(SkCanvas& canvas) const override;
    void apply(DisplayListBuilder& canvas) const override;

   private:
    const SkRect rect_;
  };

  class ClipRRectEntry : public ClipEntry {
    ClipRRectEntry(const SkRRect& rrect, SkClipOp op, bool is_aa);

    void apply(SkCanvas& canvas) const override;
    void apply(DisplayListBuilder& canvas) const override;

   private:
    const SkRRect rrect_;
  };

  class ClipPathEntry : public ClipEntry {
    ClipPathEntry(const SkPath& path, SkClipOp op, bool is_aa);

    void apply(SkCanvas& canvas) const override;
    void apply(DisplayListBuilder& canvas) const override;

   private:
    const SkPath path_;
  };

  struct RenderState {
    RenderState(SkM44& incoming_matrix);

    SkRect* save_bounds() { return layer_has_bounds ? &layer_bounds : nullptr; }
    SkPaint* save_skpaint(SkPaint& paint) {
      if (!layer_has_paint) {
        return nullptr;
      }
      layer_paint.toSkPaint(paint);
      return &paint;
    }
    DlPaint* save_dlpaint() { return layer_has_paint ? &layer_paint : nullptr; }

    bool is_layer;

    bool layer_has_bounds;
    SkRect layer_bounds;

    bool layer_has_paint;
    DlPaint layer_paint;

    SkM44 matrix;
    std::vector<std::unique_ptr<ClipEntry>> clip_ops;
  };

  std::vector<RenderState> state_stack_;

  SkCanvas* canvas_ = nullptr;
  int canvas_restore_count_;
  DisplayListBuilder* builder_ = nullptr;
  int builder_restore_count_;
  MutatorsStack* mutators_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_LAYER_STATE_STACK_H_
