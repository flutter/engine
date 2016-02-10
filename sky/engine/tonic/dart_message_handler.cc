// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/tonic/dart_message_handler.h"

#include "base/bind.h"
#include "base/logging.h"
#include "dart/runtime/include/dart_api.h"
#include "dart/runtime/include/dart_native_api.h"
#include "dart/runtime/include/dart_tools_api.h"
#include "sky/engine/tonic/dart_error.h"
#include "sky/engine/tonic/dart_state.h"

namespace blink {

DartMessageHandler::DartMessageHandler()
    : handled_first_message_(false),
      task_runner_(nullptr) {
}

DartMessageHandler::~DartMessageHandler() {
  task_runner_ = nullptr;
}

void DartMessageHandler::Initialize(
    const scoped_refptr<base::SingleThreadTaskRunner>& runner) {
  // Only can be called once.
  CHECK(!task_runner_);
  task_runner_ = runner;
  CHECK(task_runner_);
  Dart_SetMessageNotifyCallback(MessageNotifyCallback);
}

void DartMessageHandler::OnMessage(DartState* dart_state) {
  auto task_runner = dart_state->message_handler().task_runner();

  // Schedule a task to run on the message loop thread.
  task_runner->PostTask(FROM_HERE,
                        base::Bind(&HandleMessage, dart_state->GetWeakPtr()));
}

void DartMessageHandler::OnHandleMessage(DartState* dart_state) {
  DartIsolateScope scope(dart_state->isolate());
  DartApiScope dart_api_scope;

  // On the first message, check if we should pause on isolate start.
  if (!handled_first_message()) {
    set_handled_first_message(true);
    if (Dart_ShouldPauseOnStart()) {
      // Mark that we are paused on isolate start.
      Dart_SetPausedOnStart(true);
    }
  }

  if (Dart_IsPausedOnStart()) {
    // We are paused on isolate start. Only handle service messages until we are
    // requested to resume.
    if (Dart_HasServiceMessages()) {
      bool resume = Dart_HandleServiceMessages();
      if (!resume) {
        return;
      }
      Dart_SetPausedOnStart(false);
      // We've resumed, handle *all* normal messages that are in the queue.
      LogIfError(Dart_HandleMessages());
    }
  } else if (Dart_IsPausedOnExit()) {
    // We are paused on isolate exit. Only handle service messages until we are
    // requested to resume.
    if (Dart_HasServiceMessages()) {
      bool resume = Dart_HandleServiceMessages();
      if (!resume) {
        return;
      }
      Dart_SetPausedOnExit(false);
    }
  } else {
    // We are processing messages normally.
    LogIfError(Dart_HandleMessage());
  }

  if (!Dart_HasLivePorts()) {
    // The isolate has no live ports and would like to exit.
    if (Dart_ShouldPauseOnExit()) {
      // Mark that we are paused on exit.
      Dart_SetPausedOnExit(true);
    } else {
      // Quit.
      base::MessageLoop::current()->QuitWhenIdle();
    }
  }
}

void DartMessageHandler::MessageNotifyCallback(Dart_Isolate dest_isolate) {
  auto dart_state = DartState::From(dest_isolate);
  CHECK(dart_state);
  dart_state->message_handler().OnMessage(dart_state);
}

void DartMessageHandler::HandleMessage(
    base::WeakPtr<DartState> dart_state) {
  if (!dart_state)
    return;
  dart_state->message_handler().OnHandleMessage(dart_state.get());
}

}  // namespace blink
