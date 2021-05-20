
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_COMMON_ENGINE_CONTEXT_H_
#define SHELL_COMMON_ENGINE_CONTEXT_H_

#include "flutter/common/task_runners.h"
#include "flutter/fml/memory/weak_ptr.h"

namespace flutter {

class DartSnapshot;

class SnapshotDelegate;

class HintFreedDelegate;

class IOManager;

class SkiaUnrefQueue;

class ImageDecoder;

class ImageGeneratorRegistry;

class VolatilePathTracker;

/// @brief  The subset of engine-owned state which is passed through the
///         runtime controller and accessible via the UIDartState.
struct EngineContext {
  EngineContext(const TaskRunners& task_runners);

  EngineContext(const TaskRunners& task_runners,
                fml::WeakPtr<SnapshotDelegate> snapshot_delegate,
                fml::WeakPtr<HintFreedDelegate> hint_freed_delegate,
                fml::WeakPtr<IOManager> io_manager,
                fml::RefPtr<SkiaUnrefQueue> unref_queue,
                fml::WeakPtr<ImageDecoder> image_decoder,
                fml::WeakPtr<ImageGeneratorRegistry> image_generator_registry,
                std::string advisory_script_uri,
                std::string advisory_script_entrypoint,
                std::shared_ptr<VolatilePathTracker> volatile_path_tracker);

  /// The task runners used by the shell hosting this runtime controller. This
  /// may be used by the isolate to scheduled asynchronous texture uploads or
  /// post tasks to the platform task runner.
  const TaskRunners task_runners;

  /// The snapshot delegate used by the
  /// isolate to gather raster snapshots
  /// of Flutter view hierarchies.
  fml::WeakPtr<SnapshotDelegate> snapshot_delegate;

  /// The delegate used by the isolate to hint the Dart VM when additional
  /// memory may be freed if a GC ran at the next NotifyIdle.
  fml::WeakPtr<HintFreedDelegate> hint_freed_delegate;

  /// The IO manager used by the isolate for asynchronous texture uploads.
  fml::WeakPtr<IOManager> io_manager;

  /// The unref queue used by the isolate to collect resources that may
  /// reference resources on the GPU.
  fml::RefPtr<SkiaUnrefQueue> unref_queue;

  /// The image decoder.
  fml::WeakPtr<ImageDecoder> image_decoder;

  /// Cascading registry of image generator builders. Given compressed image
  /// bytes as input, this is used to find and create image generators, which
  /// can then be used for image decoding.
  fml::WeakPtr<ImageGeneratorRegistry> image_generator_registry;

  /// The advisory script URI (only used for debugging). This does not affect
  /// the code being run in the isolate in any way.
  std::string advisory_script_uri;

  /// The advisory script entrypoint (only used for debugging). This does not
  /// affect the code being run in the isolate in any way. The isolate must be
  /// transitioned to the running state explicitly by the caller.
  std::string advisory_script_entrypoint;

  /// Cache for tracking path volatility.
  std::shared_ptr<VolatilePathTracker> volatile_path_tracker;
};

}  // namespace flutter

#endif  // SHELL_COMMON_ENGINE_CONTEXT_H_
