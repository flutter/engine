// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_LAYER_STATE_STACK_H_
#define FLUTTER_FLOW_LAYERS_LAYER_STATE_STACK_H_

#include "flutter/display_list/display_list_builder.h"
#include "flutter/display_list/display_list_canvas_recorder.h"

namespace flutter {

class LayerStateStack {
 public:
  LayerStateStack() = default;

  void setCanvasDelegate(SkCanvas* canvas);
  void setBuilderDelegate(DisplayListBuilder* builder);
  void setBuilderDelegate(sk_sp<DisplayListBuilder> builder) {
    setBuilderDelegate(builder.get());
  }
  void setBuilderDelegate(DisplayListCanvasRecorder& recorder) {
    setBuilderDelegate(recorder.builder().get());
  }

  class AutoRestore {
   public:
    ~AutoRestore();

   private:
    AutoRestore(LayerStateStack* stack);
    friend class LayerStateStack;

    LayerStateStack* stack_;
    size_t stack_restore_count_;
  };

  static constexpr int CALLER_CAN_APPLY_OPACITY = 0x1;
  static constexpr int CALLER_CAN_APPLY_ANYTHING = 0x1;

  struct RenderingAttributes {
    SkRect content_bounds;
    SkScalar opacity = SK_Scalar1;
  };

  // Apply the outstanding state via saveLayer if necessary,
  // respecting the flags representing which potentially
  // outstanding attributes the calling layer can apply
  // themselves.
  //
  // A saveLayer may or may not be sent to the delegates depending
  // on the outstanding state and the flags supplied by the caller.
  //
  // An AutoRestore instance will always be returned even if there
  // was no saveLayer applied.
  [[nodiscard]] AutoRestore applyState(const SkRect& bounds,
                                       int can_apply_flags = 0);

  SkScalar outstanding_opacity() { return outstanding_.opacity; }

  // Return a pointer to an SkPaint instance representing the
  // currently outstanding rendering attributes, or a nullptr
  // if there are no outstanding attributes for the caller to
  // apply during rendering operations.
  const SkPaint* sk_paint();

  // Return a pointer to an DlPaint instance representing the
  // currently outstanding rendering attributes, or a nullptr
  // if there are no outstanding attributes for the caller to
  // apply during rendering operations.
  const DlPaint* dl_paint();

  bool needs_painting() { return outstanding_.opacity > 0; }

  // Saves the current state of the state stack until the next
  // matching restore call.
  [[nodiscard]] AutoRestore save();

  // Saves the state stack and immediately executes a saveLayer
  // with all accumulated state onto the canvas or builder to
  // be applied at the next matching restore. A saveLayer is
  // always executed by this method even if there are no
  // outstanding attributes.
  [[nodiscard]] AutoRestore saveLayer(const SkRect* bounds,
                                      const SkRect* checker_bounds = nullptr);

  // Records the opacity for application at the next call to
  // saveLayer or applyState. A saveLayer may be executed at
  // this time if the opacity cannot be batched with other
  // outstanding attributes.
  [[nodiscard]] AutoRestore saveWithOpacity(
      const SkRect* bounds,
      SkScalar opacity,
      const SkRect* checker_bounds = nullptr);

  // Records the image filter for application at the next call to
  // saveLayer or applyState. A saveLayer may be executed at
  // this time if the image filter cannot be batched with other
  // outstanding attributes.
  // (Currently only opacity is recorded for batching)
  [[nodiscard]] AutoRestore saveWithImageFilter(
      const SkRect* bounds,
      const std::shared_ptr<const DlImageFilter> filter,
      const SkRect* checker_bounds = nullptr);

  // Records the color filter for application at the next call to
  // saveLayer or applyState. A saveLayer may be executed at
  // this time if the color filter cannot be batched with other
  // outstanding attributes.
  // (Currently only opacity is recorded for batching)
  [[nodiscard]] AutoRestore saveWithColorFilter(
      const SkRect* bounds,
      const std::shared_ptr<const DlColorFilter> filter,
      const SkRect* checker_bounds = nullptr);

  // Saves the state stack and immediately executes a saveLayer
  // with the indicated backdrop filter and any outstanding
  // state attributes. Since the backdrop filter only applies
  // to the pixels alrady on the screen when this call is made,
  // the backdrop filter will only be applied to the canvas or
  // builder installed at the time that this call is made, and
  // subsequent canvas or builder objects that are made delegates
  // will only see a saveLayer with the indicated blend_mode.
  [[nodiscard]] AutoRestore saveWithBackdropFilter(
      const SkRect* bounds,
      const std::shared_ptr<const DlImageFilter> filter,
      DlBlendMode blend_mode,
      const SkRect* checker_bounds = nullptr);

  void translate(SkScalar tx, SkScalar ty);
  void transform(const SkM44& matrix);
  void transform(const SkMatrix& matrix);

  void clipRect(const SkRect& rect, bool is_aa);
  void clipRRect(const SkRRect& rect, bool is_aa);
  void clipPath(const SkPath& rect, bool is_aa);

 private:
  size_t getStackCount() { return state_stack_.size(); }
  void restoreToCount(size_t restore_count);

  void pushAttributes();

  void resolve(const SkRect& bounds);

  static std::optional<SkRect> OptionalBounds(const SkRect* bounds) {
    return bounds ? std::make_optional<SkRect>(*bounds) : std::nullopt;
  }
  static const SkRect* BoundsPtr(const std::optional<SkRect>& bounds) {
    return bounds.has_value() ? &bounds.value() : nullptr;
  }

  class StateEntry {
   public:
    virtual ~StateEntry() = default;

