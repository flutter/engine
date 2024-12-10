// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is a little helper function that helps us start the fetch and compilation
// of an emscripten wasm module in parallel with the fetch of its script.
export const createWasmInstantiator = (url) => {
  console.log('## createWasmInstantiator:', url);
  const modulePromise = WebAssembly.compileStreaming(fetch(url));
  console.log('## modulePromise:', modulePromise);
  return (imports, successCallback) => {
    console.log('...wasmInstantiator...');
    (async () => {
      console.log('await modulePromise');
      const module = await modulePromise;
      console.log('## module:', module);
      const instance = await WebAssembly.instantiate(module, imports);
      console.log('## instance:', instance);
      successCallback(instance, module);
    })();
    return {};
  };
}
