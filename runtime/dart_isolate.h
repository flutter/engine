// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_RUNTIME_DART_ISOLATE_H_
#define FLUTTER_RUNTIME_DART_ISOLATE_H_

#include <string>

#include "flutter/common/task_runners.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/window.h"
#include "lib/fxl/compiler_specific.h"
#include "lib/fxl/macros.h"
#include "lib/tonic/dart_state.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace blink {
class DartVM;

class DartIsolate : public UIDartState {
 public:
  enum class Phase {
    Unknown,
    Uninitialized,
    Initialized,
    LibrariesSetup,
    Ready,
    Running,
    Shutdown,
  };

  static fxl::WeakPtr<DartIsolate> CreateRootIsolate(
      const DartVM* vm,
      TaskRunners task_runners,
      std::unique_ptr<Window> window,
      fml::WeakPtr<GrContext> resource_context,
      fxl::RefPtr<flow::SkiaUnrefQueue> unref_queue,
      std::string advisory_script_uri = "main.dart",
      std::string advisory_script_entrypoint = "main");

  Phase GetPhase() const;

  FXL_WARN_UNUSED_RESULT
  bool PrepareForRunningFromPrecompiledCode();

  FXL_WARN_UNUSED_RESULT
  bool PrepareForRunningFromKernel(std::vector<uint8_t> kernel);

  FXL_WARN_UNUSED_RESULT
  bool PrepareForRunningFromScriptSnapshot(
      std::vector<uint8_t> script_snapshot);

  FXL_WARN_UNUSED_RESULT
  bool PrepareForRunningFromSource(const std::string& main_source_file,
                                   const std::string& packages);

  FXL_WARN_UNUSED_RESULT
  bool Run(const std::string& entrypoint);

  FXL_WARN_UNUSED_RESULT
  bool Shutdown();

 private:
  friend class DartVM;

  const DartVM* vm_ = nullptr;
  Phase phase_ = Phase::Unknown;

  DartIsolate(const DartVM* vm,
              TaskRunners task_runners,
              fml::WeakPtr<GrContext> resource_context,
              fxl::RefPtr<flow::SkiaUnrefQueue> unref_queue,
              std::string advisory_script_uri,
              std::string advisory_script_entrypoint);

  ~DartIsolate() override;

  FXL_WARN_UNUSED_RESULT
  bool Initialize(Dart_Isolate isolate);

  FXL_WARN_UNUSED_RESULT
  bool LoadLibraries();

  bool UpdateThreadPoolNames() const;

  const DartVM* GetDartVM() const;

  FXL_WARN_UNUSED_RESULT
  bool MarkIsolateRunnable();

  // |Dart_IsolateCreateCallback|
  static Dart_Isolate DartIsolateCreateCallback(
      const char* advisory_script_uri,
      const char* advisory_script_entrypoint,
      const char* package_root,
      const char* package_config,
      Dart_IsolateFlags* flags,
      DartIsolate* embedder_isolate,
      char** error);

  static std::pair<Dart_Isolate /* vm */,
                   fxl::WeakPtr<DartIsolate> /* embedder */>
  CreateDartVMAndEmbedderObjectPair(const char* advisory_script_uri,
                                    const char* advisory_script_entrypoint,
                                    const char* package_root,
                                    const char* package_config,
                                    Dart_IsolateFlags* flags,
                                    DartIsolate* parent_embedder_isolate,
                                    char** error);

  // |Dart_IsolateShutdownCallback|
  static void DartIsolateShutdownCallback(DartIsolate* embedder_isolate);

  // |Dart_IsolateCleanupCallback|
  static void DartIsolateCleanupCallback(DartIsolate* embedder_isolate);

  FXL_DISALLOW_COPY_AND_ASSIGN(DartIsolate);
};

}  // namespace blink

#endif  // FLUTTER_RUNTIME_DART_ISOLATE_H_
