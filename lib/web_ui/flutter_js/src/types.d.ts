// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

export type CompileTarget =
    "dart2js" |
    "dartdevc" |
    "dart2wasm";

export type WebRenderer =
    "html" |
    "canvaskit" |
    "skwasm";

export interface ApplicationBuild {
    target: CompileTarget;
    renderer: WebRenderer;
}

export interface BuildConfig {
    serviceWorkerVersion: string;
    builds: ApplicationBuild[];
    url: string?;
}

export interface BrowserEnvironment {
    hasImageCodecs: boolean;
    hasChromiumBreakIterators: boolean; 
    supportsWasmGC: boolean;
    crossOriginIsolated: boolean;
}
