// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/compositing/scene_host.h"

#include <lib/ui/scenic/cpp/view_token_pair.h>

#include "flutter/flow/scene_update_context.h"
#include "flutter/fml/thread_local.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/third_party/tonic/dart_args.h"
#include "flutter/third_party/tonic/dart_binding_macros.h"
#include "flutter/third_party/tonic/logging/dart_invoke.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace {

struct SceneHostBindingKey {
  std::string isolate_service_id;
  scenic::ResourceId resource_id;

  SceneHostBindingKey(const scenic::ResourceId resource_id,
                      const std::string isolate_service_id) {
    this->resource_id = resource_id;
    this->isolate_service_id = isolate_service_id;
  }

  bool operator==(const SceneHostBindingKey& other) const {
    return isolate_service_id == other.isolate_service_id &&
           resource_id == other.resource_id;
  }
};

struct SceneHostBindingKeyHasher {
  std::size_t operator()(const SceneHostBindingKey& key) const {
    std::size_t resource_id_hash =
        std::hash<scenic::ResourceId>()(key.resource_id);
    std::size_t isolate_hash = std::hash<std::string>()(key.isolate_service_id);
    return resource_id_hash ^ isolate_hash;
  }
};

using SceneHostBindings = std::unordered_map<SceneHostBindingKey,
                                             flutter::SceneHost*,
                                             SceneHostBindingKeyHasher>;

static SceneHostBindings scene_host_bindings;

void SceneHost_constructor(Dart_NativeArguments args) {
  flutter::UIDartState::ThrowIfUIOperationsProhibited();
  tonic::DartCallConstructor(&flutter::SceneHost::Create, args);
}

flutter::SceneHost* GetSceneHost(scenic::ResourceId id,
                                 std::string isolate_service_id) {
  auto binding =
      scene_host_bindings.find(SceneHostBindingKey(id, isolate_service_id));
  if (binding == scene_host_bindings.end()) {
    return nullptr;
  } else {
    return binding->second;
  }
}

flutter::SceneHost* GetSceneHostForCurrentIsolate(scenic::ResourceId id) {
  auto isolate = Dart_CurrentIsolate();
  if (!isolate) {
    return nullptr;
  } else {
    std::string isolate_service_id = Dart_IsolateServiceId(isolate);
    return GetSceneHost(id, isolate_service_id);
  }
}

void InvokeDartClosure(const tonic::DartPersistentValue& closure) {
  auto dart_state = closure.dart_state().lock();
  if (!dart_state) {
    return;
  }

  tonic::DartState::Scope scope(dart_state);
  auto dart_handle = closure.value();

  FML_DCHECK(dart_handle && !Dart_IsNull(dart_handle) &&
             Dart_IsClosure(dart_handle));
  tonic::DartInvoke(dart_handle, {});
}

template <typename T>
void InvokeDartFunction(const tonic::DartPersistentValue& function, T& arg) {
  auto dart_state = function.dart_state().lock();
  if (!dart_state) {
    return;
  }

  tonic::DartState::Scope scope(dart_state);
  auto dart_handle = function.value();

  FML_DCHECK(dart_handle && !Dart_IsNull(dart_handle) &&
             Dart_IsClosure(dart_handle));
  tonic::DartInvoke(dart_handle, {tonic::ToDart(arg)});
}

}  // namespace

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, SceneHost);

