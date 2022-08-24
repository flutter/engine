// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_LAYER_STATE_STACK_H_
#define FLUTTER_FLOW_LAYERS_LAYER_STATE_STACK_H_

#include "flutter/display_list/display_list_builder.h"

namespace flutter {

class LayerStateStack {
 public:
  LayerStateStack() = default;

  void setCanvasDelegate(SkCanvas* canvas);
  void setBuilderDelegate(DisplayListBuilder* canvas);

  class AutoRestore {
   public:
    ~AutoRestore();

   private:
    AutoRestore(LayerStateStack* stack);
    friend class LayerStateStack;

    LayerStateStack* stack_;
    const size_t stack_restore_count_;
  };

  [[nodiscard]] AutoRestore save();
  [[nodiscard]] AutoRestore saveLayer(const SkRect* bounds);
  [[nodiscard]] AutoRestore saveWithOpacity(const SkRect* bounds,
                                            SkScalar opacity);
  [[nodiscard]] AutoRestore saveWithImageFilter(
      const SkRect* bounds,
      const std::shared_ptr<const DlImageFilter> filter);
  [[nodiscard]] AutoRestore saveWithColorFilter(
      const SkRect* bounds,
      const std::shared_ptr<const DlColorFilter> filter);
  [[nodiscard]] AutoRestore saveWithBackdropFilter(
      const SkRect* bounds,
      const std::shared_ptr<const DlImageFilter> filter,
      DlBlendMode blend_mode);

  void translate(SkScalar tx, SkScalar ty);
  void transform(const SkM44& matrix);
  void transform(const SkMatrix& matrix);

  void clipRect(const SkRect& rect, bool is_aa);
  void clipRRect(const SkRRect& rect, bool is_aa);
  void clipPath(const SkPath& rect, bool is_aa);

 private:
  size_t getStackCount() { return state_stack_.size(); }
  void restoreToCount(size_t restore_count);

  class StateEntry {
   public:
    virtual ~StateEntry() = default;

    virtual void apply(SkCanvas* canvas, DisplayListBuilder* builder) const = 0;
    virtual void restore(SkCanvas* canvas, DisplayListBuilder* builder) const {}
    virtual bool is_backdrop_filter() const { return false; }
  };

  class SaveEntry : public StateEntry {
   public:
    SaveEntry() = default;

    void apply(SkCanvas* canvas, DisplayListBuilder* builder) const override;
    virtual void restore(SkCanvas* canvas,
                         DisplayListBuilder* builder) const override;
  };

  class SaveLayerEntry : public SaveEntry {
   public:
    SaveLayerEntry(const SkRect* bounds)
        : bounds_(bounds ? *bounds : SkRect::MakeEmpty()),
          has_bounds_(bounds != nullptr) {}

    void apply(SkCanvas* canvas, DisplayListBuilder* builder) const override;

   protected:
    const SkRect bounds_;
    const bool has_bounds_;

    const SkRect* save_bounds() const {
      return has_bounds_ ? &bounds_ : nullptr;
    }
  };

  class OpacityEntry : public SaveLayerEntry {
   public:
    OpacityEntry(const SkRect* bounds, SkScalar opacity)
        : SaveLayerEntry(bounds), opacity_(opacity) {}

    void apply(SkCanvas* canvas, DisplayListBuilder* builder) const override;

   private:
    const SkScalar opacity_;
  };

  class ImageFilterEntry : public SaveLayerEntry {
   public:
    ImageFilterEntry(const SkRect* bounds,
                     const std::shared_ptr<const DlImageFilter> filter)
        : SaveLayerEntry(bounds), filter_(filter) {}
    ~ImageFilterEntry() override = default;

    void apply(SkCanvas* canvas, DisplayListBuilder* builder) const override;

   private:
    const std::shared_ptr<const DlImageFilter> filter_;
  };

  class ColorFilterEntry : public SaveLayerEntry {
   public:
    ColorFilterEntry(const SkRect* bounds,
                     const std::shared_ptr<const DlColorFilter> filter)
        : SaveLayerEntry(bounds), filter_(filter) {}
    ~ColorFilterEntry() override = default;

    void apply(SkCanvas* canvas, DisplayListBuilder* builder) const override;

   private:
    const std::shared_ptr<const DlColorFilter> filter_;
  };

  class BackdropFilterEntry : public SaveLayerEntry {
   public:
    BackdropFilterEntry(const SkRect* bounds,
                        const std::shared_ptr<const DlImageFilter> filter,
                        DlBlendMode blend_mode)
        : SaveLayerEntry(bounds), filter_(filter), blend_mode_(blend_mode) {}
    ~BackdropFilterEntry() override = default;

    void apply(SkCanvas* canvas, DisplayListBuilder* builder) const override;

    bool is_backdrop_filter() const override { return false; }

   private:
    const std::shared_ptr<const DlImageFilter> filter_;
    const DlBlendMode blend_mode_;
  };

  class TransformEntry : public StateEntry {};

  class TranslateEntry : public TransformEntry {
   public:
    TranslateEntry(SkScalar tx, SkScalar ty) : tx_(tx), ty_(ty) {}

    void apply(SkCanvas* canvas, DisplayListBuilder* builder) const override;

   private:
    const SkScalar tx_;
    const SkScalar ty_;
  };

  class TransformMatrixEntry : public TransformEntry {
   public:
    TransformMatrixEntry(const SkMatrix& matrix) : matrix_(matrix) {}

    void apply(SkCanvas* canvas, DisplayListBuilder* builder) const override;

   private:
    const SkMatrix matrix_;
  };

  class TransformM44Entry : public TransformEntry {
   public:
    TransformM44Entry(const SkM44& m44) : m44_(m44) {}

    void apply(SkCanvas* canvas, DisplayListBuilder* builder) const override;

   private:
    const SkM44 m44_;
  };

  class ClipEntry : public StateEntry {
   protected:
    ClipEntry(bool is_aa) : is_aa_(is_aa) {}

    const bool is_aa_;
  };

  class ClipRectEntry : public ClipEntry {
   public:
    ClipRectEntry(const SkRect& rect, bool is_aa)
        : ClipEntry(is_aa), rect_(rect) {}

    void apply(SkCanvas* canvas, DisplayListBuilder* builder) const override;

   private:
    const SkRect rect_;
  };

  class ClipRRectEntry : public ClipEntry {
   public:
    ClipRRectEntry(const SkRRect& rrect, bool is_aa)
        : ClipEntry(is_aa), rrect_(rrect) {}

    void apply(SkCanvas* canvas, DisplayListBuilder* builder) const override;

   private:
    const SkRRect rrect_;
  };

  class ClipPathEntry : public ClipEntry {
   public:
    ClipPathEntry(const SkPath& path, bool is_aa)
        : ClipEntry(is_aa), path_(path) {}
    ~ClipPathEntry() override = default;

    void apply(SkCanvas* canvas, DisplayListBuilder* builder) const override;

   private:
    const SkPath path_;
  };

  std::vector<std::unique_ptr<StateEntry>> state_stack_;

  SkCanvas* canvas_ = nullptr;
  int canvas_restore_count_;
  DisplayListBuilder* builder_ = nullptr;
  int builder_restore_count_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_LAYER_STATE_STACK_H_
