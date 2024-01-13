// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/platform_message_handler_android.h"

namespace flutter {

PlatformMessageHandlerAndroid::PlatformMessageHandlerAndroid(
    const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade)
    : jni_facade_(jni_facade) {}

void PlatformMessageHandlerAndroid::InvokePlatformMessageResponseCallback(
    int response_id,
    std::unique_ptr<fml::Mapping> mapping) {
  // Called from any thread.
  if (!response_id) {
    return;
  }
  // TODO(gaaclarke): Move the jump to the ui thread here from
  // PlatformMessageResponseDart so we won't need to use a mutex anymore.
  fml::RefPtr<flutter::PlatformMessageResponse> message_response;
  {
    std::lock_guard lock(pending_responses_mutex_);
    auto it = pending_responses_.find(response_id);
    if (it == pending_responses_.end()) {
      return;
    }
    message_response = std::move(it->second);
    pending_responses_.erase(it);
  }

  message_response->Complete(std::move(mapping));
}

void PlatformMessageHandlerAndroid::InvokePlatformMessageEmptyResponseCallback(
    int response_id) {
  // Called from any thread.
  if (!response_id) {
    return;
  }
  fml::RefPtr<flutter::PlatformMessageResponse> message_response;
  {
    std::lock_guard lock(pending_responses_mutex_);
    auto it = pending_responses_.find(response_id);
    if (it == pending_responses_.end()) {
      return;
    }
    message_response = std::move(it->second);
    pending_responses_.erase(it);
  }
  message_response->CompleteEmpty();
}

// |PlatformView|
void PlatformMessageHandlerAndroid::HandlePlatformMessage(
    std::unique_ptr<flutter::PlatformMessage> message) {
  // Called from any thread.
  int response_id = next_response_id_.fetch_add(1);
  if (auto response = message->response()) {
    std::lock_guard lock(pending_responses_mutex_);
    pending_responses_[response_id] = response;
  }
  // This call can re-enter in InvokePlatformMessageXxxResponseCallback.
  jni_facade_->FlutterViewHandlePlatformMessage(std::move(message),
                                                response_id);
}

bool PlatformMessageHandlerAndroid::SendToPlatformPortCallback(
    const std::string& name,
    std::unique_ptr<fml::MallocMapping> message_data,
    int32_t response_id,
    fml::RefPtr<flutter::PlatformMessageResponse> response) {
  FML_LOG(INFO)
      << "PlatformMessageHandlerAndroid::SendToPlatformPortCallback | channel: "
      << name << " | response_id: " << response_id;

  std::lock_guard lock(pending_callback_responses_mutex_);
  auto it = active_callbacks_.find(name);
  if (it == active_callbacks_.end()) {
    FML_LOG(WARNING)
        << "PlatformMessageHandlerAndroid::AddPlatformPortCallback "
           "No isolate callbacks set for "
        << name << " channel.";
    return false;
  }

  pending_callback_responses_[response_id] = std::move(response);
  it->second->Send(response_id, name, std::move(message_data));

  return true;
}

bool PlatformMessageHandlerAndroid::SendToPlatformPortCallbackEmpty(
    const std::string& name,
    int32_t response_id,
    fml::RefPtr<flutter::PlatformMessageResponse> response) {
  FML_LOG(INFO) << "PlatformMessageHandlerAndroid::"
                   "SendToPlatformPortCallbackEmpty | channel: "
                << name << " | response_id: " << response_id;

  std::lock_guard lock(pending_callback_responses_mutex_);
  auto it = active_callbacks_.find(name);
  if (it == active_callbacks_.end()) {
    FML_LOG(WARNING)
        << "PlatformMessageHandlerAndroid::AddPlatformPortCallback "
           "No isolate callbacks set for "
        << name << " channel.";
    return false;
  }

  pending_callback_responses_[response_id] = std::move(response);
  it->second->SendEmpty();

  return false;
}

void PlatformMessageHandlerAndroid::SendToPlatformPortCallbackResponse(
    int response_id,
    std::unique_ptr<fml::Mapping> mapping) {
  if (!response_id) {
    return;
  }

  fml::RefPtr<flutter::PlatformMessageResponse> message_response;
  {
    std::lock_guard lock(pending_callback_responses_mutex_);
    auto it = pending_callback_responses_.find(response_id);
    if (it == pending_callback_responses_.end()) {
      FML_LOG(ERROR) << "PlatformMessageHandlerAndroid::"
                        "SendToPlatformPortCallbackResponse "
                     << response_id << " not found";
      return;
    }
    message_response = std::move(it->second);
    pending_callback_responses_.erase(it);
  }

  message_response->Complete(std::move(mapping));
  FML_LOG(INFO)
      << "PlatformMessageHandlerAndroid::SendToPlatformPortCallbackResponse "
      << response_id << " completed";
}

void PlatformMessageHandlerAndroid::SendToPlatformPortCallbackResponseEmpty(
    int response_id) {
  if (!response_id) {
    return;
  }

  fml::RefPtr<flutter::PlatformMessageResponse> message_response;
  {
    std::lock_guard lock(pending_callback_responses_mutex_);
    auto it = pending_callback_responses_.find(response_id);
    if (it == pending_callback_responses_.end()) {
      FML_LOG(ERROR) << "PlatformMessageHandlerAndroid::"
                        "SendToPlatformPortCallbackResponseEmpty "
                     << response_id << " not found";
      return;
    }
    message_response = std::move(it->second);
    pending_callback_responses_.erase(it);
  }

  message_response->CompleteEmpty();
  FML_LOG(INFO) << "PlatformMessageHandlerAndroid::"
                   "SendToPlatformPortCallbackResponseEmpty "
                << response_id << " completed";
}

bool PlatformMessageHandlerAndroid::AddPlatformPortCallback(
    int64_t port,
    const std::string& channel) {
  fml::RefPtr<flutter::PlatformListenerDartPort> listener =
      fml::MakeRefCounted<flutter::PlatformListenerDartPort>(port, 1);

  std::lock_guard lock(pending_callback_responses_mutex_);

  auto it = active_callbacks_.find(channel);
  if (it != active_callbacks_.end()) {
    FML_LOG(ERROR) << "PlatformMessageHandlerAndroid::AddPlatformPortCallback "
                      ": Isolate callbacks already set for "
                   << channel << " channel.";
    return false;
  }

  active_callbacks_[channel] = listener;
  FML_LOG(INFO) << "PlatformMessageHandlerAndroid::AddPlatformPortCallback "
                << channel << " added.";
  return true;
}

bool PlatformMessageHandlerAndroid::RemovePlatformPortCallback(
    const std::string& channel) {
  std::lock_guard lock(pending_callback_responses_mutex_);
  auto it = active_callbacks_.find(channel);
  if (it == active_callbacks_.end()) {
    FML_LOG(ERROR)
        << "PlatformMessageHandlerAndroid::RemovePlatformPortCallback "
           ":No isolate callbacks set for "
        << channel << " channel.";
    return false;
  }
  FML_LOG(INFO) << "PlatformMessageHandlerAndroid::RemovePlatformPortCallback "
                << channel << " removed.";
  active_callbacks_.erase(it);
  return true;
}

}  // namespace flutter
