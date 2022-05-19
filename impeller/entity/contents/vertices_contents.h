// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "flutter/fml/macros.h"
#include "impeller/entity/contents/contents.h"
#include "impeller/geometry/path.h"
#include "impeller/geometry/point.h"
#include "impeller/renderer/sampler_descriptor.h"
#include "impeller/geometry/vertices.h"

namespace impeller {

class VerticesContents final : public Contents {
 public:
  explicit VerticesContents(Vertices vertices);

  ~VerticesContents() override;

  // |Contents|
  std::optional<Rect> GetCoverage(const Entity& entity) const override;

  // |Contents|
  bool Render(const ContentContext& renderer,
              const Entity& entity,
              RenderPass& pass) const override;

 public:
  Vertices vertices_;

  FML_DISALLOW_COPY_AND_ASSIGN(VerticesContents);
};

}  // namespace impeller
