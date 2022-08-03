// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/renderer/backend/vulkan/vk.h"
#include "impeller/renderer/context.h"
#include "impeller/renderer/surface.h"

namespace impeller {

class SurfaceVK final : public Surface {
 public:
  SurfaceVK(RenderTarget target, vk::UniqueSurfaceKHR surface);

  // |Surface|
  ~SurfaceVK() override;

  vk::SurfaceKHR GetSurface() const;

 private:
  vk::UniqueSurfaceKHR surface_;

  // |Surface|
  bool Present() const override;

  FML_DISALLOW_COPY_AND_ASSIGN(SurfaceVK);
};

}  // namespace impeller
