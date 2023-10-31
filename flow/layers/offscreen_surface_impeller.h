// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_OFFSCREEN_SURFACE_IMPELLER_H_
#define FLUTTER_FLOW_LAYERS_OFFSCREEN_SURFACE_IMPELLER_H_

#include "display_list/dl_builder.h"
#include "flow/layers/offscreen_surface.h"
#include "flutter/display_list/dl_canvas.h"

#include "third_party/skia/include/core/SkData.h"
#include "third_party/skia/include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkSize.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace impeller {
class AiksContext;
}

namespace flutter {

class OffscreenSurfaceImpeller : public OffscreenSurfaceBase {
 public:
  OffscreenSurfaceImpeller(
      const std::shared_ptr<impeller::AiksContext>& surface_context,
      const SkISize& size);

  ~OffscreenSurfaceImpeller() override = default;

  sk_sp<SkData> GetRasterData(bool compressed) const override;

  DlCanvas* GetCanvas() override;

  bool IsValid() const override;

 private:
  std::shared_ptr<impeller::AiksContext> surface_context_;
  std::shared_ptr<DisplayListBuilder> builder_ =
      std::make_shared<DisplayListBuilder>(false);

  OffscreenSurfaceImpeller(const OffscreenSurfaceImpeller&) = default;
  OffscreenSurfaceImpeller(OffscreenSurfaceImpeller&&) = delete;
  OffscreenSurfaceImpeller& operator=(const OffscreenSurfaceImpeller&) =
      default;
  OffscreenSurfaceImpeller& operator=(OffscreenSurfaceImpeller&&) = delete;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_OFFSCREEN_SURFACE_IMPELLER_H_
