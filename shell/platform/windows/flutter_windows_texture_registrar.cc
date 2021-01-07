// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/flutter_windows_texture_registrar.h"

#include "flutter/shell/platform/windows/flutter_windows_engine.h"

#include <iostream>

namespace flutter {

FlutterWindowsTextureRegistrar::FlutterWindowsTextureRegistrar(
    FlutterWindowsEngine* engine)
    : engine_(engine) {}

int64_t FlutterWindowsTextureRegistrar::RegisterTexture(
    const FlutterDesktopTextureInfo* texture_info) {
  if (texture_info->type != kFlutterDesktopPixelBufferTexture) {
    std::cerr << "Attempted to register texture of unsupport type."
              << std::endl;
    return -1;
  }

  if (!texture_info->pixel_buffer_config.callback) {
    std::cerr << "Invalid pixel buffer texture callback." << std::endl;
    return -1;
  }

  auto texture_gl = std::make_unique<flutter::ExternalTextureGL>(
      engine_, texture_info->pixel_buffer_config.callback,
      texture_info->pixel_buffer_config.user_data);

  int64_t texture_id = texture_gl->texture_id();
  textures_[texture_id] = std::move(texture_gl);

  if (engine_->RegisterExternalTexture(texture_id)) {
    return texture_id;
  }

  return -1;
}

bool FlutterWindowsTextureRegistrar::UnregisterTexture(int64_t texture_id) {
  auto it = textures_.find(texture_id);
  if (it != textures_.end()) {
    textures_.erase(it);
  }
  return engine_->UnregisterExternalTexture(texture_id);
}

bool FlutterWindowsTextureRegistrar::MarkTextureFrameAvailable(
    int64_t texture_id) {
  auto it = textures_.find(texture_id);
  if (it != textures_.end()) {
    return engine_->PostPlatformThreadTask(
        [](void* data) {
          auto texture = reinterpret_cast<ExternalTextureGL*>(data);
          texture->MarkFrameAvailable();
        },
        it->second.get());
  }
  return false;
}

bool FlutterWindowsTextureRegistrar::PopulateTexture(
    int64_t texture_id,
    size_t width,
    size_t height,
    FlutterOpenGLTexture* texture) {
  auto it = textures_.find(texture_id);
  if (it != textures_.end()) {
    return it->second->PopulateTexture(width, height, texture);
  }
  return false;
}

};  // namespace flutter