    virtual void apply(RenderingAttributes* attributes,
                       SkCanvas* canvas,
                       DisplayListBuilder* builder) const = 0;

    virtual void reapply(RenderingAttributes* attributes,
                         SkCanvas* canvas,
                         DisplayListBuilder* builder) const {
      apply(attributes, canvas, builder);
    }

    virtual void restore(RenderingAttributes* attributes,
                         SkCanvas* canvas,
                         DisplayListBuilder* builder) const {}
  };

  class AttributesEntry : public StateEntry {
   public:
    AttributesEntry(RenderingAttributes attributes) : attributes_(attributes) {}

    virtual void apply(RenderingAttributes* attributes,
                       SkCanvas* canvas,
                       DisplayListBuilder* builder) const override {}

    void restore(RenderingAttributes* attributes,
                 SkCanvas* canvas,
                 DisplayListBuilder* builder) const override;

   private:
    const RenderingAttributes attributes_;
  };

  class SaveEntry : public StateEntry {
   public:
    SaveEntry() = default;

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;
    void restore(RenderingAttributes* attributes,
                 SkCanvas* canvas,
                 DisplayListBuilder* builder) const override;

   protected:
    virtual void do_checkerboard(SkCanvas* canvas,
                                 DisplayListBuilder* builder) const {}
  };

  class SaveLayerEntry : public SaveEntry {
   public:
    SaveLayerEntry(const SkRect* bounds, const SkRect* checker_bounds)
        : bounds_(OptionalBounds(bounds)),
          checkerboard_(OptionalBounds(checker_bounds)) {}

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   protected:
    const std::optional<SkRect> bounds_;
    const std::optional<SkRect> checkerboard_;

    void do_checkerboard(SkCanvas* canvas,
                         DisplayListBuilder* builder) const override;

    const SkRect* save_bounds() const { return BoundsPtr(bounds_); }

    const SkRect* checkerboard_bounds() const {
      return BoundsPtr(checkerboard_);
    }
  };

  class OpacityEntry : public SaveLayerEntry {
   public:
    OpacityEntry(const SkRect* bounds,
                 SkScalar opacity,
                 const SkRect* checker_bounds)
        : SaveLayerEntry(bounds, checker_bounds),
          opacity_(opacity) {}

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   private:
    const SkScalar opacity_;
  };

  class ImageFilterEntry : public SaveLayerEntry {
   public:
    ImageFilterEntry(const SkRect* bounds,
                     const std::shared_ptr<const DlImageFilter> filter,
                     const SkRect* checker_bounds)
        : SaveLayerEntry(bounds, checker_bounds), filter_(filter) {}
    ~ImageFilterEntry() override = default;

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   private:
    const std::shared_ptr<const DlImageFilter> filter_;
  };

  class ColorFilterEntry : public SaveLayerEntry {
   public:
    ColorFilterEntry(const SkRect* bounds,
                     const std::shared_ptr<const DlColorFilter> filter,
                     const SkRect* checker_bounds)
        : SaveLayerEntry(bounds, checker_bounds), filter_(filter) {}
    ~ColorFilterEntry() override = default;

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   private:
    const std::shared_ptr<const DlColorFilter> filter_;
  };

  class BackdropFilterEntry : public SaveLayerEntry {
   public:
    BackdropFilterEntry(const SkRect* bounds,
                        const std::shared_ptr<const DlImageFilter> filter,
                        DlBlendMode blend_mode,
                        const SkRect* checker_bounds)
        : SaveLayerEntry(bounds, checker_bounds),
          filter_(filter),
          blend_mode_(blend_mode) {}
    ~BackdropFilterEntry() override = default;

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

    void reapply(RenderingAttributes* attributes,
                 SkCanvas* canvas,
                 DisplayListBuilder* builder) const override;

   private:
    const std::shared_ptr<const DlImageFilter> filter_;
    const DlBlendMode blend_mode_;
  };

  class TransformEntry : public StateEntry {};

  class TranslateEntry : public TransformEntry {
   public:
    TranslateEntry(SkScalar tx, SkScalar ty) : tx_(tx), ty_(ty) {}

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   private:
    const SkScalar tx_;
    const SkScalar ty_;
  };

  class TransformMatrixEntry : public TransformEntry {
   public:
    TransformMatrixEntry(const SkMatrix& matrix) : matrix_(matrix) {}

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   private:
    const SkMatrix matrix_;
  };

  class TransformM44Entry : public TransformEntry {
   public:
    TransformM44Entry(const SkM44& m44) : m44_(m44) {}

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

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

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   private:
    const SkRect rect_;
  };

  class ClipRRectEntry : public ClipEntry {
   public:
    ClipRRectEntry(const SkRRect& rrect, bool is_aa)
        : ClipEntry(is_aa), rrect_(rrect) {}

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   private:
    const SkRRect rrect_;
  };

  class ClipPathEntry : public ClipEntry {
   public:
    ClipPathEntry(const SkPath& path, bool is_aa)
        : ClipEntry(is_aa), path_(path) {}
    ~ClipPathEntry() override = default;

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   private:
    const SkPath path_;
  };

  std::vector<std::unique_ptr<StateEntry>> state_stack_;

  SkCanvas* canvas_ = nullptr;
  int canvas_restore_count_ = 0.0;
  DisplayListBuilder* builder_ = nullptr;
  int builder_restore_count_ = 0.0;
  RenderingAttributes outstanding_;

  SkPaint temp_sk_paint_;
  DlPaint temp_dl_paint_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_LAYER_STATE_STACK_H_
