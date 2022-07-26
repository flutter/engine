// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/gles/blit_pass_gles.h"

#include <algorithm>

#include "flutter/fml/trace_event.h"
#include "impeller/base/config.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/backend/gles/device_buffer_gles.h"
#include "impeller/renderer/backend/gles/formats_gles.h"
#include "impeller/renderer/backend/gles/pipeline_gles.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"
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

static void DeleteFBO(const ProcTableGLES& gl, GLuint fbo, GLenum type) {
  if (fbo != GL_NONE) {
    gl.BindFramebuffer(type, GL_NONE);
    gl.DeleteFramebuffers(1u, &fbo);
  }
};

static std::optional<GLuint> ConfigureFBO(
    const ProcTableGLES& gl,
    const std::shared_ptr<Texture>& texture,
    GLenum fbo_type) {
  auto handle = TextureGLES::Cast(texture.get())->GetGLHandle();
  if (!handle.has_value()) {
    return std::nullopt;
  }

  if (TextureGLES::Cast(*texture).IsWrapped()) {
    // The texture is attached to the default FBO, so there's no need to
    // create/configure one.
    gl.BindFramebuffer(fbo_type, 0);
    return 0;
  }

  GLuint fbo;
  gl.GenFramebuffers(1u, &fbo);
  gl.BindFramebuffer(fbo_type, fbo);

  if (!TextureGLES::Cast(*texture).SetAsFramebufferAttachment(
          fbo_type, fbo, TextureGLES::AttachmentPoint::kColor0)) {
    VALIDATION_LOG << "Could not attach texture to framebuffer.";
    DeleteFBO(gl, fbo, fbo_type);
    return std::nullopt;
  }

  if (gl.CheckFramebufferStatus(fbo_type) != GL_FRAMEBUFFER_COMPLETE) {
    VALIDATION_LOG << "Could not create a complete frambuffer.";
    DeleteFBO(gl, fbo, fbo_type);
    return std::nullopt;
  }

  return fbo;
};

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
      // glBlitFramebuffer is a GLES3 proc. Since we target GLES2, we need to
      // emulate the blit when it's not available in the driver.
      if (!gl.BlitFramebuffer.IsAvailable()) {
        // TODO(bdero): Emulate the blit using a raster draw call here.
        return true;
      }

      GLuint read_fbo = GL_NONE;
      GLuint draw_fbo = GL_NONE;
      fml::ScopedCleanupClosure delete_fbos([&gl, &read_fbo, &draw_fbo]() {
        DeleteFBO(gl, read_fbo, GL_READ_FRAMEBUFFER);
        DeleteFBO(gl, draw_fbo, GL_DRAW_FRAMEBUFFER);
      });

      {
        auto read = ConfigureFBO(gl, copy_command->source, GL_READ_FRAMEBUFFER);
        if (!read.has_value()) {
          return false;
        }
        read_fbo = read.value();
      }

      {
        auto draw =
            ConfigureFBO(gl, copy_command->destination, GL_DRAW_FRAMEBUFFER);
        if (!draw.has_value()) {
          return false;
        }
        draw_fbo = draw.value();
      }

      gl.Disable(GL_SCISSOR_TEST);
      gl.Disable(GL_DEPTH_TEST);
      gl.Disable(GL_STENCIL_TEST);

      gl.BlitFramebuffer(copy_command->source_region.origin.x,     // srcX0
                         copy_command->source_region.origin.y,     // srcY0
                         copy_command->source_region.size.width,   // srcX1
                         copy_command->source_region.size.height,  // srcY1
                         copy_command->destination_origin.x,       // dstX0
                         copy_command->destination_origin.y,       // dstY0
                         copy_command->source_region.size.width,   // dstX1
                         copy_command->source_region.size.height,  // dstY1
                         GL_COLOR_BUFFER_BIT,                      // mask
                         GL_NEAREST                                // filter
      );

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
