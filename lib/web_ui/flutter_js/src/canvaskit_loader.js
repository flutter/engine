// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import { createWasmInstantiator } from "./instantiate_wasm.js";

export const loadCanvasKit = async (deps, config, browserEnvironment, engineRevision) => {
  window.flutterCanvasKitLoaded = (async () => {
    if (window.flutterCanvasKit) {
      // The user has set this global variable ahead of time, so we just return that.
      return window.flutterCanvasKit;
    }
    const supportsChromiumCanvasKit = browserEnvironment.hasChromiumBreakIterators && browserEnvironment.hasImageCodecs;
    if (!supportsChromiumCanvasKit && config.canvasKitVariant == "chromium") {
      throw "Chromium CanvasKit variant specifically requested, but unsupported in this browser";
    }
    const useChromiumCanvasKit = supportsChromiumCanvasKit && (config.canvasKitVariant !== "full");
    let baseUrl = config.canvasKitBaseUrl ?? `https://www.gstatic.com/flutter-canvaskit/${engineRevision}/`;
    if (useChromiumCanvasKit) {
      baseUrl = `${baseUrl}chromium/`;
    }
    let canvasKitUrl = `${baseUrl}canvaskit.js`;
    if (deps.flutterTT.policy) {
      canvasKitUrl = deps.flutterTT.policy.createScriptURL(canvasKitUrl);
    }
    const wasmInstantiator = createWasmInstantiator(`${baseUrl}canvaskit.wasm`);
    const canvasKitModule = await import(canvasKitUrl);
    window.flutterCanvasKit = await canvasKitModule.default({
      instantiateWasm: wasmInstantiator,
    });
    return window.flutterCanvasKit;
  })();
  return window.flutterCanvasKitLoaded;
}
