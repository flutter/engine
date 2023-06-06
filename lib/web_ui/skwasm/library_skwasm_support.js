// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file adds JavaScript APIs that are accessible to the C++ layer.
// See: https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html#implement-a-c-api-in-javascript

mergeInto(LibraryManager.library, {
  $skwasm_support_setup__postset: 'skwasm_support_setup();',
  $skwasm_support_setup: function() {
    const objectMap = new Map();
    const handleToCanvasMap = new Map();
    let currentUniqueId = 0;
    _skwasm_generateUniqueId = function() {
      return ++currentUniqueId;
    }
    skwasm_generateUniqueId = _skwasm_generateUniqueId;
    skwasm_registerObject = function(id, object) {
      objectMap.set(id, object);
    };
    skwasm_unregisterObject = function(id) {
      objectMap.delete(id);
    }
    skwasm_getObject = function(id) {
      return objectMap.get(id);
    }
    skwasm_transferObjectToThread = function(objectId, threadId) {
      PThread.pthreads[threadId].postMessage({
        skwasmObjectTransfers: new Map([
          [objectId, objectMap.get(objectId)]
        ])
      });
      objectMap.delete(objectId);
    }
    _skwasm_transferObjectToMain = function(objectId) {
      postMessage({
        skwasmObjectTransfers: new Map([
          [objectId, objectMap.get(objectId)]
        ])
      });
      objectMap.delete(objectId);
    }
    _skwasm_registerMessageListener = function(threadId) {
      eventListener = function(event) {
        const transfers = event.data.skwasmObjectTransfers;
        if (!transfers) {
         return;
        }
        transfers.forEach(function(object, objectId) {
         objectMap.set(objectId, object);
        });
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
    _skwasm_createGlTextureFromVideoFrame = function(videoFrameId, width, height) {
      const videoFrame = skwasm_getObject(videoFrameId);
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
    _skwasm_disposeVideoFrame = function(videoFrameId) {
      const videoFrame = skwasm_getObject(videoFrameId);
      videoFrame.close();
      skwasm_unregisterObject(videoFrameId);
    }
  },
  skwasm_generateUniqueId: function() {},
  skwasm_generateUniqueId__deps: ['$skwasm_support_setup'],
  $skwasm_generateUniqueId: function() {},
  $skwasm_generateUniqueId__deps: ['$skwasm_support_setup'],
  $skwasm_registerObject: function() {},
  $skwasm_registerObject__deps: ['$skwasm_support_setup'],
  $skwasm_unregisterObject: function() {},
  $skwasm_unregisterObject__deps: ['$skwasm_support_setup'],
  $skwasm_getObject: function() {},
  $skwasm_getObject__deps: ['$skwasm_support_setup'],
  $skwasm_transferObjectToThread: function() {},
  $skwasm_transferObjectToThread__deps: ['$skwasm_support_setup'],
  skwasm_transferObjectToMain: function() {},
  skwasm_transferObjectToMain__deps: ['$skwasm_support_setup'],
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
  