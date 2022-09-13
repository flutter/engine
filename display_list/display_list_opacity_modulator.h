// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_OPACITY_MODULATOR_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_OPACITY_MODULATOR_H_

#include "flutter/display_list/display_list_delegate.h"

namespace flutter {

class OpacityModulator : public DispatcherDelegate {
 public:
  OpacityModulator(Dispatcher& delegate, SkScalar opacity)
      : DispatcherDelegate(delegate) {
    opacity_stack_.emplace_back(opacity);
    update_color(current_color_, opacity);
  }

  void setColor(DlColor color) override;
  void save() override;
  void saveLayer(const SkRect* bounds,
                 RenderWith with,
                 const DlImageFilter* backdrop,
                 int optimizations) override;
  void restore() override;
  void drawColor(DlColor color, DlBlendMode mode) override;
  void drawImage(const sk_sp<DlImage> image,
                 const SkPoint point,
                 DlImageSampling sampling,
                 RenderWith with) override;
  void drawImageRect(const sk_sp<DlImage> image,
                     const SkRect& src,
                     const SkRect& dst,
                     DlImageSampling sampling,
                     RenderWith with,
                     SkCanvas::SrcRectConstraint constraint) override;
  void drawImageNine(const sk_sp<DlImage> image,
                     const SkIRect& center,
                     const SkRect& dst,
                     DlFilterMode filter,
                     RenderWith with) override;
  void drawImageLattice(const sk_sp<DlImage> image,
                        const SkCanvas::Lattice& lattice,
                        const SkRect& dst,
                        DlFilterMode filter,
                        RenderWith with) override;
  void drawAtlas(const sk_sp<DlImage> atlas,
                 const SkRSXform xform[],
                 const SkRect tex[],
                 const DlColor colors[],
                 int count,
                 DlBlendMode mode,
                 DlImageSampling sampling,
                 const SkRect* cull_rect,
                 RenderWith with) override;
  void drawPicture(const sk_sp<SkPicture> picture,
                   const SkMatrix* matrix,
                   RenderWith with) override;
  void drawDisplayList(const sk_sp<DisplayList> display_list,
                       SkScalar opacity) override;

 private:
  std::vector<SkScalar> opacity_stack_;
  DlColor current_color_ = DlPaint::kDefaultColor;

  SkScalar current_opacity() { return opacity_stack_.back(); }
  void update_color(DlColor color, SkScalar opacity);

  class AutoModulate {
   public:
    AutoModulate(OpacityModulator* modulator, RenderWith with);

    ~AutoModulate();

    RenderWith with() { return with_; }

   private:
    OpacityModulator* modulator_;
    RenderWith with_;
    bool needs_restore_;
  };
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_OPACITY_MODULATOR_H_
