// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import { FlutterEntrypointLoader } from './entrypoint_loader.js';
import { FlutterServiceWorkerLoader } from './service_worker_loader.js';
import { FlutterTrustedTypesPolicy } from './trusted_types.js';

/**
 * The public interface of _flutter.loader. Exposes two methods:
 * * loadEntrypoint (which coordinates the default Flutter web loading procedure)
 * * didCreateEngineInitializer (which is called by Flutter to notify that its
 *                              Engine is ready to be initialized)
 */
export class FlutterLoader {
  /**
   * Initializes the Flutter web app.
   * @param {*} options
   * @returns {Promise?} a (Deprecated) Promise that will eventually resolve
   *                     with an EngineInitializer, or will be rejected with
   *                     any error caused by the loader. Or Null, if the user
   *                     supplies an `onEntrypointLoaded` Function as an option.
   */
  async loadEntrypoint(options) {
    const { serviceWorker, ...entrypoint } = options || {};
    // A Trusted Types policy that is going to be used by the loader.
    const flutterTT = new FlutterTrustedTypesPolicy();
    // The FlutterServiceWorkerLoader instance could be injected as a dependency
    // (and dynamically imported from a module if not present).
    const serviceWorkerLoader = new FlutterServiceWorkerLoader();
    serviceWorkerLoader.setTrustedTypesPolicy(flutterTT.policy);
    await serviceWorkerLoader.loadServiceWorker(serviceWorker).catch(e => {
      // Regardless of what happens with the injection of the SW, the show must go on
      console.warn("Exception while loading service worker:", e);
    });
    // The FlutterEntrypointLoader instance could be injected as a dependency
    // (and dynamically imported from a module if not present).
    const entrypointLoader = new FlutterEntrypointLoader();
    entrypointLoader.setTrustedTypesPolicy(flutterTT.policy);
    // Install the `didCreateEngineInitializer` listener where Flutter web expects it to be.
    this.didCreateEngineInitializer =
      entrypointLoader.didCreateEngineInitializer.bind(entrypointLoader);
    return entrypointLoader.loadEntrypoint(entrypoint);
  }
}
