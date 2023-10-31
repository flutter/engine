// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_OFFSCREEN_SURFACE_H_
#define FLUTTER_FLOW_LAYERS_OFFSCREEN_SURFACE_H_

#include "flutter/display_list/dl_canvas.h"
#include "flutter/display_list/skia/dl_sk_canvas.h"
#include "third_party/skia/include/core/SkData.h"
#include "third_party/skia/include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkSize.h"
#include "third_party/skia/include/core/SkSurface.h"

class GrDirectContext;

namespace flutter {

class OffscreenSurfaceBase {
 public:
  virtual ~OffscreenSurfaceBase() {}

  virtual sk_sp<SkData> GetRasterData(bool compressed) const = 0;

  virtual DlCanvas* GetCanvas() = 0;

  virtual bool IsValid() const = 0;
};

class OffscreenSurfaceSkia : public OffscreenSurfaceBase {
 public:
  explicit OffscreenSurfaceSkia(GrDirectContext* surface_context,
                                const SkISize& size);

  ~OffscreenSurfaceSkia() override = default;

  sk_sp<SkData> GetRasterData(bool compressed) const override;

  DlCanvas* GetCanvas() override;

  bool IsValid() const override;

 private:
  sk_sp<SkSurface> offscreen_surface_;
  DlSkCanvasAdapter adapter_;

  OffscreenSurfaceSkia(const OffscreenSurfaceSkia&) = default;
  OffscreenSurfaceSkia(OffscreenSurfaceSkia&&) = delete;
  OffscreenSurfaceSkia& operator=(const OffscreenSurfaceSkia&) = default;
  OffscreenSurfaceSkia& operator=(OffscreenSurfaceSkia&&) = delete;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_OFFSCREEN_SURFACE_H_
