// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_opacity_modulator.h"

namespace flutter {

void OpacityModulator::setColor(DlColor color) {
  if (current_color_ != color) {
    current_color_ = color;
    SkScalar opacity = current_opacity();
    delegate_.setColor(color.modulateOpacity(opacity));
  }
}

void OpacityModulator::save() {
  opacity_stack_.emplace_back(current_opacity());
}

void OpacityModulator::saveLayer(const SkRect* bounds,
                                 RenderWith with,
                                 const DlImageFilter* backdrop,
                                 int optimizations) {
  SkScalar layer_opacity = current_opacity();
  if (bounds == nullptr && (optimizations & kSaveLayerOpacityCompatible) != 0 &&
      backdrop == nullptr) {
    // We know that:
    // - no bounds is needed for clipping here
    // - no backdrop filter is used to initialize the layer
    // - the current attributes only have an alpha
    // - the children are compatible with individually rendering with
    //   an inherited opacity
    // Therefore we can just use a save instead of a saveLayer and pass the
    // intended opacity to the children.
    delegate_.save();
    // If the saveLayer does not use attributes, the children should continue
    // to render with the inherited opacity unmodified. If alpha is to
    // be applied, the children should render with the combination of the
    // inherited opacity combined with the alpha from the current color.
    if (with >= RenderWith::kAlpha) {
      layer_opacity *= current_color_.getAlphaF();
      // The last color forwarded to the delegate will already have included
      // this combined opacity so no need to recompute it.
    }
    opacity_stack_.emplace_back(layer_opacity);
  } else {
    if (layer_opacity >= SK_Scalar1 || with >= RenderWith::kAlpha) {
      delegate_.saveLayer(bounds, with, backdrop, optimizations);
      if (layer_opacity < SK_Scalar1) {
        // The layer will absorb the opacity, so adjust the current color
        // to match what it would have been without the opacity.
        delegate_.setColor(current_color_);
      }
    } else {
      FML_DCHECK(layer_opacity < SK_Scalar1);
      FML_DCHECK(with == RenderWith::kDefaults);
      delegate_.setColor(DlPaint::kDefaultColor.modulateOpacity(layer_opacity));
      delegate_.saveLayer(bounds, RenderWith::kAlpha, backdrop, optimizations);
      delegate_.setColor(current_color_);
    }
    opacity_stack_.emplace_back(SK_Scalar1);
  }
}

void OpacityModulator::restore() {
  SkScalar old_opacity = current_opacity();
  opacity_stack_.pop_back();
  SkScalar new_opacity = current_opacity();
  if (new_opacity != old_opacity) {
    delegate_.setColor(current_color_.modulateOpacity(new_opacity));
  }
}

void OpacityModulator::drawColor(DlColor color, DlBlendMode mode) {
  delegate_.drawColor(color.modulateOpacity(current_opacity()), mode);
}

void OpacityModulator::drawImage(const sk_sp<DlImage> image,
                                 const SkPoint point,
                                 DlImageSampling sampling,
                                 RenderWith with) {
  AutoModulate save(this, with);
  delegate_.drawImage(image, point, sampling, save.with());
}
void OpacityModulator::drawImageRect(const sk_sp<DlImage> image,
                                     const SkRect& src,
                                     const SkRect& dst,
                                     DlImageSampling sampling,
                                     RenderWith with,
                                     SkCanvas::SrcRectConstraint constraint) {
  AutoModulate save(this, with);
  delegate_.drawImageRect(image, src, dst, sampling, save.with(), constraint);
}
void OpacityModulator::drawImageNine(const sk_sp<DlImage> image,
                                     const SkIRect& center,
                                     const SkRect& dst,
                                     DlFilterMode filter,
                                     RenderWith with) {
  AutoModulate save(this, with);
  delegate_.drawImageNine(image, center, dst, filter, save.with());
}
void OpacityModulator::drawImageLattice(const sk_sp<DlImage> image,
                                        const SkCanvas::Lattice& lattice,
                                        const SkRect& dst,
                                        DlFilterMode filter,
                                        RenderWith with) {
  AutoModulate save(this, with);
  delegate_.drawImageLattice(image, lattice, dst, filter, save.with());
}
void OpacityModulator::drawAtlas(const sk_sp<DlImage> atlas,
                                 const SkRSXform xform[],
                                 const SkRect tex[],
                                 const DlColor colors[],
                                 int count,
                                 DlBlendMode mode,
                                 DlImageSampling sampling,
                                 const SkRect* cull_rect,
                                 RenderWith with) {
  AutoModulate save(this, with);
  delegate_.drawAtlas(atlas, xform, tex, colors, count, mode, sampling,
                      cull_rect, save.with());
}
void OpacityModulator::drawPicture(const sk_sp<SkPicture> picture,
                                   const SkMatrix* matrix,
                                   RenderWith with) {
  AutoModulate save(this, with);
  delegate_.drawPicture(picture, matrix, save.with());
}

void OpacityModulator::drawDisplayList(const sk_sp<DisplayList> display_list,
                                       SkScalar opacity) {
  delegate_.drawDisplayList(display_list, opacity * current_opacity());
}

void OpacityModulator::update_color(DlColor color, SkScalar opacity) {
  delegate_.setColor(color.modulateOpacity(opacity));
}

OpacityModulator::AutoModulate::AutoModulate(OpacityModulator* modulator,
                                             RenderWith with)
    : modulator_(modulator), with_(with), needs_restore_(false) {
  switch (with) {
    case RenderWith::kDefaults: {
      SkScalar opacity = modulator->current_opacity();
      if (opacity < SK_Scalar1) {
        with_ = RenderWith::kAlpha;
        needs_restore_ = true;
        modulator->delegate_.setColor(
            DlColor::kBlack().modulateOpacity(opacity));
      }
      return;
    }
    case RenderWith::kAlpha:
    case RenderWith::kDlPaint:
      return;
  }
  FML_DCHECK(false);
}

OpacityModulator::AutoModulate::~AutoModulate() {
  if (needs_restore_) {
    modulator_->update_color(modulator_->current_color_,
                             modulator_->current_opacity());
  }
}

}  // namespace flutter
