// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

export const loadSkwasm = (deps, config, browserEnvironment) => {
    return new Promise((resolve, reject) => {
      const baseUrl = config.canvasKitBaseUrl ?? `https://www.gstatic.com/flutter-canvaskit/${engineVersion}`;
      const skwasmUrl = `${baseUrl}/skwasm.js`;
      const trustedSkwasmUrl = deps.flutterTT.policy.createScriptURL(skwasmUrl);
      const script = document.createElement("script");
      script.src = trustedSkwasmUrl;
      script.addEventListener('load', async () => {
        try {
          const skwasmInstance = await skwasm();
          resolve(skwasmInstance);
        } catch (e) {
          reject(e);
        }
      });
      script.addEventListener('error', (e) => {
        reject(e);
      });
      document.head.appendChild(script);
    });
  }
  