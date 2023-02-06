// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include "impeller/base/backend_cast.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/fenced_command_buffer_vk.h"
#include "impeller/renderer/blit_command.h"
#include "impeller/renderer/context.h"

namespace impeller {

struct BlitCommandEncoderArgsVK {
  std::weak_ptr<const impeller::Context> context;
  vk::Device device;
  std::shared_ptr<FencedCommandBufferVK> command_buffer;
};

/// Mixin for dispatching Metal commands.
struct BlitEncodeVK : BackendCast<BlitEncodeVK, BlitCommand> {
  virtual ~BlitEncodeVK();

  virtual std::string GetLabel() const = 0;

  [[nodiscard]] virtual bool Encode(
      const BlitCommandEncoderArgsVK& args) const = 0;
};

struct BlitCopyTextureToTextureCommandVK
    : public BlitCopyTextureToTextureCommand,
      public BlitEncodeVK {
  ~BlitCopyTextureToTextureCommandVK() override;

  std::string GetLabel() const override;

  [[nodiscard]] bool Encode(
      const BlitCommandEncoderArgsVK& args) const override;
};

struct BlitCopyTextureToBufferCommandVK : public BlitCopyTextureToBufferCommand,
                                          public BlitEncodeVK {
  ~BlitCopyTextureToBufferCommandVK() override;

  std::string GetLabel() const override;

  [[nodiscard]] bool Encode(
      const BlitCommandEncoderArgsVK& args) const override;
};

struct BlitGenerateMipmapCommandVK : public BlitGenerateMipmapCommand,
                                     public BlitEncodeVK {
  ~BlitGenerateMipmapCommandVK() override;

  std::string GetLabel() const override;

  [[nodiscard]] bool Encode(
      const BlitCommandEncoderArgsVK& args) const override;
};

}  // namespace impeller
