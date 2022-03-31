// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_CACHEABL_LAYER_H_
#define FLUTTER_FLOW_LAYERS_CACHEABL_LAYER_H_

#include <memory>
#include "flutter/flow/embedded_views.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/raster_cache.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"

namespace flutter {

class CacheableLayer : public Layer {
 public:
  enum class CacheType { kNone, kCurrent, kChildren };

  bool IsCacheable() { return true; }

  virtual void TryToPrepareRasterCache(PrerollContext* context,
                                       const SkMatrix& matrix) {}

  virtual CacheType NeedCaching(PrerollContext* context,
                                const SkMatrix& ctm) = 0;

  virtual ~CacheableLayer() = default;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_CACHEABL_LAYER_H_
