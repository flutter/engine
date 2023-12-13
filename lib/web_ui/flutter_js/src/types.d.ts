// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

type JSCompileTarget = "dart2js" | "dartdevc";
type WasmCompileTarget = "dart2wasm";

export type CompileTarget = JSCompileTarget | WasmCompileTarget;

export type WebRenderer =
    "html" |
    "canvaskit" |
    "skwasm";

interface ApplicationBuildBase {
    renderer: WebRenderer;
}

export interface JSApplicationBuild extends ApplicationBuildBase {
    compileTarget: JSCompileTarget;
    mainJsPath: String;
}

export interface WasmApplicationBuild extends ApplicationBuildBase {
    compileTarget: WasmCompileTarget;
    mainWasmPath: String;
    jsSupportRuntimePath: String;
}

export type ApplicationBuild = JSApplicationBuild | WasmApplicationBuild;

export interface BuildConfig {
    serviceWorkerVersion: string;
    builds: ApplicationBuild[];
}

export interface BrowserEnvironment {
    hasImageCodecs: boolean;
    hasChromiumBreakIterators: boolean; 
    supportsWasmGC: boolean;
    crossOriginIsolated: boolean;
}