#define FOR_EACH_BINDING(V) \
  V(SceneHost, dispose)     \
  V(SceneHost, setProperties)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void SceneHost::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register({{"SceneHost_constructor", SceneHost_constructor, 5, true},
                     FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

fml::RefPtr<SceneHost> SceneHost::Create(
    fml::RefPtr<zircon::dart::Handle> viewHolderToken,
    Dart_Handle viewConnectedCallback,
    Dart_Handle viewDisconnectedCallback,
    Dart_Handle viewStateChangedCallback) {
  return fml::MakeRefCounted<SceneHost>(viewHolderToken, viewConnectedCallback,
                                        viewDisconnectedCallback,
                                        viewStateChangedCallback);
}

void SceneHost::OnViewConnected(scenic::ResourceId id) {
  auto* scene_host = GetSceneHostForCurrentIsolate(id);

  if (scene_host && !scene_host->view_connected_callback_.is_empty()) {
    InvokeDartClosure(scene_host->view_connected_callback_);
  }
}

void SceneHost::OnViewDisconnected(scenic::ResourceId id) {
  auto* scene_host = GetSceneHostForCurrentIsolate(id);

  if (scene_host && !scene_host->view_disconnected_callback_.is_empty()) {
    InvokeDartClosure(scene_host->view_disconnected_callback_);
  }
}

void SceneHost::OnViewStateChanged(scenic::ResourceId id, bool state) {
  auto* scene_host = GetSceneHostForCurrentIsolate(id);

  if (scene_host && !scene_host->view_state_changed_callback_.is_empty()) {
    InvokeDartFunction(scene_host->view_state_changed_callback_, state);
  }
}

SceneHost::SceneHost(fml::RefPtr<zircon::dart::Handle> viewHolderToken,
                     Dart_Handle viewConnectedCallback,
                     Dart_Handle viewDisconnectedCallback,
                     Dart_Handle viewStateChangedCallback)
    : raster_task_runner_(
          UIDartState::Current()->GetTaskRunners().GetRasterTaskRunner()),
      handle_(viewHolderToken->handle()) {
  auto dart_state = UIDartState::Current();
  isolate_service_id_ = Dart_IsolateServiceId(Dart_CurrentIsolate());

  // Initialize callbacks it they are non-null in Dart.
  if (!Dart_IsNull(viewConnectedCallback)) {
    view_connected_callback_.Set(dart_state, viewConnectedCallback);
  }
  if (!Dart_IsNull(viewDisconnectedCallback)) {
    view_disconnected_callback_.Set(dart_state, viewDisconnectedCallback);
  }
  if (!Dart_IsNull(viewStateChangedCallback)) {
    view_state_changed_callback_.Set(dart_state, viewStateChangedCallback);
  }

  // Keep the |SceneHost| alive with an extra ref; the logic in the destructor
  // is paired with this logic and must not run until afterwards.
  //
  // Pass the raw handle to the raster thread; destroying a
  // |zircon::dart::Handle| on that thread can cause a race condition.
  raster_task_runner_->PostTask(
      [keep_this_alive = Ref(this), view_id = viewHolderToken->ReleaseHandle(),
       ui_task_runner =
           UIDartState::Current()->GetTaskRunners().GetUITaskRunner()]() {
        auto* scene_update_context = SceneUpdateContext::GetCurrent();
        FML_DCHECK(scene_update_context);

        // Create the View, then inform the originating |SceneHost| of its ID
        // for event processing purposes.
        auto resource_id = scene_update_context->CreateView(view_id);
        ui_task_runner->PostTask(
            [keep_this_alive = std::move(keep_this_alive), resource_id]() {
              keep_this_alive->resource_id_ = resource_id;
              scene_host_bindings.emplace(std::make_pair(
                  SceneHostBindingKey(keep_this_alive->resource_id_,
                                      keep_this_alive->isolate_service_id_),
                  keep_this_alive.get()));
            });
      });
}

SceneHost::~SceneHost() {
  FML_DCHECK(resource_id_ != 0);

  scene_host_bindings.erase(
      SceneHostBindingKey(resource_id_, isolate_service_id_));

  raster_task_runner_->PostTask([view_id = handle_]() {
    auto* scene_update_context = SceneUpdateContext::GetCurrent();
    FML_DCHECK(scene_update_context);

    scene_update_context->DestroyView(view_id);
  });
}

void SceneHost::dispose() {
  ClearDartWrapper();
}

void SceneHost::setProperties(double width,
                              double height,
                              double insetTop,
                              double insetRight,
                              double insetBottom,
                              double insetLeft,
                              bool focusable) {
  raster_task_runner_->PostTask(
      [view_id = handle_, width, height, focusable]() {
        auto* scene_update_context = SceneUpdateContext::GetCurrent();
        FML_DCHECK(scene_update_context);

        scene_update_context->SetViewProperties(
            view_id, std::nullopt, SkSize::Make(width, height), std::nullopt,
            std::nullopt, focusable);
      });
}

}  // namespace flutter
