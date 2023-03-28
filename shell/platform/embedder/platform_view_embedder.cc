// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/platform_view_embedder.h"

#include <utility>

#include "flutter/fml/make_copyable.h"

static uint64_t platform_message_counter = 1;

namespace flutter {

class PlatformViewEmbedder::EmbedderPlatformMessageHandler
    : public PlatformMessageHandler {
 public:
  EmbedderPlatformMessageHandler(
      fml::WeakPtr<PlatformView> parent,
      fml::RefPtr<fml::TaskRunner> platform_task_runner)
      : parent_(std::move(parent)),
        platform_task_runner_(std::move(platform_task_runner)) {}

  virtual void HandlePlatformMessage(std::unique_ptr<PlatformMessage> message) {
    if (!DoesHandlePlatformMessageOnPlatformThread()) {
      parent_->HandlePlatformMessage(std::move(message));
      return;
    }

    // platform_task_runner_->PostTask(fml::MakeCopyable(
    //     [parent = parent_, message = std::move(message)]() mutable {
    //       if (parent) {
    //         parent->HandlePlatformMessage(std::move(message));
    //       } else {
    //         FML_DLOG(WARNING) << "Deleted engine dropping message on channel "
    //                           << message->channel();
    //       }
    //     }));
  }

  virtual bool DoesHandlePlatformMessageOnPlatformThread() const {
    return false;
  }

  virtual void InvokePlatformMessageResponseCallback(
      int response_id,
      std::unique_ptr<fml::Mapping> mapping) {}
  virtual void InvokePlatformMessageEmptyResponseCallback(int response_id) {}

 private:
  fml::WeakPtr<PlatformView> parent_;
  fml::RefPtr<fml::TaskRunner> platform_task_runner_;
};

PlatformViewEmbedder::PlatformViewEmbedder(
    PlatformView::Delegate& delegate,
    const flutter::TaskRunners& task_runners,
    const EmbedderSurfaceSoftware::SoftwareDispatchTable&
        software_dispatch_table,
    PlatformDispatchTable platform_dispatch_table,
    std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : PlatformView(delegate, task_runners),
      external_view_embedder_(std::move(external_view_embedder)),
      embedder_surface_(
          std::make_unique<EmbedderSurfaceSoftware>(software_dispatch_table,
                                                    external_view_embedder_)),
      platform_message_handler_(new EmbedderPlatformMessageHandler(
          GetWeakPtr(),
          task_runners.GetPlatformTaskRunner())),
      platform_dispatch_table_(std::move(platform_dispatch_table)) {}

#ifdef SHELL_ENABLE_GL
PlatformViewEmbedder::PlatformViewEmbedder(
    PlatformView::Delegate& delegate,
    const flutter::TaskRunners& task_runners,
    const EmbedderSurfaceGL::GLDispatchTable& gl_dispatch_table,
    bool fbo_reset_after_present,
    PlatformDispatchTable platform_dispatch_table,
    std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : PlatformView(delegate, task_runners),
      external_view_embedder_(std::move(external_view_embedder)),
      embedder_surface_(
          std::make_unique<EmbedderSurfaceGL>(gl_dispatch_table,
                                              fbo_reset_after_present,
                                              external_view_embedder_)),
      platform_message_handler_(new EmbedderPlatformMessageHandler(
          GetWeakPtr(),
          task_runners.GetPlatformTaskRunner())),
      platform_dispatch_table_(std::move(platform_dispatch_table)) {}
#endif

#ifdef SHELL_ENABLE_METAL
PlatformViewEmbedder::PlatformViewEmbedder(
    PlatformView::Delegate& delegate,
    const flutter::TaskRunners& task_runners,
    std::unique_ptr<EmbedderSurfaceMetal> embedder_surface,
    PlatformDispatchTable platform_dispatch_table,
    std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : PlatformView(delegate, task_runners),
      external_view_embedder_(std::move(external_view_embedder)),
      embedder_surface_(std::move(embedder_surface)),
      platform_message_handler_(new EmbedderPlatformMessageHandler(
          GetWeakPtr(),
          task_runners.GetPlatformTaskRunner())),
      platform_dispatch_table_(std::move(platform_dispatch_table)) {}
#endif

#ifdef SHELL_ENABLE_VULKAN
PlatformViewEmbedder::PlatformViewEmbedder(
    PlatformView::Delegate& delegate,
    const flutter::TaskRunners& task_runners,
    std::unique_ptr<EmbedderSurfaceVulkan> embedder_surface,
    PlatformDispatchTable platform_dispatch_table,
    std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : PlatformView(delegate, task_runners),
      external_view_embedder_(std::move(external_view_embedder)),
      embedder_surface_(std::move(embedder_surface)),
      platform_message_handler_(new EmbedderPlatformMessageHandler(
          GetWeakPtr(),
          task_runners.GetPlatformTaskRunner())),
      platform_dispatch_table_(std::move(platform_dispatch_table)) {}
#endif

PlatformViewEmbedder::~PlatformViewEmbedder() = default;

void PlatformViewEmbedder::UpdateSemantics(
    flutter::SemanticsNodeUpdates update,
    flutter::CustomAccessibilityActionUpdates actions) {
  if (platform_dispatch_table_.update_semantics_callback != nullptr) {
    platform_dispatch_table_.update_semantics_callback(std::move(update),
                                                       std::move(actions));
  }
}

  // struct HandlerInfo {
  //   std::unique_ptr<flutter::PlatformMessageTaskQueue> task_queue;
  //   FlutterEmbedderMessageHandler handler;
  //   void* user_data;
  //   int64_t connection;
  // };

void PlatformViewEmbedder::HandlePlatformMessage(
    std::unique_ptr<flutter::PlatformMessage> message) {
  if (!message) {
    return;
  }

  fml::RefPtr<flutter::PlatformMessageResponse> completer = message->response();
  HandlerInfo handler_info;
  {
    std::lock_guard lock(message_handlers_mutex_);
    auto it = message_handlers_.find(message->channel());
    if (it != message_handlers_.end()) {
      handler_info = it->second;
    }
  }
  if (handler_info.handler) {
    FlutterEmbedderMessageHandler handler = handler_info.handler;
    const uint8_t* data = nullptr;
    size_t size = 0;
    if (message->hasData()) {
      data = message->data().GetMapping();
      size = message->data().GetSize();
    }

    uint64_t platform_message_id = platform_message_counter++;
    TRACE_EVENT_ASYNC_BEGIN1("flutter", "PlatformChannel ScheduleHandler", platform_message_id,
                             "channel", message->channel().c_str());
    fml::closure run_handler = []() {};
      // handler(data, size, handler_info.user_data, nullptr);
      // handler(data, size, handler_info.user_data, [&completer, &platform_message_id](const uint8_t* data,
      //                               size_t size,
      //                               void* user_data) {
        // TRACE_EVENT_ASYNC_END0("flutter", "PlatformChannel ScheduleHandler", platform_message_id);
        // // Called from any thread.
        // if (completer) {
        //   if (size > 0) {
        //     completer->Complete(std::make_unique<fml::NonOwnedMapping>(data, size));
        //   } else {
        //     completer->CompleteEmpty();
        //   }
        // }
      // });
    // };

    if (handler_info.task_queue.get()) {
      handler_info.task_queue.get()->CallDispatch(run_handler);
    } else {
      task_runners_.GetPlatformTaskRunner()->PostTask(fml::MakeCopyable(
        [&run_handler]() mutable {
          run_handler();
        }));
    }
  }
  else {
    if (completer) {
      completer->CompleteEmpty();
    }
  }




  // if (platform_dispatch_table_.platform_message_response_callback == nullptr) {
  //   if (message->response()) {
  //     message->response()->CompleteEmpty();
  //   }
  //   return;
  // }

  // platform_dispatch_table_.platform_message_response_callback(
  //     std::move(message));
}

int64_t PlatformViewEmbedder::SetMessageHandlerOnQueue(
    const char* channel,
    FlutterEmbedderMessageHandler handler,
    std::shared_ptr<flutter::PlatformMessageTaskQueue> task_queue,
    void* user_data) {
  FML_CHECK(task_runners_.GetPlatformTaskRunner()->RunsTasksOnCurrentThread());
  std::lock_guard lock(message_handlers_mutex_);
  message_handlers_.erase(channel);
  if (handler) {
    message_handlers_[channel] = {
        .task_queue = task_queue,
        .handler = handler,
        .user_data = std::move(user_data),
        .connection = ++current_connection,
    };
  }
  return current_connection;
}

// bool PlatformViewEmbedder::DoesHandlePlatformMessageOnPlatformThread () {
//   if
//   (platform_dispatch_table_.does_handle_platform_message_on_platform_thread
//   == nullptr) {
//     return true;
//   }
//   return
//   platform_dispatch_table_.does_handle_platform_message_on_platform_thread();
// }

// |PlatformView|
std::unique_ptr<Surface> PlatformViewEmbedder::CreateRenderingSurface() {
  if (embedder_surface_ == nullptr) {
    FML_LOG(ERROR) << "Embedder surface was null.";
    return nullptr;
  }
  return embedder_surface_->CreateGPUSurface();
}

// |PlatformView|
std::shared_ptr<ExternalViewEmbedder>
PlatformViewEmbedder::CreateExternalViewEmbedder() {
  return external_view_embedder_;
}

// |PlatformView|
sk_sp<GrDirectContext> PlatformViewEmbedder::CreateResourceContext() const {
  if (embedder_surface_ == nullptr) {
    FML_LOG(ERROR) << "Embedder surface was null.";
    return nullptr;
  }
  return embedder_surface_->CreateResourceContext();
}

// |PlatformView|
std::unique_ptr<VsyncWaiter> PlatformViewEmbedder::CreateVSyncWaiter() {
  if (!platform_dispatch_table_.vsync_callback) {
    // Superclass implementation creates a timer based fallback.
    return PlatformView::CreateVSyncWaiter();
  }

  return std::make_unique<VsyncWaiterEmbedder>(
      platform_dispatch_table_.vsync_callback, task_runners_);
}

// |PlatformView|
std::unique_ptr<std::vector<std::string>>
PlatformViewEmbedder::ComputePlatformResolvedLocales(
    const std::vector<std::string>& supported_locale_data) {
  if (platform_dispatch_table_.compute_platform_resolved_locale_callback !=
      nullptr) {
    return platform_dispatch_table_.compute_platform_resolved_locale_callback(
        supported_locale_data);
  }
  std::unique_ptr<std::vector<std::string>> out =
      std::make_unique<std::vector<std::string>>();
  return out;
}

// |PlatformView|
void PlatformViewEmbedder::OnPreEngineRestart() const {
  if (platform_dispatch_table_.on_pre_engine_restart_callback != nullptr) {
    platform_dispatch_table_.on_pre_engine_restart_callback();
  }
}

std::shared_ptr<PlatformMessageHandler>
PlatformViewEmbedder::GetPlatformMessageHandler() const {
  return platform_message_handler_;
}

PlatformMessageTaskQueue::PlatformMessageTaskQueue(
    const TaskQueueCallbackEmbedder task_queue_callback, void* user_data)
    : task_queue_callback_(task_queue_callback),
      user_data_(user_data) {}

void PlatformMessageTaskQueue::CallDispatch(fml::closure callback) {
  task_queue_callback_(callback, user_data_);
}

}  // namespace flutter
