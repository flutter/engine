// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "flutter/impeller/renderer/backend/gles/reactor_gles.h"
#include "flutter/impeller/renderer/blit_pass.h"

namespace impeller {

class BlitPassGLES final : public BlitPass {
 public:
  // |BlitPass|
  ~BlitPassGLES() override;

 private:
  friend class CommandBufferGLES;

  ReactorGLES::Ref reactor_;
  std::string label_;
  bool is_valid_ = false;

  explicit BlitPassGLES(ReactorGLES::Ref reactor);

  // |BlitPass|
  bool IsValid() const override;

  // |BlitPass|
  void OnSetLabel(std::string label) override;

  // |BlitPass|
  bool EncodeCommands(
      const std::shared_ptr<Allocator>& transients_allocator) const override;

  FML_DISALLOW_COPY_AND_ASSIGN(BlitPassGLES);
};

}  // namespace impeller
