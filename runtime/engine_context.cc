// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/engine_context.h"
#include "flutter/flow/skia_gpu_object.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace flutter {

EngineContext::EngineContext()
    : task_runners("io.flutter." DART_VM_SERVICE_ISOLATE_NAME,
                   nullptr,
                   nullptr,
                   nullptr,
                   nullptr) {}

EngineContext::EngineContext(
    TaskRunners task_runners,
    fml::WeakPtr<SnapshotDelegate> snapshot_delegate,
    fml::WeakPtr<HintFreedDelegate> hint_freed_delegate,
    fml::WeakPtr<IOManager> io_manager,
    fml::RefPtr<SkiaUnrefQueue> unref_queue,
    fml::WeakPtr<ImageDecoder> image_decoder,
    fml::WeakPtr<ImageGeneratorRegistry> image_generator_registry,
    std::string advisory_script_uri,
    std::string advisory_script_entrypoint,
    std::shared_ptr<VolatilePathTracker> volatile_path_tracker)
    : task_runners(task_runners),
      snapshot_delegate(snapshot_delegate),
      hint_freed_delegate(hint_freed_delegate),
      io_manager(io_manager),
      unref_queue(unref_queue),
      image_decoder(image_decoder),
      image_generator_registry(image_generator_registry),
      advisory_script_uri(advisory_script_uri),
      advisory_script_entrypoint(advisory_script_entrypoint),
      volatile_path_tracker(volatile_path_tracker) {}

}  // namespace flutter
