// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/external_texture.h"

namespace flutter {

const GlProcs& ResolveGlFunctions() {
  static struct GlProcs procs = {};
  static bool initialized = false;

  if (!initialized) {
    procs.glGenTextures =
        reinterpret_cast<glGenTexturesProc>(eglGetProcAddress("glGenTextures"));
    procs.glDeleteTextures = reinterpret_cast<glDeleteTexturesProc>(
        eglGetProcAddress("glDeleteTextures"));
    procs.glBindTexture =
        reinterpret_cast<glBindTextureProc>(eglGetProcAddress("glBindTexture"));
    procs.glTexParameteri = reinterpret_cast<glTexParameteriProc>(
        eglGetProcAddress("glTexParameteri"));
    procs.glTexImage2D =
        reinterpret_cast<glTexImage2DProc>(eglGetProcAddress("glTexImage2D"));

    procs.valid = procs.glGenTextures && procs.glDeleteTextures &&
                  procs.glBindTexture && procs.glTexParameteri &&
                  procs.glTexImage2D;
    initialized = true;
  }
  return procs;
}

}  // namespace flutter
