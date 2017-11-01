// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/dart_isolate.h"

#include <tuple>

#include "flutter/lib/io/dart_io.h"
#include "flutter/lib/ui/dart_runtime_hooks.h"
#include "flutter/lib/ui/dart_ui.h"
#include "flutter/runtime/dart_service_isolate.h"
#include "flutter/runtime/dart_vm.h"
#include "lib/fxl/files/path.h"
#include "lib/tonic/converter/dart_converter.h"
#include "lib/tonic/dart_class_library.h"
#include "lib/tonic/dart_class_provider.h"
#include "lib/tonic/dart_message_handler.h"
#include "lib/tonic/dart_state.h"
#include "lib/tonic/dart_sticky_error.h"
#include "lib/tonic/file_loader/file_loader.h"
#include "lib/tonic/scopes/dart_api_scope.h"
#include "lib/tonic/scopes/dart_isolate_scope.h"
#include "third_party/dart/runtime/include/dart_tools_api.h"

namespace blink {

fxl::WeakPtr<DartIsolate> DartIsolate::CreateRootIsolate(
    const DartVM* vm,
    TaskRunners task_runners,
    std::unique_ptr<Window> window,
    fml::WeakPtr<GrContext> resource_context,
    fxl::RefPtr<flow::SkiaUnrefQueue> unref_queue,
    std::string advisory_script_uri,
    std::string advisory_script_entrypoint) {
  Dart_Isolate vm_isolate = nullptr;
  fxl::WeakPtr<DartIsolate> embedder_isolate;

  char* error = nullptr;

  // Since this is the root isolate, we fake a parent embedder data object. We
  // cannot use unique_ptr here because the destructor is private (since the
  // isolate lifecycle is entirely managed by the VM).
  DartIsolate* root_embedder_data =
      new DartIsolate(vm,                           // VM
                      task_runners,                 // task runners
                      std::move(resource_context),  // resource context
                      std::move(unref_queue),       // skia unref queue
                      advisory_script_uri,          // advisory URI
                      advisory_script_entrypoint    // advisory entrypoint
      );

  std::tie(vm_isolate, embedder_isolate) = CreateDartVMAndEmbedderObjectPair(
      advisory_script_uri.c_str(),         // advisory script URI
      advisory_script_entrypoint.c_str(),  // advisory script entrypoint
      nullptr,                             // package root
      nullptr,                             // package config
      nullptr,                             // flags
      root_embedder_data,                  // parent embedder data
      &error                               // error (out)
  );

  delete root_embedder_data;

  if (error != nullptr) {
    free(error);
  }

  if (vm_isolate == nullptr) {
    return {};
  }

  if (embedder_isolate) {
    // Only root isolates can interact with windows.
    embedder_isolate->SetWindow(std::move(window));
    embedder_isolate->set_use_blink(vm->GetSettings().using_blink);
  }

  return embedder_isolate;
}

DartIsolate::DartIsolate(const DartVM* vm,
                         TaskRunners task_runners,
                         fml::WeakPtr<GrContext> resource_context,
                         fxl::RefPtr<flow::SkiaUnrefQueue> unref_queue,
                         std::string advisory_script_uri,
                         std::string advisory_script_entrypoint)
    : UIDartState(std::move(task_runners),
                  std::move(resource_context),
                  std::move(unref_queue),
                  advisory_script_uri,
                  advisory_script_entrypoint),
      vm_(vm) {
  if (vm_ == nullptr) {
    return;
  }

  phase_ = Phase::Uninitialized;
}

DartIsolate::~DartIsolate() = default;

DartIsolate::Phase DartIsolate::GetPhase() const {
  return phase_;
}

const DartVM* DartIsolate::GetDartVM() const {
  return vm_;
}

bool DartIsolate::Initialize(Dart_Isolate dart_isolate) {
  if (phase_ != Phase::Uninitialized) {
    return false;
  }

  if (dart_isolate == nullptr) {
    return false;
  }

  if (Dart_CurrentIsolate() != dart_isolate) {
    return false;
  }

  if (Dart_IsolateData(dart_isolate) != this) {
    return false;
  }

  // After this point, isolate scopes can be safely used.
  SetIsolate(dart_isolate);

  // We are entering a new scope (for the first time since initialization) and
  // we want to restore the current scope to null when we exit out of this
  // method. This balances the implicit Dart_EnterIsolate call made by
  // Dart_CreateIsolate (which calls the Initialize).
  Dart_ExitIsolate();

  tonic::DartIsolateScope scope(isolate());

  if (auto task_runner = GetTaskRunners().GetUITaskRunner()) {
    // Isolates may not have any particular thread affinity. Only initialize the
    // message handler if a task runner is explicitly specified.
    message_handler().Initialize(task_runner);
  }

  if (tonic::LogIfError(
          Dart_SetLibraryTagHandler(tonic::DartState::HandleLibraryTag))) {
    return false;
  }

  if (!UpdateThreadPoolNames()) {
    return false;
  }

  phase_ = Phase::Initialized;
  return true;
}

bool DartIsolate::UpdateThreadPoolNames() const {
  // TODO(chinmaygarde): This implementation does not account for multiple
  // shells sharing the same (or subset of) threads.
  const auto& task_runners = GetTaskRunners();

  if (auto task_runner = task_runners.GetGPUTaskRunner()) {
    task_runner->PostTask([label = task_runners.GetLabel() +
                                   std::string{".gpu"}]() {
      Dart_SetThreadName(label.c_str());
    });
  }

  if (auto task_runner = task_runners.GetUITaskRunner()) {
    task_runner->PostTask([label =
                               task_runners.GetLabel() + std::string{".ui"}]() {
      Dart_SetThreadName(label.c_str());
    });
  }

  if (auto task_runner = task_runners.GetIOTaskRunner()) {
    task_runner->PostTask([label =
                               task_runners.GetLabel() + std::string{".io"}]() {
      Dart_SetThreadName(label.c_str());
    });
  }

  if (auto task_runner = task_runners.GetPlatformTaskRunner()) {
    task_runner->PostTask([label = task_runners.GetLabel() +
                                   std::string{".platform"}]() {
      Dart_SetThreadName(label.c_str());
    });
  }

  return true;
}

bool DartIsolate::LoadLibraries() {
  if (phase_ != Phase::Initialized) {
    return false;
  }

  tonic::DartState::Scope scope(this);

  // TODO: Figure out if these make sense for the service isolate.
  DartIO::InitForIsolate();

  DartUI::InitForIsolate();

  const bool is_service_isolate = Dart_IsServiceIsolate(isolate());

  DartRuntimeHooks::Install(is_service_isolate
                                ? DartRuntimeHooks::SecondaryIsolate
                                : DartRuntimeHooks::MainIsolate,
                            GetAdvisoryScriptURI());

  if (!is_service_isolate) {
    class_library().add_provider(
        "ui", std::make_unique<tonic::DartClassProvider>(this, "dart:ui"));
  }

  phase_ = Phase::LibrariesSetup;
  return true;
}

bool DartIsolate::PrepareForRunningFromPrecompiledCode() {
  if (phase_ != Phase::LibrariesSetup) {
    return false;
  }

  if (!DartVM::IsRunningPrecompiledCode()) {
    return false;
  }

  tonic::DartState::Scope scope(this);

  if (Dart_IsNull(Dart_RootLibrary())) {
    return false;
  }

  if (!MarkIsolateRunnable()) {
    return false;
  }

  phase_ = Phase::Ready;
  return true;
}

FXL_WARN_UNUSED_RESULT
bool DartIsolate::PrepareForRunningFromKernel(std::vector<uint8_t> kernel) {
  if (phase_ != Phase::LibrariesSetup) {
    return false;
  }

  if (DartVM::IsRunningPrecompiledCode()) {
    return false;
  }

  if (kernel.size() == 0) {
    return false;
  }

  tonic::DartState::Scope(this);

  if (!Dart_IsNull(Dart_RootLibrary())) {
    return false;
  }

  void* kernel_bytes = ::malloc(kernel.size());
  if (kernel_bytes == nullptr) {
    return false;
  }
  ::memmove(kernel_bytes, kernel.data(), kernel.size());
  void* kernel_program = Dart_ReadKernelBinary(
      static_cast<const uint8_t*>(kernel_bytes), kernel.size(),
      [](uint8_t* buffer) { ::free(buffer); });

  if (tonic::LogIfError(Dart_LoadKernel(kernel_program))) {
    return false;
  }

  if (Dart_IsNull(Dart_RootLibrary())) {
    return false;
  }

  if (!MarkIsolateRunnable()) {
    return false;
  }

  phase_ = Phase::Ready;
  return true;
}

FXL_WARN_UNUSED_RESULT
bool DartIsolate::PrepareForRunningFromScriptSnapshot(
    std::vector<uint8_t> script_snapshot) {
  if (phase_ != Phase::LibrariesSetup) {
    return false;
  }

  if (DartVM::IsRunningPrecompiledCode()) {
    return false;
  }

  if (script_snapshot.size() == 0) {
    return false;
  }

  tonic::DartState::Scope scope(this);

  if (!Dart_IsNull(Dart_RootLibrary())) {
    return false;
  }

  if (tonic::LogIfError(Dart_LoadScriptFromSnapshot(script_snapshot.data(),
                                                    script_snapshot.size()))) {
    return false;
  }

  if (Dart_IsNull(Dart_RootLibrary())) {
    return false;
  }

  if (!MarkIsolateRunnable()) {
    return false;
  }

  phase_ = Phase::Ready;
  return true;
}

bool DartIsolate::PrepareForRunningFromSource(
    const std::string& main_source_file,
    const std::string& packages) {
  if (phase_ != Phase::LibrariesSetup) {
    return false;
  }

  if (DartVM::IsRunningPrecompiledCode()) {
    return false;
  }

  if (main_source_file.empty()) {
    return false;
  }

  tonic::DartState::Scope scope(this);

  if (!Dart_IsNull(Dart_RootLibrary())) {
    return false;
  }

  auto& loader = file_loader();

  if (!packages.empty()) {
    if (!loader.LoadPackagesMap(files::AbsolutePath(packages))) {
      return false;
    }
  }

  if (tonic::LogIfError(
          loader.LoadScript(files::AbsolutePath(main_source_file)))) {
    return false;
  }

  if (Dart_IsNull(Dart_RootLibrary())) {
    return false;
  }

  if (!MarkIsolateRunnable()) {
    return false;
  }

  phase_ = Phase::Ready;
  return true;
}

bool DartIsolate::MarkIsolateRunnable() {
  if (phase_ != Phase::LibrariesSetup) {
    return false;
  }

  // This function may only be called from an active isolate scope.
  if (Dart_CurrentIsolate() != isolate()) {
    return false;
  }

  // There must be no current isolate to mark an isolate as being runnable.
  Dart_ExitIsolate();

  if (!Dart_IsolateMakeRunnable(isolate())) {
    // Failed. Restore the isolate.
    Dart_EnterIsolate(isolate());
    return false;
  }
  // Success. Restore the isolate.
  Dart_EnterIsolate(isolate());
  return true;
}

FXL_WARN_UNUSED_RESULT
bool DartIsolate::Run(const std::string& entrypoint_name) {
  if (phase_ != Phase::Ready) {
    return false;
  }

  tonic::DartState::Scope scope(this);

  Dart_Handle entrypoint = Dart_GetClosure(
      Dart_RootLibrary(), tonic::ToDart(entrypoint_name.c_str()));
  if (tonic::LogIfError(entrypoint)) {
    return false;
  }

  Dart_Handle isolate_lib = Dart_LookupLibrary(tonic::ToDart("dart:isolate"));
  if (tonic::LogIfError(isolate_lib)) {
    return false;
  }

  Dart_Handle isolate_args[] = {
      entrypoint,
      Dart_Null(),
  };

  if (tonic::LogIfError(Dart_Invoke(
          isolate_lib, tonic::ToDart("_startMainIsolate"),
          sizeof(isolate_args) / sizeof(isolate_args[0]), isolate_args))) {
    return false;
  }

  phase_ = Phase::Running;
  return true;
}

bool DartIsolate::Shutdown() {
  // This call may be re-entrant since Dart_ShutdownIsolate can invoke the
  // cleanup callback which deletes the embedder side object of the dart isolate
  // (a.k.a. this).
  if (phase_ == Phase::Shutdown) {
    return false;
  }
  phase_ = Phase::Shutdown;
  Dart_Isolate vm_isolate = isolate();
  // The isolate can be nullptr if this instance is the stub isolate data used
  // during root isolate creation.
  if (vm_isolate != nullptr) {
    // We need to enter the isolate because Dart_ShutdownIsolate does not take
    // the isolate to shutdown as a parameter.
    FXL_DCHECK(Dart_CurrentIsolate() == nullptr);
    Dart_EnterIsolate(vm_isolate);
    Dart_ShutdownIsolate();
    FXL_DCHECK(Dart_CurrentIsolate() == nullptr);
  }
  return true;
}

static Dart_Isolate DartCreateAndStartServiceIsolate(
    const char* advisory_script_uri,
    const char* advisory_script_entrypoint,
    const char* package_root,
    const char* package_config,
    Dart_IsolateFlags* flags,
    char** error) {
  auto vm = DartVM::ForProcessIfInitialized();

  if (!vm) {
    *error = strdup(
        "Could not resolve the VM when attempting to create the service "
        "isolate.");
    return nullptr;
  }

  const auto& settings = vm->GetSettings();

  if (!settings.enable_observatory) {
    return nullptr;
  }

  blink::TaskRunners null_task_runners("io.flutter.vm-service", nullptr,
                                       nullptr, nullptr, nullptr);

  auto service_isolate = DartIsolate::CreateRootIsolate(
      vm.get(),           // vm
      null_task_runners,  // task runners
      nullptr,            // window
      {},                 // resource context
      {},                 // unref queue
      advisory_script_uri == nullptr ? "" : advisory_script_uri,  // script uri
      advisory_script_entrypoint == nullptr
          ? ""
          : advisory_script_entrypoint  // script entrypoint
  );

  if (!service_isolate) {
    *error = strdup("Could not create the service isolate.");
    return nullptr;
  }

  // The engine never holds a strong reference to the VM service isolate. Since
  // we are about to lose our last weak reference to it, start the VM service
  // while we have this reference.

  const bool running_from_sources =
      !DartVM::IsRunningPrecompiledCode() && vm->GetPlatformKernel() == nullptr;

  tonic::DartState::Scope scope(service_isolate.get());
  if (!DartServiceIsolate::Startup(
          settings.ipv6 ? "::1" : "127.0.0.1",  // server IP address
          settings.observatory_port,            // server observatory port
          tonic::DartState::HandleLibraryTag,   // embedder library tag handler
          running_from_sources,                 // running from source code
          false,  //  disable websocket origin check
          error   // error (out)
          )) {
    // Error is populated by call to startup.
    return nullptr;
  }

  return service_isolate->isolate();
}

// |Dart_IsolateCreateCallback|
Dart_Isolate DartIsolate::DartIsolateCreateCallback(
    const char* advisory_script_uri,
    const char* advisory_script_entrypoint,
    const char* package_root,
    const char* package_config,
    Dart_IsolateFlags* flags,
    DartIsolate* parent_embedder_isolate,
    char** error) {
  if (parent_embedder_isolate == nullptr &&
      strcmp(advisory_script_uri, "vm-service") == 0) {
    // The VM attempts to start the VM service for us on |Dart_Initialize|. In
    // such a case, the callback data will be null and the script URI will be
    // "vm-service". In such cases, we just create the service isolate like
    // normal but dont hold a reference to it at all. We also start this isolate
    // since we will never again reference it from the engine.
    return DartCreateAndStartServiceIsolate(advisory_script_uri,         //
                                            advisory_script_entrypoint,  //
                                            package_root,                //
                                            package_config,              //
                                            flags,                       //
                                            error                        //
    );
  }

  return CreateDartVMAndEmbedderObjectPair(
             advisory_script_uri, advisory_script_entrypoint, package_root,
             package_config, flags, parent_embedder_isolate, error)
      .first;
}

std::pair<Dart_Isolate, fxl::WeakPtr<DartIsolate>>
DartIsolate::CreateDartVMAndEmbedderObjectPair(
    const char* advisory_script_uri,
    const char* advisory_script_entrypoint,
    const char* package_root,
    const char* package_config,
    Dart_IsolateFlags* flags,
    DartIsolate* parent_embedder_isolate,
    char** error) {
  if (parent_embedder_isolate == nullptr ||
      parent_embedder_isolate->GetDartVM() == nullptr) {
    *error =
        strdup("Parent isolate did not have embedder specific callback data.");
    return {nullptr, {}};
  }

  const DartVM* vm = parent_embedder_isolate->GetDartVM();

  // Create the native object on the embedder side. This object is deleted in
  // the cleanup callback.
  auto embedder_isolate =
      new DartIsolate(vm,                                             //
                      parent_embedder_isolate->GetTaskRunners(),      //
                      parent_embedder_isolate->GetResourceContext(),  //
                      parent_embedder_isolate->GetSkiaUnrefQueue(),   //
                      advisory_script_uri,                            //
                      advisory_script_entrypoint                      //
      );

  // Create the Dart VM isolate and give it the embedder object as the baton.
  Dart_Isolate isolate =
      vm->GetPlatformKernel() != nullptr
          ? Dart_CreateIsolateFromKernel(advisory_script_uri,         //
                                         advisory_script_entrypoint,  //
                                         vm->GetPlatformKernel(),     //
                                         nullptr,                     //
                                         embedder_isolate,            //
                                         error                        //
                                         )
          : Dart_CreateIsolate(
                advisory_script_uri,                                    //
                advisory_script_entrypoint,                             //
                vm->GetDartSnapshot().GetDefaultIsolateSnapshotData(),  //
                vm->GetDartSnapshot()
                    .GetDefaultIsolateSnapshotInstructions(),  //
                nullptr,                                       //
                embedder_isolate,                              //
                error                                          //
            );

  if (!embedder_isolate->Initialize(isolate)) {
    *error = strdup("Embedder could not initialize the Dart isolate.");
    return {nullptr, {}};
  }

  if (!embedder_isolate->LoadLibraries()) {
    *error =
        strdup("Embedder could not load libraries in the new Dart isolate.");
    return {nullptr, {}};
  }

  // The ownership of the embedder object is controlled by the Dart VM. So the
  // only reference returned to the caller is weak.
  return {isolate, embedder_isolate->GetWeakPtr()};
}

// |Dart_IsolateShutdownCallback|
void DartIsolate::DartIsolateShutdownCallback(DartIsolate* embedder_isolate) {
  if (!tonic::DartStickyError::IsSet()) {
    return;
  }

  tonic::DartApiScope api_scope;
  FXL_LOG(ERROR) << "Isolate " << tonic::StdStringFromDart(Dart_DebugName())
                 << " exited with an error";
  Dart_Handle sticky_error = Dart_GetStickyError();
  FXL_CHECK(tonic::LogIfError(sticky_error));
}

// |Dart_IsolateCleanupCallback|
void DartIsolate::DartIsolateCleanupCallback(DartIsolate* embedder_isolate) {
  delete embedder_isolate;
}

}  // namespace blink
