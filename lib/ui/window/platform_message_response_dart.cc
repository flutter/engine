// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/platform_message_response_dart.h"

#include <utility>

#include "flutter/common/task_runners.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/lib/ui/window/window.h"
#include "third_party/tonic/dart_state.h"
#include "third_party/tonic/logging/dart_invoke.h"

namespace flutter {

namespace {

// Avoid copying the contents of messages beyond a certain size.
const int kMessageCopyThreshold = 1000;

class DataWrapper {
 public:
  DataWrapper(void* data) { data_ = data; }

  ~DataWrapper() { free(data_); }

 private:
  void* data_;
};

void MessageDataFinalizer(void* isolate_callback_data,
                          Dart_WeakPersistentHandle handle,
                          void* peer) {
  DataWrapper* data = reinterpret_cast<DataWrapper*>(peer);
  delete data;
}

Dart_Handle WrapByteData(std::unique_ptr<fml::Mapping> mapping) {
  size_t size = mapping->GetSize();
  if (size < kMessageCopyThreshold) {
    std::vector<uint8_t> data(size);
    memcpy(data.data(), mapping->GetMapping(), size);
    return ToByteData(data);
  } else {
    void* data = malloc(size);
    memset(data, 0, size);
    memcpy(data, mapping->GetMapping(), size);
    DataWrapper* wrapper = new DataWrapper(data);
    return Dart_NewExternalTypedDataWithFinalizer(Dart_TypedData_kByteData,
                                                  data, size, wrapper, size,
                                                  MessageDataFinalizer);
  }
}

}  // anonymous namespace

PlatformMessageResponseDart::PlatformMessageResponseDart(
    tonic::DartPersistentValue callback,
    fml::RefPtr<fml::TaskRunner> ui_task_runner)
    : callback_(std::move(callback)),
      ui_task_runner_(std::move(ui_task_runner)) {}

PlatformMessageResponseDart::~PlatformMessageResponseDart() {
  if (!callback_.is_empty()) {
    ui_task_runner_->PostTask(fml::MakeCopyable(
        [callback = std::move(callback_)]() mutable { callback.Clear(); }));
  }
}

void PlatformMessageResponseDart::Complete(std::unique_ptr<fml::Mapping> data) {
  if (callback_.is_empty())
    return;
  FML_DCHECK(!is_complete_);
  is_complete_ = true;
  ui_task_runner_->PostTask(fml::MakeCopyable(
      [callback = std::move(callback_), data = std::move(data)]() mutable {
        std::shared_ptr<tonic::DartState> dart_state =
            callback.dart_state().lock();
        if (!dart_state)
          return;
        tonic::DartState::Scope scope(dart_state);

        Dart_Handle byte_buffer = WrapByteData(std::move(data));
        tonic::DartInvoke(callback.Release(), {byte_buffer});
      }));
}

void PlatformMessageResponseDart::CompleteEmpty() {
  if (callback_.is_empty())
    return;
  FML_DCHECK(!is_complete_);
  is_complete_ = true;
  ui_task_runner_->PostTask(
      fml::MakeCopyable([callback = std::move(callback_)]() mutable {
        std::shared_ptr<tonic::DartState> dart_state =
            callback.dart_state().lock();
        if (!dart_state)
          return;
        tonic::DartState::Scope scope(dart_state);
        tonic::DartInvoke(callback.Release(), {Dart_Null()});
      }));
}

}  // namespace flutter
