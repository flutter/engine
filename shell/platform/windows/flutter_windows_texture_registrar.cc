// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/flutter_windows_texture_registrar.h"

#include <iostream>
#include <mutex>

#include "flutter/shell/platform/windows/external_texture_d3d.h"
#include "flutter/shell/platform/windows/external_texture_pixelbuffer.h"
#include "flutter/shell/platform/windows/flutter_windows_engine.h"

namespace flutter {

FlutterWindowsTextureRegistrar::FlutterWindowsTextureRegistrar(
    FlutterWindowsEngine* engine)
    : engine_(engine) {}

int64_t FlutterWindowsTextureRegistrar::RegisterTexture(
    const FlutterDesktopTextureInfo* texture_info) {
  const auto& gl_procs = ResolveGlFunctions();
  if (!gl_procs.valid) {
    std::cerr << "Unable to resolve GL functions" << std::endl;
    return -1;
  }

  if (texture_info->type == kFlutterDesktopPixelBufferTexture) {
    if (!texture_info->pixel_buffer_config.callback) {
      std::cerr << "Invalid pixel buffer texture callback." << std::endl;
      return -1;
    }

    return EmplaceTexture(std::make_unique<flutter::ExternalTexturePixelBuffer>(
        texture_info->pixel_buffer_config.callback,
        texture_info->pixel_buffer_config.user_data, gl_procs));
  } else if (texture_info->type == kFlutterDesktopGpuSurfaceTexture &&
             texture_info->gpu_surface_config.type ==
                 kFlutterDesktopGpuSurfaceTypeDxgi ==
                 kFlutterDesktopGpuSurfaceTypeDxgi) {
    if (!texture_info->gpu_surface_config.callback) {
      std::cerr << "Invalid DXGI surface callback." << std::endl;
      return -1;
    }

    return EmplaceTexture(std::make_unique<flutter::ExternalTextureD3d>(
        texture_info->gpu_surface_config.callback,
        texture_info->gpu_surface_config.user_data, engine_->surface_manager(),
        gl_procs));
  }

  std::cerr << "Attempted to register texture of unsupport type." << std::endl;
  return -1;
}

int64_t FlutterWindowsTextureRegistrar::EmplaceTexture(
    std::unique_ptr<ExternalTexture> texture) {
  int64_t texture_id = texture->texture_id();
  {
    std::lock_guard<std::mutex> lock(map_mutex_);
    textures_[texture_id] = std::move(texture);
  }

  engine_->task_runner()->RunNowOrPostTask([engine = engine_, texture_id]() {
    engine->RegisterExternalTexture(texture_id);
  });

  return texture_id;
}

bool FlutterWindowsTextureRegistrar::UnregisterTexture(int64_t texture_id) {
  {
    std::lock_guard<std::mutex> lock(map_mutex_);
    auto it = textures_.find(texture_id);
    if (it == textures_.end()) {
      return false;
    }
    textures_.erase(it);
  }

  engine_->task_runner()->RunNowOrPostTask([engine = engine_, texture_id]() {
    engine->UnregisterExternalTexture(texture_id);
  });
  return true;
}

bool FlutterWindowsTextureRegistrar::MarkTextureFrameAvailable(
    int64_t texture_id) {
  engine_->task_runner()->RunNowOrPostTask([engine = engine_, texture_id]() {
    engine->MarkExternalTextureFrameAvailable(texture_id);
  });
  return true;
}

bool FlutterWindowsTextureRegistrar::PopulateTexture(
    int64_t texture_id,
    size_t width,
    size_t height,
    FlutterOpenGLTexture* opengl_texture) {
  flutter::ExternalTexture* texture;
  {
    std::lock_guard<std::mutex> lock(map_mutex_);
    auto it = textures_.find(texture_id);
    if (it == textures_.end()) {
      return false;
    }
    texture = it->second.get();
  }
  return texture->PopulateTexture(width, height, opengl_texture);
}

};  // namespace flutter
