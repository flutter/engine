// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import { engineRevision } from "./revision.js";

export const loadCanvasKit = (deps, config, browserEnvironment) => {
  window.flutterCanvasKitLoaded = new Promise((resolve, reject) => {
    const supportsChromiumCanvasKit = browserEnvironment.hasChromiumBreakIterators && browserEnvironment.hasImageCodecs;
    if (!supportsChromiumCanvasKit && config.canvasKitVariant == "chromium") {
      throw "Chromium CanvasKit variant specifically requested, but unsupported in this browser";
    }
    const useChromiumCanvasKit = supportsChromiumCanvasKit && (config.canvasKitVariant !== "full");
    const baseUrl = config.canvasKitBaseUrl ?? `https://www.gstatic.com/flutter-canvaskit/${engineRevision}/`;
    let canvasKitUrl = useChromiumCanvasKit ? `${baseUrl}/chromium/canvaskit.js` : `${baseUrl}canvaskit.js`;
    if (deps.flutterTT.policy) {
      canvasKitUrl = deps.flutterTT.policy.createScriptURL(canvasKitUrl);
    }
    const script = document.createElement("script");
    script.src = canvasKitUrl;
    script.addEventListener('load', async () => {
      try {
        const canvasKit = await CanvasKitInit();
        window.flutterCanvasKit = canvasKit;
        resolve(canvasKit);  
      } catch (e) {
        reject(e);
      }
    });
    script.addEventListener('error', (e) => {
      reject(e);
    });
    document.head.appendChild(script);
  });
  return window.flutterCanvasKitLoaded;
}
