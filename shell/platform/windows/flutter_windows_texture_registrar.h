// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_TEXTURE_REGISTRAR_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_TEXTURE_REGISTRAR_H_

#include <map>
#include <memory>

#include "flutter/shell/platform/windows/external_texture_gl.h"

namespace flutter {

class FlutterWindowsEngine;

// An object managing the registration of an external texture.
class FlutterWindowsTextureRegistrar {
 public:
  explicit FlutterWindowsTextureRegistrar(FlutterWindowsEngine* engine);

  // Registers a texture whose pixel data will be obtained using the
  // specified |texture_callback|
  // Returns the non-zero, positive texture id or -1 on error.
  int64_t RegisterTexture(FlutterDesktopTextureCallback texture_callback,
                          void* user_data);

  // Attempts to unregister the texture identified by |texture_id|.
  // Returns true if the texture was successfully unregistered.
  bool UnregisterTexture(int64_t texture_id);

  // Notifies the engine about a new frame being available.
  // Returns true on success.
  bool MarkTextureFrameAvailable(int64_t texture_id);

  // Attempts to populate the given |texture| by copying the
  // contents of the texture identified by |texture_id|.
  // Returns true on success.
  bool PopulateTexture(int64_t texture_id,
                       size_t width,
                       size_t height,
                       FlutterOpenGLTexture* texture);

 private:
  FlutterWindowsEngine* engine_ = nullptr;

  // All registered textures, keyed by their IDs.
  std::map<int64_t, std::unique_ptr<flutter::ExternalTextureGL>> textures_;
};

};  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_TEXTURE_REGISTRAR_H_
