// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

export const loadCanvasKit = (deps, options, browserEnvironment) => {
  window.flutterCanvasKitLoaded = new Promise((resolve, reject) => {
    const supportsChromiumCanvasKit = browserEnvironment.hasChromiumBreakIterators && browserEnvironment.hasImageCodecs;
    if (!supportsChromiumCanvasKit && options.canvasKitVariant == "chromium") {
      throw "Chromium CanvasKit variant specifically requested, but unsupported in this browser";
    }
    const useChromiumCanvasKit = supportsChromiumCanvasKit && (options.canvasKitVariant !== "full");
    const baseUrl = options.canvasKitBaseUrl ?? `https://www.gstatic.com/flutter-canvaskit/${engineVersion}`;
    const canvasKitUrl = useChromiumCanvasKit ? `${baseUrl}/canvaskit.js` : `${baseUrl}/chromium/canvaskit.js`;
    const trustedCanvasKitUrl = deps.flutterTT.policy.createScriptURL(canvasKitUrl);
    const script = document.createElement("script");
    script.src = trustedCanvasKitUrl;
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
    document.body.appendChild(script);
  });
  return window.flutterCanvasKitLoaded;
}
