// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_WEB_UI_SKWASM_SURFACE_H_
#define FLUTTER_LIB_WEB_UI_SKWASM_SURFACE_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <emscripten.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/threading.h>
#include <webgl/webgl1.h>
#include <cassert>
#include "export.h"
#include "render_strategy.h"
#include "skwasm_support.h"
#include "wrappers.h"

namespace Skwasm {
// This must be kept in sync with the `ImageByteFormat` enum in dart:ui.
enum class ImageByteFormat {
  rawRgba,
  rawStraightRgba,
  rawUnmodified,
  png,
};

class TextureSourceWrapper {
 public:
  TextureSourceWrapper(unsigned long threadId, SkwasmObject textureSource)
      : _rasterThreadId(threadId) {
    skwasm_setAssociatedObjectOnThread(_rasterThreadId, this, textureSource);
  }

  ~TextureSourceWrapper() {
    skwasm_disposeAssociatedObjectOnThread(_rasterThreadId, this);
  }

  SkwasmObject getTextureSource() { return skwasm_getAssociatedObject(this); }

 private:
  unsigned long _rasterThreadId;
};

class Surface {
 public:
  using CallbackHandler = void(uint32_t, void*, SkwasmObject);

  // Main thread only
  Surface();

  unsigned long getThreadId() { return _thread; }

  // Main thread only
  void dispose();
  uint32_t renderPictures(Picture** picture, int count);
  uint32_t rasterizeImage(Image* image, ImageByteFormat format);
  void setCallbackHandler(CallbackHandler* callbackHandler);
  void onRenderComplete(uint32_t callbackId, SkwasmObject imageBitmap);

  // Any thread
  std::unique_ptr<TextureSourceWrapper> createTextureSourceWrapper(
      SkwasmObject textureSource);

  // Worker thread
  void renderPicturesOnWorker(sk_sp<Picture>* picture,
                              int pictureCount,
                              uint32_t callbackId,
                              double rasterStart);

 private:
  void _runWorker();
  void _init();
  void _dispose();
  void _resizeCanvasToFit(int width, int height);
  void _recreateSurface();
  void _rasterizeImage(Image* image,
                       ImageByteFormat format,
                       uint32_t callbackId);
  void _onRasterizeComplete(SkData* data, uint32_t callbackId);

  std::string _canvasID;
  CallbackHandler* _callbackHandler = nullptr;
  uint32_t _currentCallbackId = 0;

  int _canvasWidth = 0;
  int _canvasHeight = 0;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE _glContext = 0;
  sk_sp<GraphicsContext> _grContext = nullptr;
  sk_sp<GraphicsSurface> _surface = nullptr;
  int _sampleCount;
  int _stencil;

  pthread_t _thread;

  static void fDispose(Surface* surface);
  static void fOnRenderComplete(Surface* surface,
                                uint32_t callbackId,
                                SkwasmObject imageBitmap);
  static void fRasterizeImage(Surface* surface,
                              Image* image,
                              ImageByteFormat format,
                              uint32_t callbackId);
  static void fOnRasterizeComplete(Surface* surface,
                                   SkData* imageData,
                                   uint32_t callbackId);
};
}  // namespace Skwasm

#endif  // FLUTTER_LIB_WEB_UI_SKWASM_SURFACE_H_
