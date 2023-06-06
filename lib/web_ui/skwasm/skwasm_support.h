// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <emscripten/threading.h>
#include <cinttypes>

namespace Skwasm {
class Surface;
}

using SkwasmObjectId = uint32_t;

extern "C" {
extern SkwasmObjectId skwasm_generateUniqueId();
extern void skwasm_transferObjectToMain(SkwasmObjectId objectId);
extern void skwasm_transferObjectToThread(SkwasmObjectId objectId,
                                          pthread_t threadId);
extern void skwasm_registerMessageListener(pthread_t threadId);
extern uint32_t skwasm_createOffscreenCanvas(int width, int height);
extern void skwasm_resizeCanvas(uint32_t contextHandle, int width, int height);
extern void skwasm_captureImageBitmap(Skwasm::Surface* surfaceHandle,
                                      uint32_t contextHandle,
                                      uint32_t bitmapId,
                                      int width,
                                      int height);
extern unsigned int skwasm_createGlTextureFromVideoFrame(
    SkwasmObjectId videoFrameId,
    int width,
    int height);
extern void skwasm_disposeVideoFrame(SkwasmObjectId videoFrameId);
}
