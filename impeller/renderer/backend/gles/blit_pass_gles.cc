// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/gles/blit_pass_gles.h"

#include <algorithm>

#include "GLES2/gl2.h"
#include "flutter/fml/trace_event.h"
#include "impeller/base/config.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/backend/gles/device_buffer_gles.h"
#include "impeller/renderer/backend/gles/formats_gles.h"
#include "impeller/renderer/backend/gles/pipeline_gles.h"
#include "impeller/renderer/backend/gles/texture_gles.h"
#include "impeller/renderer/formats.h"

namespace impeller {

BlitPassGLES::BlitPassGLES(ReactorGLES::Ref reactor)
    : reactor_(std::move(reactor)),
      is_valid_(reactor_ && reactor_->IsValid()) {}

// |BlitPass|
BlitPassGLES::~BlitPassGLES() = default;

// |BlitPass|
bool BlitPassGLES::IsValid() const {
  return is_valid_;
}

// |BlitPass|
void BlitPassGLES::OnSetLabel(std::string label) {
  label_ = std::move(label);
}

[[nodiscard]] bool EncodeCommandsInReactor(
    const std::shared_ptr<Allocator>& transients_allocator,
    const ReactorGLES& reactor,
    const std::vector<BlitCommand>& commands,
    const std::string& label) {
  TRACE_EVENT0("impeller", __FUNCTION__);

  if (commands.empty()) {
    return true;
  }

  const auto& gl = reactor.GetProcTable();

  fml::ScopedCleanupClosure pop_pass_debug_marker(
      [&gl]() { gl.PopDebugGroup(); });
  if (!label.empty()) {
    gl.PushDebugGroup(label);
  } else {
    pop_pass_debug_marker.Release();
  }

  for (const auto& command : commands) {
    fml::ScopedCleanupClosure pop_cmd_debug_marker(
        [&gl]() { gl.PopDebugGroup(); });
    if (!command.label.empty()) {
      gl.PushDebugGroup(command.label);
    } else {
      pop_cmd_debug_marker.Release();
    }

    if (auto* copy_command =
            std::get_if<BlitCommand::CopyTextureToTexture>(&command.data)) {
      if (!gl.BindFramebuffer.IsAvailable()) {
        // TODO(bdero): Emulate the blit using a raster draw call here.
        return true;
      }
      gl.Disable(GL_SCISSOR_TEST);
      gl.Disable(GL_DEPTH_TEST);
      gl.Disable(GL_STENCIL_TEST);
      auto source = TextureGLES::Cast(copy_command->source.get());

    }

    else if (auto* mipmap_command =
                 std::get_if<BlitCommand::GenerateMipmaps>(&command.data)) {
      auto texture = TextureGLES::Cast(mipmap_command->texture.get());
      if (!texture->GenerateMipmaps()) {
        return false;
      }
    }
  }

  return true;
}

// |BlitPass|
bool BlitPassGLES::EncodeCommands(
    const std::shared_ptr<Allocator>& transients_allocator) const {
  if (!IsValid()) {
    return false;
  }
  if (commands_.empty()) {
    return true;
  }

  return reactor_->AddOperation([transients_allocator, commands = commands_,
                                 label = label_](const auto& reactor) {
    auto result =
        EncodeCommandsInReactor(transients_allocator, reactor, commands, label);
    FML_CHECK(result) << "Must be able to encode GL commands without error.";
  });
}

}  // namespace impeller
