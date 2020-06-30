// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/flutter_windows_texture_registrar.h"

#include "flutter/shell/platform/windows/flutter_windows_engine.h"

namespace flutter {

FlutterWindowsTextureRegistrar::FlutterWindowsTextureRegistrar(
    FlutterWindowsEngine* engine)
    : engine_(engine) {}

int64_t FlutterWindowsTextureRegistrar::RegisterTexture(
    FlutterDesktopTextureCallback texture_callback,
    void* user_data) {
  auto texture_gl =
      std::make_unique<flutter::ExternalTextureGL>(texture_callback, user_data);

  int64_t texture_id = texture_gl->texture_id();
  textures_[texture_id] = std::move(texture_gl);

  if (FlutterEngineRegisterExternalTexture(engine_->engine(), texture_id) ==
      kSuccess) {
    return texture_id;
  }

  return -1;
}

bool FlutterWindowsTextureRegistrar::UnregisterTexture(int64_t texture_id) {
  auto it = textures_.find(texture_id);
  if (it != textures_.end()) {
    textures_.erase(it);
  }
  return (FlutterEngineUnregisterExternalTexture(engine_->engine(),
                                                 texture_id) == kSuccess);
}

bool FlutterWindowsTextureRegistrar::MarkTextureFrameAvailable(
    int64_t texture_id) {
  return (FlutterEngineMarkExternalTextureFrameAvailable(
              engine_->engine(), texture_id) == kSuccess);
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
