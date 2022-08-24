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

  [[nodiscard]] AutoRestore save();
  [[nodiscard]] AutoRestore saveWithImageFilter(
      const SkRect* bounds,
      const std::shared_ptr<const DlImageFilter> filter);
  [[nodiscard]] AutoRestore saveWithColorFilter(
      const SkRect* bounds,
      const std::shared_ptr<const DlColorFilter> filter);
  [[nodiscard]] AutoRestore saveWithBackdropFilter(
      const SkRect* bounds,
      const std::shared_ptr<const DlImageFilter> filter);

  void translate(SkScalar tx, SkScalar ty);
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
    virtual void apply(SkCanvas* canvas, DisplayListBuilder* builder) const = 0;
    virtual bool is_backdrop_filter() const { return false; }
  };

  class SaveEntry : public StateEntry {
   public:
    SaveEntry() = default;
  };

  class SaveLayerEntry : public StateEntry {
   public:
    SaveLayerEntry(const SkRect* bounds)
        : bounds_(bounds ? *bounds : SkRect::MakeEmpty()),
          has_bounds_(bounds != nullptr) {}

    void apply(SkCanvas* canvas,
               DisplayListBuilder* builder) const override = 0;

   protected:
    const SkRect bounds_;
    const bool has_bounds_;

    const SkRect* save_bounds() const {
      return has_bounds_ ? &bounds_ : nullptr;
    }
  };

  class ImageFilterEntry : public SaveLayerEntry {
   public:
    ImageFilterEntry(const SkRect* bounds,
                     const std::shared_ptr<const DlImageFilter> filter)
        : SaveLayerEntry(bounds), filter_(filter) {}

    void apply(SkCanvas* canvas,
               DisplayListBuilder* builder) const override = 0;

   private:
    const std::shared_ptr<const DlImageFilter> filter_;
  };

  class ColorFilterEntry : public SaveLayerEntry {
   public:
    ColorFilterEntry(const SkRect* bounds,
                     const std::shared_ptr<const DlColorFilter> filter)
        : SaveLayerEntry(bounds), filter_(filter) {}

    void apply(SkCanvas* canvas,
               DisplayListBuilder* builder) const override = 0;

   private:
    const std::shared_ptr<const DlColorFilter> filter_;
  };

  class BackdropFilterEntry : public SaveLayerEntry {
   public:
    BackdropFilterEntry(const SkRect* bounds,
                        const std::shared_ptr<const DlImageFilter> filter)
        : SaveLayerEntry(bounds), filter_(filter) {}

    void apply(SkCanvas* canvas,
               DisplayListBuilder* builder) const override = 0;

    bool is_backdrop_filter() const override { return false; }

   private:
    const std::shared_ptr<const DlImageFilter> filter_;
  };

  class TransformEntry : public StateEntry {};

  class TranslateEntry : public TransformEntry {
   public:
    TranslateEntry(SkScalar tx, SkScalar ty) : tx_(tx), ty_(ty) {}

    void apply(SkCanvas* canvas,
               DisplayListBuilder* builder) const override = 0;

   private:
    const SkScalar tx_;
    const SkScalar ty_;
  };

  class TransformMatrixEntry : public TransformEntry {
   public:
    TransformMatrixEntry(SkMatrix& matrix) : matrix_(matrix) {}

    void apply(SkCanvas* canvas,
               DisplayListBuilder* builder) const override = 0;

   private:
    const SkMatrix matrix_;
  };

  class TransformM44Entry : public TransformEntry {
   public:
    TransformM44Entry(SkM44 m44) : m44_(m44) {}

    void apply(SkCanvas* canvas,
               DisplayListBuilder* builder) const override = 0;

   private:
    const SkM44 m44_;
  };

  class ClipEntry : public StateEntry {
   protected:
    ClipEntry(SkClipOp op, bool is_aa) : clip_op_(op), is_aa_(is_aa) {}

    const SkClipOp clip_op_;
    const bool is_aa_;
  };

  class ClipRectEntry : public ClipEntry {
    ClipRectEntry(const SkRect& rect, SkClipOp op, bool is_aa)
        : ClipEntry(op, is_aa), rect_(rect) {}

    void apply(SkCanvas* canvas,
               DisplayListBuilder* builder) const override = 0;

   private:
    const SkRect rect_;
  };

  class ClipRRectEntry : public ClipEntry {
    ClipRRectEntry(const SkRRect& rrect, SkClipOp op, bool is_aa)
        : ClipEntry(op, is_aa), rrect_(rrect) {}

    void apply(SkCanvas* canvas,
               DisplayListBuilder* builder) const override = 0;

   private:
    const SkRRect rrect_;
  };

  class ClipPathEntry : public ClipEntry {
    ClipPathEntry(const SkPath& path, SkClipOp op, bool is_aa)
        : ClipEntry(op, is_aa), path_(path) {}

    void apply(SkCanvas* canvas,
               DisplayListBuilder* builder) const override = 0;

   private:
    const SkPath path_;
  };

  std::vector<std::unique_ptr<StateEntry>> state_stack_;

  SkCanvas* canvas_ = nullptr;
  int canvas_restore_count_;
  DisplayListBuilder* builder_ = nullptr;
  int builder_restore_count_;
  MutatorsStack* mutators_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_LAYER_STATE_STACK_H_
