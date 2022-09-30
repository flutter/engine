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

  bool checkerboard_save_layers() { return do_checkerboard_; }
  void set_checkerboard_save_layers(bool checkerboard) {
    do_checkerboard_ = checkerboard;
  }

  // Clears out any old delegate to make room for a new one.
  void clear_delegate();

  // Return the SkCanvas delegate if the state stack has such a delegate.
  // The state stack will only have either an SkCanvas or a Builder
  // delegate at any given time.
  // See also |builder_delegate|.
  SkCanvas* canvas_delegate() { return canvas_; }

  // Return the DisplayListBuilder delegate if the state stack has such a
  // delegate.
  // The state stack will only have either an SkCanvas or a Builder
  // delegate at any given time.
  // See also |builder_delegate|.
  DisplayListBuilder* builder_delegate() { return builder_; }

  // Clears the old delegate and sets the canvas delegate to the indicated
  // canvas (if not nullptr). This ensures that only one delegate - either
  // a canvas or a builder - is present at any one time.
  void set_delegate(SkCanvas* canvas);

  // Clears the old delegate and sets the builder delegate to the indicated
  // buider (if not nullptr). This ensures that only one delegate - either
  // a canvas or a builder - is present at any one time.
  void set_delegate(DisplayListBuilder* builder);
  void set_delegate(sk_sp<DisplayListBuilder> builder) {
    set_delegate(builder.get());
  }
  void set_delegate(DisplayListCanvasRecorder& recorder) {
    set_delegate(recorder.builder().get());
  }

  class AutoRestore {
   public:
    ~AutoRestore();

   protected:
    LayerStateStack* layer_state_stack_;

   private:
    AutoRestore(LayerStateStack* stack);
    friend class LayerStateStack;

    const size_t stack_restore_count_;
  };

  static constexpr int CALLER_CAN_APPLY_OPACITY = 0x1;
  static constexpr int CALLER_CAN_APPLY_COLOR_FILTER = 0x2;
  static constexpr int CALLER_CAN_APPLY_IMAGE_FILTER = 0x4;
  static constexpr int CALLER_CAN_APPLY_ANYTHING = 0x7;

  class MutatorContext : public AutoRestore {
   public:
    // Immediately executes a saveLayer with all accumulated state
    // onto the canvas or builder to be applied at the next matching
    // restore. A saveLayer is always executed by this method even if
    // there are no outstanding attributes.
    void saveLayer(const SkRect& bounds);

    // Records the opacity for application at the next call to
    // saveLayer or applyState. A saveLayer may be executed at
    // this time if the opacity cannot be batched with other
    // outstanding attributes.
    void applyOpacity(const SkRect& bounds, SkScalar opacity);

    // Records the image filter for application at the next call to
    // saveLayer or applyState. A saveLayer may be executed at
    // this time if the image filter cannot be batched with other
    // outstanding attributes.
    // (Currently only opacity is recorded for batching)
    void applyImageFilter(const SkRect& bounds,
                          const std::shared_ptr<const DlImageFilter>& filter);

    // Records the color filter for application at the next call to
    // saveLayer or applyState. A saveLayer may be executed at
    // this time if the color filter cannot be batched with other
    // outstanding attributes.
    // (Currently only opacity is recorded for batching)
    void applyColorFilter(const SkRect& bounds,
                          const std::shared_ptr<const DlColorFilter>& filter);

    // Saves the state stack and immediately executes a saveLayer
    // with the indicated backdrop filter and any outstanding
    // state attributes. Since the backdrop filter only applies
    // to the pixels alrady on the screen when this call is made,
    // the backdrop filter will only be applied to the canvas or
    // builder installed at the time that this call is made, and
    // subsequent canvas or builder objects that are made delegates
    // will only see a saveLayer with the indicated blend_mode.
    void applyBackdropFilter(const SkRect& bounds,
                             const std::shared_ptr<const DlImageFilter>& filter,
                             DlBlendMode blend_mode);

    void translate(SkScalar tx, SkScalar ty);
    void transform(const SkM44& m44);
    void transform(const SkMatrix& matrix);
    void integralTransform();

    void clipRect(const SkRect& rect, bool is_aa);
    void clipRRect(const SkRRect& rrect, bool is_aa);
    void clipPath(const SkPath& path, bool is_aa);

   private:
    MutatorContext(LayerStateStack* stack) : AutoRestore(stack) {}
    friend class LayerStateStack;
  };

  // Apply the outstanding state via saveLayer if necessary,
  // respecting the flags representing which potentially
  // outstanding attributes the calling layer can apply
  // themselves.
  //
  // A saveLayer may or may not be sent to the delegates depending
  // on how the outstanding state intersects with the flags supplied
  // by the caller.
  //
  // An AutoRestore instance will always be returned even if there
  // was no saveLayer applied.
  [[nodiscard]] AutoRestore applyState(const SkRect& bounds,
                                       int can_apply_flags = 0);

  SkScalar outstanding_opacity() { return outstanding_.opacity; }

  std::shared_ptr<const DlColorFilter> outstanding_color_filter() {
    return outstanding_.color_filter;
  }

  std::shared_ptr<const DlImageFilter> outstanding_image_filter() {
    return outstanding_.image_filter;
  }

  SkRect outstanding_bounds() { return outstanding_.save_layer_bounds; }

  // Fill the provided paint object with any oustanding attributes and
  // return a pointer to it, or return a nullptr if there were no
  // outstanding attributes to paint with.
  SkPaint* fill(SkPaint& paint) { return outstanding_.fill(paint); }

  // Fill the provided paint object with any oustanding attributes and
  // return a pointer to it, or return a nullptr if there were no
  // outstanding attributes to paint with.
  DlPaint* fill(DlPaint& paint) const { return outstanding_.fill(paint); }

  // Tests if painting content with the current outstanding attributes
  // will produce any content.
  bool needs_painting() const { return outstanding_.opacity > 0; }

  // Tests if painting content with the given bounds will produce any output.
  // This method also tests whether the outstanding attributes will allow
  // output to be produced, but then goes on to test if the supplied bounds
  // will fall within the current clip bounds based on the transform.
  bool needs_painting(const SkRect& bounds) const;

  // Saves the current state of the state stack and returns a
  // MutatorContext which can be used to manipulate the state.
  // The state stack will be restored to its current state
  // when the MutatorContext object goes out of scope.
  [[nodiscard]] MutatorContext save();

 private:
  size_t stack_count() { return state_stack_.size(); }
  void restore_to_count(size_t restore_count);
  void reapply_all(SkCanvas* canvas, DisplayListBuilder* builder);

  void apply_last_entry() {
    state_stack_.back()->apply(&outstanding_, canvas_, builder_);
  }

  // The push methods simply push an associated StateEntry on the stack
  // and then apply it to the current canvas and builder.
  // ---------------------
  void push_attributes();
  void push_opacity(const SkRect& rect, SkScalar opacity);
  void push_color_filter(const SkRect& bounds,
                         const std::shared_ptr<const DlColorFilter>& filter);
  void push_image_filter(const SkRect& bounds,
                         const std::shared_ptr<const DlImageFilter>& filter);
  void push_backdrop(const SkRect& bounds,
                     const std::shared_ptr<const DlImageFilter>& filter,
                     DlBlendMode blend_mode);

  void push_translate(SkScalar tx, SkScalar ty);
  void push_transform(const SkM44& matrix);
  void push_transform(const SkMatrix& matrix);
  void push_integral_transform();

  void push_clip_rect(const SkRect& rect, bool is_aa);
  void push_clip_rrect(const SkRRect& rrect, bool is_aa);
  void push_clip_path(const SkPath& path, bool is_aa);
  // ---------------------

  // The maybe/needs_save_layer methods will determine if the indicated
  // attribute can be incorporated into the outstanding attributes as is,
  // or if the apply_flags are compatible with the outstanding attributes.
  // If the oustanding attributes are incompatible with the new attribute
  // or the apply flags, then a protective saveLayer will be executed.
  // ---------------------
  bool needs_save_layer(int flags) const;
  void save_layer(const SkRect& bounds);
  void maybe_save_layer_for_transform();
  void maybe_save_layer_for_clip();
  void maybe_save_layer(int apply_flags);
  void maybe_save_layer(SkScalar opacity);
  void maybe_save_layer(const std::shared_ptr<const DlColorFilter>& filter);
  void maybe_save_layer(const std::shared_ptr<const DlImageFilter>& filter);
  // ---------------------

  struct RenderingAttributes {
    // We need to record the last bounds we received for the last
    // attribute that we recorded so that we can perform a saveLayer
    // on the proper area. When an attribute is applied that cannot
    // be merged with the existing attributes, it will be submitted
    // with a bounds for its own source content, not the bounds for
    // the content that will be included in the saveLayer that applies
    // the existing outstanding attributes - thus we need to record
    // the bounds that were supplied with the most recent previous
    // attribute to be applied.
    SkRect save_layer_bounds{0, 0, 0, 0};

    SkScalar opacity = SK_Scalar1;
    std::shared_ptr<const DlColorFilter> color_filter;
    std::shared_ptr<const DlImageFilter> image_filter;

    SkPaint* fill(SkPaint& paint,
                  DlBlendMode mode = DlBlendMode::kSrcOver) const;
    DlPaint* fill(DlPaint& paint,
                  DlBlendMode mode = DlBlendMode::kSrcOver) const;

    bool operator==(const RenderingAttributes& other) const {
      return save_layer_bounds == other.save_layer_bounds &&
             opacity == other.opacity &&
             Equals(color_filter, other.color_filter) &&
             Equals(image_filter, other.image_filter);
    }
  };

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
    SaveLayerEntry(const SkRect& bounds,
                   DlBlendMode blend_mode,
                   bool checkerboard)
        : bounds_(bounds),
          blend_mode_(blend_mode),
          do_checkerboard_(checkerboard) {}

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   protected:
    const SkRect bounds_;
    const DlBlendMode blend_mode_;
    const bool do_checkerboard_;

    void do_checkerboard(SkCanvas* canvas,
                         DisplayListBuilder* builder) const override;
  };

  class OpacityEntry : public StateEntry {
   public:
    OpacityEntry(const SkRect& bounds, SkScalar opacity)
        : bounds_(bounds), opacity_(opacity) {}

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   private:
    const SkRect bounds_;
    const SkScalar opacity_;
  };

  class ImageFilterEntry : public StateEntry {
   public:
    ImageFilterEntry(const SkRect& bounds,
                     const std::shared_ptr<const DlImageFilter>& filter)
        : bounds_(bounds), filter_(filter) {}
    ~ImageFilterEntry() override = default;

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   private:
    const SkRect bounds_;
    const std::shared_ptr<const DlImageFilter> filter_;
  };

  class ColorFilterEntry : public StateEntry {
   public:
    ColorFilterEntry(const SkRect& bounds,
                     const std::shared_ptr<const DlColorFilter>& filter)
        : bounds_(bounds), filter_(filter) {}
    ~ColorFilterEntry() override = default;

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

   private:
    const SkRect bounds_;
    const std::shared_ptr<const DlColorFilter> filter_;
  };

  class BackdropFilterEntry : public SaveLayerEntry {
   public:
    BackdropFilterEntry(const SkRect& bounds,
                        const std::shared_ptr<const DlImageFilter>& filter,
                        DlBlendMode blend_mode,
                        bool checkerboard)
        : SaveLayerEntry(bounds, blend_mode, checkerboard), filter_(filter) {}
    ~BackdropFilterEntry() override = default;

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;

    void reapply(RenderingAttributes* attributes,
                 SkCanvas* canvas,
                 DisplayListBuilder* builder) const override;

   private:
    const std::shared_ptr<const DlImageFilter> filter_;
    friend class LayerStateStack;
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

  class IntegralTransformEntry : public StateEntry {
   public:
    IntegralTransformEntry() {}

    void apply(RenderingAttributes* attributes,
               SkCanvas* canvas,
               DisplayListBuilder* builder) const override;
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
  friend class MutatorContext;

  SkCanvas* canvas_ = nullptr;
  int restore_count_ = 0;
  DisplayListBuilder* builder_ = nullptr;
  RenderingAttributes outstanding_;

  bool do_checkerboard_ = false;
  friend class SaveLayerEntry;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_LAYER_STATE_STACK_H_
