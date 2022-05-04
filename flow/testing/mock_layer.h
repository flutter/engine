// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLOW_TESTING_MOCK_LAYER_H_
#define FLOW_TESTING_MOCK_LAYER_H_

#include <functional>
#include <memory>
#include "flutter/flow/diff_context.h"
#include "flutter/flow/layer_raster_cache_item.h"
#include "flutter/flow/layers/cacheable_layer.h"
#include "flutter/flow/layers/container_layer.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/raster_cache.h"

namespace flutter {
namespace testing {

// Mock implementation of the |Layer| interface that does nothing but paint
// the specified |path| into the canvas.  It records the |PrerollContext| and
// |PaintContext| data passed in by its parent |Layer|, so the test can later
// verify the data against expected values.
class MockLayer : public Layer {
 public:
  explicit MockLayer(SkPath path,
                     SkPaint paint = SkPaint(),
                     bool fake_has_platform_view = false,
                     bool fake_reads_surface = false,
                     bool fake_opacity_compatible_ = false);

  static std::shared_ptr<MockLayer> Make(SkPath path,
                                         SkPaint paint = SkPaint()) {
    return std::make_shared<MockLayer>(path, paint, false, false, false);
  }

  static std::shared_ptr<MockLayer> MakeOpacityCompatible(SkPath path) {
    return std::make_shared<MockLayer>(path, SkPaint(), false, false, true);
  }

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;
  void Paint(PaintContext& context) const override;

  const MutatorsStack& parent_mutators() { return parent_mutators_; }
  const SkMatrix& parent_matrix() { return parent_matrix_; }
  const SkRect& parent_cull_rect() { return parent_cull_rect_; }
  bool parent_has_platform_view() { return parent_has_platform_view_; }

  bool IsReplacing(DiffContext* context, const Layer* layer) const override;
  void Diff(DiffContext* context, const Layer* old_layer) override;
  const MockLayer* as_mock_layer() const override { return this; }

 private:
  MutatorsStack parent_mutators_;
  SkMatrix parent_matrix_;
  SkRect parent_cull_rect_ = SkRect::MakeEmpty();
  SkPath fake_paint_path_;
  SkPaint fake_paint_;
  bool parent_has_platform_view_ = false;
  bool fake_has_platform_view_ = false;
  bool fake_reads_surface_ = false;
  bool fake_opacity_compatible_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(MockLayer);
};

class MockLayerCacheableItem : public LayerRasterCacheItem {
 public:
  using LayerRasterCacheItem::LayerRasterCacheItem;
};

class MockCacheableContainerLayer : public CacheableContainerLayer {
 public:
  static void FirstTryToCacheChildren(int render_count,
                                      LayerRasterCacheItem* entry) {
    // if (render_count < 3) {
    //   render_count++;
    //   entry->set_need_cached(false);
    //   return;
    // }

    return;
  }

  static void FirstNotTryToCache(int render_count, RasterCacheItem* entry) {
    // if render count < 3 not cache anything
    // if (render_count < 3) {
    //   render_count++;
    //   entry->set_need_cached(false);
    //   return;
    // }
    // if render count >= 3 try cache itself
    return;
  }

  explicit MockCacheableContainerLayer(
      std::function<void(int, LayerRasterCacheItem*)> strategy =
          FirstTryToCacheChildren)
      : CacheableContainerLayer(), cache_strategy_(strategy) {}

  Layer* asLayer() { return this; }

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

 private:
  std::function<void(int, LayerRasterCacheItem*)> cache_strategy_;
  std::unique_ptr<LayerRasterCacheItem> raster_cache_item_;
};

class MockCacheableLayer : public MockLayer {
 public:
  explicit MockCacheableLayer(SkPath path,
                              SkPaint paint = SkPaint(),
                              int render_limit = 3,
                              bool fake_has_platform_view = false,
                              bool fake_reads_surface = false,
                              bool fake_opacity_compatible = false)
      : MockLayer(path,
                  paint,
                  fake_has_platform_view,
                  fake_reads_surface,
                  fake_opacity_compatible) {
    raster_cache_item_ = std::make_unique<MockLayerCacheableItem>(this, 3);
  }

  Layer* asLayer() { return this; }

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

 private:
  std::unique_ptr<LayerRasterCacheItem> raster_cache_item_;
  // int render_limit_ = 3;
  // int render_count_ = 1;
};

}  // namespace testing
}  // namespace flutter

#endif  // FLOW_TESTING_MOCK_LAYER_H_
