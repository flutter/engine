// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <Metal/Metal.h>

#include "flutter/fml/macros.h"
#include "impeller/renderer/blit_pass.h"

namespace impeller {

class BlitPassMTL final : public BlitPass {
 public:
  // |RenderPass|
  ~BlitPassMTL() override;

 private:
  friend class CommandBufferMTL;

  id<MTLCommandBuffer> buffer_ = nil;
  std::string label_;
  bool is_valid_ = false;

  explicit BlitPassMTL(id<MTLCommandBuffer> buffer);

  // |RenderPass|
  bool IsValid() const override;

  // |RenderPass|
  void OnSetLabel(std::string label) override;

  // |RenderPass|
  bool EncodeCommands(
      const std::shared_ptr<Allocator>& transients_allocator) const override;

  bool EncodeCommands(id<MTLBlitCommandEncoder> pass) const;

  FML_DISALLOW_COPY_AND_ASSIGN(BlitPassMTL);
};

}  // namespace impeller
