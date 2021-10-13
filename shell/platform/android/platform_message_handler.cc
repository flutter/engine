
#include "flutter/shell/platform/android/platform_message_handler.h"

namespace flutter {

PlatformMessageHandler::PlatformMessageHandler(
    const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade)
    : jni_facade_(jni_facade) {}

void PlatformMessageHandler::InvokePlatformMessageResponseCallback(
    JNIEnv* env,
    jint response_id,
    jobject java_response_data,
    jint java_response_position) {
  // Called from any thread.
  if (!response_id) {
    return;
  }
  // TODO(gaaclarke): Move the jump to the ui thread here from
  // PlatformMessageResponseDart so we won't need to use a mutex anymore.
  std::unique_lock lock(pending_responses_mutex_);
  auto it = pending_responses_.find(response_id);
  if (it == pending_responses_.end())
    return;
  uint8_t* response_data =
      static_cast<uint8_t*>(env->GetDirectBufferAddress(java_response_data));
  FML_DCHECK(response_data != nullptr);
  std::vector<uint8_t> response = std::vector<uint8_t>(
      response_data, response_data + java_response_position);
  auto message_response = std::move(it->second);
  pending_responses_.erase(it);
  lock.unlock();
  message_response->Complete(
      std::make_unique<fml::DataMapping>(std::move(response)));
}

void PlatformMessageHandler::InvokePlatformMessageEmptyResponseCallback(
    JNIEnv* env,
    jint response_id) {
  // Called from any thread.
  if (!response_id) {
    return;
  }
  std::unique_lock lock(pending_responses_mutex_);
  auto it = pending_responses_.find(response_id);
  if (it == pending_responses_.end())
    return;
  auto message_response = std::move(it->second);
  pending_responses_.erase(it);
  lock.unlock();
  message_response->CompleteEmpty();
}

// |PlatformView|
void PlatformMessageHandler::HandlePlatformMessage(
    std::unique_ptr<flutter::PlatformMessage> message) {
  // Called from the ui thread.
  int response_id = next_response_id_++;
  if (auto response = message->response()) {
    std::lock_guard lock(pending_responses_mutex_);
    pending_responses_[response_id] = response;
  }
  // This call can re-enter in InvokePlatformMessageXxxResponseCallback.
  jni_facade_->FlutterViewHandlePlatformMessage(std::move(message),
                                                response_id);
  message = nullptr;
}

}  // namespace flutter
