// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLOW_TESTING_MOCK_LAYER_H_
#define FLOW_TESTING_MOCK_LAYER_H_

#include "flutter/flow/layers/layer.h"

namespace flutter {
namespace testing {

class MockLayer : public Layer {
 public:
  MockLayer(SkPath path,
            SkPaint paint = SkPaint(),
            bool fake_has_platform_view = false,
            bool fake_needs_system_composite = false);
  ~MockLayer() override = default;

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;
  void Paint(PaintContext& context) const override;

  void SetChild(std::shared_ptr<Layer> child) { child_layer_ = child; }

  void ExpectMutators(const std::vector<Mutator>& stack);
  void ExpectParentMatrix(const SkMatrix& matrix);
  void ExpectParentCullRect(const SkRect& rect);
  void ExpectParentElevation(float elevation);
  void ExpectParentHasPlatformView(bool has_platform_view);

 private:
  std::shared_ptr<Layer> child_layer_;

  MutatorsStack parent_mutators_;
  SkMatrix parent_matrix_;
  SkRect parent_cull_rect_;
  float parent_elevation_ = 0;
  bool parent_has_platform_view_ = false;

  SkPath fake_paint_path_;
  SkPaint fake_paint_;
  bool fake_has_platform_view_;
  bool fake_needs_system_composite_;

  FML_DISALLOW_COPY_AND_ASSIGN(MockLayer);
};

}  // namespace testing
}  // namespace flutter

#endif  // FLOW_TESTING_MOCK_LAYER_H_
