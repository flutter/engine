// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/metal/blit_pass_mtl.h"
#include <Metal/Metal.h>
#include <variant>

#include "flutter/fml/closure.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "impeller/base/backend_cast.h"
#include "impeller/renderer/backend/metal/device_buffer_mtl.h"
#include "impeller/renderer/backend/metal/formats_mtl.h"
#include "impeller/renderer/backend/metal/pipeline_mtl.h"
#include "impeller/renderer/backend/metal/sampler_mtl.h"
#include "impeller/renderer/backend/metal/texture_mtl.h"
#include "impeller/renderer/blit_command.h"
#include "impeller/renderer/formats.h"
#include "impeller/renderer/host_buffer.h"
#include "impeller/renderer/shader_types.h"

namespace impeller {

BlitPassMTL::BlitPassMTL(id<MTLCommandBuffer> buffer) : buffer_(buffer) {
  if (!buffer_) {
    return;
  }
  is_valid_ = true;
}

BlitPassMTL::~BlitPassMTL() = default;

bool BlitPassMTL::IsValid() const {
  return is_valid_;
}

void BlitPassMTL::OnSetLabel(std::string label) {
  if (label.empty()) {
    return;
  }
  label_ = std::move(label);
}

bool BlitPassMTL::EncodeCommands(
    const std::shared_ptr<Allocator>& transients_allocator) const {
  TRACE_EVENT0("impeller", "BlitPassMTL::EncodeCommands");
  if (!IsValid()) {
    return false;
  }

  auto blit_command_encoder = [buffer_ blitCommandEncoder];

  if (!blit_command_encoder) {
    return false;
  }

  if (!label_.empty()) {
    [blit_command_encoder setLabel:@(label_.c_str())];
  }

  // Success or failure, the pass must end. The buffer can only process one pass
  // at a time.
  fml::ScopedCleanupClosure auto_end(
      [blit_command_encoder]() { [blit_command_encoder endEncoding]; });

  return EncodeCommands(blit_command_encoder);
}

bool BlitPassMTL::EncodeCommands(id<MTLBlitCommandEncoder> encoder) const {
  fml::closure pop_debug_marker = [encoder]() { [encoder popDebugGroup]; };
  for (const auto& command : commands_) {
    fml::ScopedCleanupClosure auto_pop_debug_marker(pop_debug_marker);
    if (!command.label.empty()) {
      [encoder pushDebugGroup:@(command.label.c_str())];
    } else {
      auto_pop_debug_marker.Release();
    }

    if (auto* copy_command =
            std::get_if<BlitCommand::CopyTextureToTexture>(&command.data)) {
      auto source = TextureMTL::Cast(*copy_command->source).GetMTLTexture();
      if (!source) {
        return false;
      }

      auto destination =
          TextureMTL::Cast(*copy_command->source).GetMTLTexture();
      if (!destination) {
        return false;
      }

      auto source_origin =
          MTLOriginMake(copy_command->source_region.origin.x,
                        copy_command->source_region.origin.y, 0);
      auto source_size =
          MTLSizeMake(copy_command->source_region.size.width,
                      copy_command->source_region.size.height, 1);
      auto destination_origin =
          MTLOriginMake(copy_command->destination_origin.x,
                        copy_command->destination_origin.y, 0);

      [encoder copyFromTexture:source
                   sourceSlice:0
                   sourceLevel:0
                  sourceOrigin:source_origin
                    sourceSize:source_size
                     toTexture:destination
              destinationSlice:0
              destinationLevel:0
             destinationOrigin:destination_origin];
    }

    else if (auto* mipmap_command =
                 std::get_if<BlitCommand::GenerateMipmaps>(&command.data)) {
      auto texture = TextureMTL::Cast(*mipmap_command->texture).GetMTLTexture();
      if (!texture) {
        return false;
      }

      [encoder generateMipmapsForTexture:texture];
    }
  }
  return true;
}

}  // namespace impeller
