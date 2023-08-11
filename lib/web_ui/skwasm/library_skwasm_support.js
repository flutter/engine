// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file adds JavaScript APIs that are accessible to the C++ layer.
// See: https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html#implement-a-c-api-in-javascript

mergeInto(LibraryManager.library, {
  $skwasm_support_setup__postset: 'skwasm_support_setup();',
  $skwasm_support_setup: function() {
    const handleToCanvasMap = new Map();
    _skwasm_invokeFunctionOnThread = function(functionIndex, threadId, ...arguments) {
      PThread.pthreads[threadId].postMessage({
        functionIndex,
        arguments,
      });
    }
    _skwasm_invokeFunctionOnMain = function(functionIndex, ...args) {
      postMessage({
        functionIndex,
        args,
      });
    }
    _skwasm_registerMessageListener = function(threadId) {
      eventListener = function(event) {
        const { functionIndex, args } = event.data;
        if (!functionIndex || !args) {
          return;
        }
        const invokeFunction = getWasmTableEntry(functionIndex);
        invokeFunction(...args);
      };
      if (!threadId) {
        addEventListener("message", eventListener);
      } else {
        PThread.pthreads[threadId].addEventListener("message", eventListener);
      }
    };
    _skwasm_createOffscreenCanvas = function(width, height) {
      const canvas = new OffscreenCanvas(width, height);
      var contextAttributes = {
        'majorVersion': 2,
        'alpha': true,
        'depth': true,
        'stencil': true,
        'antialias': false,
        'premultipliedAlpha': true,
        'preserveDrawingBuffer': false,
        'powerPreference': 'default',
        'failIfMajorPerformanceCaveat': false,
        'enableExtensionsByDefault': true,
      };
      const contextHandle = GL.createContext(canvas, contextAttributes);
      handleToCanvasMap.set(contextHandle, canvas);
      return contextHandle;
    }
    _skwasm_resizeCanvas = function(contextHandle, width, height) {
      const canvas = handleToCanvasMap.get(contextHandle);
      canvas.width = width;
      canvas.height = height;
    }
    _skwasm_captureImageBitmap = async function(surfaceHandle, contextHandle, bitmapId, width, height) {
      const canvas = handleToCanvasMap.get(contextHandle);
      const imageBitmap = await createImageBitmap(canvas, 0, 0, width, height);
      objectMap.set(bitmapId, imageBitmap);
      _surface_onCaptureComplete(surfaceHandle, bitmapId);
    }
    _skwasm_createGlTextureFromVideoFrame = function(videoFrame, width, height) {
      const glCtx = GL.currentContext.GLctx;
      const newTexture = glCtx.createTexture();
      glCtx.bindTexture(glCtx.TEXTURE_2D, newTexture);
      glCtx.pixelStorei(glCtx.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
      
      glCtx.texImage2D(glCtx.TEXTURE_2D, 0, glCtx.RGBA, width, height, 0, glCtx.RGBA, glCtx.UNSIGNED_BYTE, videoFrame);

      glCtx.pixelStorei(glCtx.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
      glCtx.bindTexture(glCtx.TEXTURE_2D, null);

      const textureId = GL.getNewId(GL.textures);
      GL.textures[textureId] = newTexture;
      return textureId;
    }
    _skwasm_disposeVideoFrame = function(videoFrame) {
      videoFrame.close();
    }
  },
  $skwasm_invokeFunctionOnThread: function() {},
  $skwasm_invokeFunctionOnThread__deps: ['$skwasm_support_setup'],
  skwasm_invokeFunctionOnMain: function() {},
  skwasm_invokeFunctionOnMain__deps: ['$skwasm_support_setup'],
  skwasm_registerMessageListener: function() {},
  skwasm_registerMessageListener__deps: ['$skwasm_support_setup'],
  skwasm_createOffscreenCanvas: function () {},
  skwasm_createOffscreenCanvas__deps: ['$skwasm_support_setup'],
  skwasm_resizeCanvas: function () {},
  skwasm_resizeCanvas__deps: ['$skwasm_support_setup'],
  skwasm_captureImageBitmap: function () {},
  skwasm_captureImageBitmap__deps: ['$skwasm_support_setup'],
  skwasm_createGlTextureFromVideoFrame: function () {},
  skwasm_createGlTextureFromVideoFrame__deps: ['$skwasm_support_setup', '$skwasm_getObject'],
  skwasm_disposeVideoFrame: function () {},
  skwasm_disposeVideoFrame__deps: ['$skwasm_support_setup', '$skwasm_getObject', '$skwasm_unregisterObject'],
});
  