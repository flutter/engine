// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/ui_dart_state.h"

#include "flutter/fml/message_loop.h"
#include "flutter/lib/ui/window/window.h"
#include "flutter/sky/engine/platform/fonts/FontSelector.h"
#include "lib/tonic/converter/dart_converter.h"

using tonic::ToDart;

namespace blink {

UIDartState::UIDartState(TaskRunners task_runners,
                         fml::WeakPtr<GrContext> resource_context,
                         fxl::RefPtr<flow::SkiaUnrefQueue> skia_unref_queue,
                         std::string advisory_script_uri,
                         std::string advisory_script_entrypoint)
    : task_runners_(std::move(task_runners)),
      resource_context_(std::move(resource_context)),
      advisory_script_uri_(std::move(advisory_script_uri)),
      advisory_script_entrypoint_(std::move(advisory_script_entrypoint)),
      skia_unref_queue_(std::move(skia_unref_queue)),
      weak_factory_(this) {
  AddOrRemoveTaskObserver(true /* add */);
}

UIDartState::~UIDartState() {
  AddOrRemoveTaskObserver(false /* remove */);
}

const std::string& UIDartState::GetAdvisoryScriptURI() const {
  return advisory_script_uri_;
}

const std::string& UIDartState::GetAdvisoryScriptEntrypoint() const {
  return advisory_script_entrypoint_;
}

void UIDartState::DidSetIsolate() {
  main_port_ = Dart_GetMainPortId();
  std::ostringstream debug_name;
  // main.dart$main-1234
  debug_name << advisory_script_uri_ << "$" << advisory_script_entrypoint_
             << "-" << main_port_;
  debug_name_ = debug_name.str();
}

UIDartState* UIDartState::Current() {
  return static_cast<UIDartState*>(DartState::Current());
}

void UIDartState::set_font_selector(PassRefPtr<FontSelector> selector) {
  font_selector_ = selector;
}

PassRefPtr<FontSelector> UIDartState::font_selector() {
  return font_selector_;
}

void UIDartState::SetWindow(std::unique_ptr<Window> window) {
  window_ = std::move(window);
}

const TaskRunners& UIDartState::GetTaskRunners() const {
  return task_runners_;
}

fxl::RefPtr<flow::SkiaUnrefQueue> UIDartState::GetSkiaUnrefQueue() const {
  return skia_unref_queue_;
}

void UIDartState::ScheduleMicrotask(Dart_Handle closure) {
  if (tonic::LogIfError(closure) || !Dart_IsClosure(closure)) {
    return;
  }

  microtask_queue_.ScheduleMicrotask(closure);
}

void UIDartState::FlushMicrotasksNow() {
  microtask_queue_.RunMicrotasks();
}

void UIDartState::AddOrRemoveTaskObserver(bool add) {
  auto task_runner = task_runners_.GetUITaskRunner();
  if (!task_runner) {
    // This may happen in case the isolate has no thread affinity (for example,
    // the service isolate).
    return;
  }
  FXL_DCHECK(task_runner->RunsTasksOnCurrentThread());
  auto& loop = fml::MessageLoop::GetCurrent();
  if (add) {
    loop.AddTaskObserver(this);
  } else {
    loop.RemoveTaskObserver(this);
  }
}

// |fml::TaskObserver|
void UIDartState::DidProcessTask() {
  FlushMicrotasksNow();
}

fml::WeakPtr<GrContext> UIDartState::GetResourceContext() const {
  return resource_context_;
}

}  // namespace blink
