// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define RAPIDJSON_HAS_STDSTRING 1

#include "flutter/runtime/service_protocol.h"

#include <string.h>

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "lib/fxl/synchronization/waitable_event.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "third_party/dart/runtime/include/dart_tools_api.h"

namespace blink {

const fxl::StringView ServiceProtocol::kScreenshotExtensionName =
    "_flutter.screenshot";
const fxl::StringView ServiceProtocol::kScreenshotSkpExtensionName =
    "_flutter.screenshotSkp";
const fxl::StringView ServiceProtocol::kRunInViewExtensionName =
    "_flutter.runInView";
const fxl::StringView ServiceProtocol::kFlushUIThreadTasksExtensionName =
    "_flutter.flushUIThreadTasks";
const fxl::StringView ServiceProtocol::kSetAssetBundlePathExtensionName =
    "_flutter.setAssetBundlePath";

static constexpr fxl::StringView kViewIdPrefx = "_flutterView/";
static constexpr fxl::StringView kListViewsExtensionName = "_flutter.listViews";

ServiceProtocol::ServiceProtocol()
    : endpoints_({
          // Private
          kListViewsExtensionName,

          // Public
          kScreenshotExtensionName,
          kScreenshotSkpExtensionName,
          kRunInViewExtensionName,
          kFlushUIThreadTasksExtensionName,
          kSetAssetBundlePathExtensionName,
      }) {}

ServiceProtocol::~ServiceProtocol() {
  ToggleHooks(false);
}

void ServiceProtocol::AddHandler(Handler* handler) {
  std::lock_guard<std::mutex> lock(handlers_mutex_);
  handlers_.emplace(handler);
}

void ServiceProtocol::RemoveHandler(Handler* handler) {
  std::lock_guard<std::mutex> lock(handlers_mutex_);
  handlers_.erase(handler);
}

void ServiceProtocol::ToggleHooks(bool set) {
  for (const auto& endpoint : endpoints_) {
    Dart_RegisterRootServiceRequestCallback(
        endpoint.data(),                  // method
        &ServiceProtocol::HandleMessage,  // callback
        set ? this : nullptr              // user data
    );
  }
}

static void WriteServerErrorResponse(rapidjson::Document& document,
                                     const char* message) {
  document.SetObject();
  document.AddMember("code", -32000, document.GetAllocator());
  rapidjson::Value message_value;
  message_value.SetString(message, document.GetAllocator());
  document.AddMember("message", message_value, document.GetAllocator());
}

bool ServiceProtocol::HandleMessage(const char* method,
                                    const char** param_keys,
                                    const char** param_values,
                                    intptr_t num_params,
                                    void* user_data,
                                    const char** json_object) {
  Handler::ServiceProtocolMap params;
  for (intptr_t i = 0; i < num_params; i++) {
    params[fxl::StringView{param_keys[i]}] = fxl::StringView{param_values[i]};
  }

#ifndef NDEBUG
  FXL_DLOG(INFO) << "Service protcol method: " << method;
  FXL_DLOG(INFO) << "Arguments: " << params.size();
  for (intptr_t i = 0; i < num_params; i++) {
    FXL_DLOG(INFO) << "  " << i + 1 << ": " << param_keys[i] << " = "
                   << param_values[i];
  }
#endif  // NDEBUG

  rapidjson::Document document;
  bool result = HandleMessage(fxl::StringView{method},                   //
                              params,                                    //
                              static_cast<ServiceProtocol*>(user_data),  //
                              document                                   //
  );
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  document.Accept(writer);
  *json_object = strdup(buffer.GetString());

#ifndef NDEBUG
  FXL_DLOG(INFO) << "Response: " << *json_object;
  FXL_DLOG(INFO) << "RPC Result: " << result;
#endif  // NDEBUG

  return result;
}

bool ServiceProtocol::HandleMessage(fxl::StringView method,
                                    const Handler::ServiceProtocolMap& params,
                                    ServiceProtocol* service_protocol,
                                    rapidjson::Document& response) {
  if (service_protocol == nullptr) {
    WriteServerErrorResponse(response, "Service protocol unavailable.");
    return false;
  }

  return service_protocol->HandleMessage(method, params, response);
}

FXL_WARN_UNUSED_RESULT
static bool HandleMessageOnHandler(
    ServiceProtocol::Handler* handler,
    fxl::StringView method,
    const ServiceProtocol::Handler::ServiceProtocolMap& params,
    rapidjson::Document& document) {
  FXL_DCHECK(handler);
  fxl::AutoResetWaitableEvent latch;
  bool result = false;
  fml::TaskRunner::RunNowOrPostTask(
      handler->GetServiceProtocolHandlerTaskRunner(method),
      [&latch,    //
       &result,   //
       &handler,  //
       &method,   //
       &params,   //
       &document  //
  ]() {
        result =
            handler->HandleServiceProtocolMessage(method, params, document);
        latch.Signal();
      });
  latch.Wait();
  return result;
}

bool ServiceProtocol::HandleMessage(fxl::StringView method,
                                    const Handler::ServiceProtocolMap& params,
                                    rapidjson::Document& response) const {
  if (method == kListViewsExtensionName) {
    // So far, this is the only built-in method that does not forward to the
    // dynamic set of handlers.
    return HandleListViewsMethod(response);
  }

  std::lock_guard<std::mutex> lock(handlers_mutex_);

  if (handlers_.size() == 0) {
    WriteServerErrorResponse(response,
                             "There are no running service protocol handlers.");
    return false;
  }

  // Find the handler by its "viewId" in the params.
  auto view_id_param_found = params.find(fxl::StringView{"viewId"});
  if (view_id_param_found != params.end()) {
    auto handler = reinterpret_cast<Handler*>(std::stoull(
        view_id_param_found->second.data() + kViewIdPrefx.size(), nullptr, 16));
    auto handler_found = handlers_.find(handler);
    if (handler_found != handlers_.end()) {
      return HandleMessageOnHandler(handler, method, params, response);
    }
  }

  // Handle legacy calls that do not specify a handler in their args.
  // TODO(chinmaygarde): Deprecate these calls in the tools and remove these
  // fallbacks.
  if (method == kScreenshotExtensionName ||
      method == kScreenshotSkpExtensionName) {
    return HandleMessageOnHandler(*handlers_.begin(), method, params, response);
  }

  WriteServerErrorResponse(
      response,
      "Service protocol could not handle or find a handler for the "
      "requested method.");
  return false;
}

static std::string CreateFlutterViewID(intptr_t handler) {
  std::stringstream stream;
  stream << kViewIdPrefx << "0x" << std::hex << handler;
  return stream.str();
}

static std::string CreateIsolateID(int64_t isolate) {
  std::stringstream stream;
  stream << "isolates/" << isolate;
  return stream.str();
}

void ServiceProtocol::Handler::Description::Write(
    Handler* handler,
    rapidjson::Value& view,
    rapidjson::MemoryPoolAllocator<>& allocator) const {
  view.SetObject();
  view.AddMember("type", "FlutterView", allocator);
  view.AddMember("id", CreateFlutterViewID(reinterpret_cast<intptr_t>(handler)),
                 allocator);
  if (isolate_port != 0) {
    rapidjson::Value isolate(rapidjson::Type::kObjectType);
    {
      isolate.AddMember("type", "@Isolate", allocator);
      isolate.AddMember("fixedId", true, allocator);
      isolate.AddMember("id", CreateIsolateID(isolate_port), allocator);
      isolate.AddMember("name", isolate_name, allocator);
      isolate.AddMember("number", isolate_port, allocator);
    }
    view.AddMember("isolate", isolate, allocator);
  }
}

bool ServiceProtocol::HandleListViewsMethod(
    rapidjson::Document& response) const {
  // Collect handler descriptions on their respective task runners.
  std::lock_guard<std::mutex> lock(handlers_mutex_);
  std::vector<std::pair<intptr_t, Handler::Description>> descriptions;
  for (const auto& handler : handlers_) {
    fxl::AutoResetWaitableEvent latch;
    Handler::Description description;

    fml::TaskRunner::RunNowOrPostTask(
        handler->GetServiceProtocolHandlerTaskRunner(
            kListViewsExtensionName),  // task runner
        [&latch,                       //
         &description,                 //
         &handler                      //
    ]() {
          description = handler->GetServiceProtocolDescription();
          latch.Signal();
        });
    latch.Wait();
    descriptions.emplace_back(std::make_pair<intptr_t, Handler::Description>(
        reinterpret_cast<intptr_t>(handler), std::move(description)));
  }

  auto& allocator = response.GetAllocator();

  // Construct the response objects.
  response.SetObject();
  response.AddMember("type", "FlutterViewList", allocator);

  rapidjson::Value viewsList(rapidjson::Type::kArrayType);
  for (const auto& description : descriptions) {
    rapidjson::Value view(rapidjson::Type::kObjectType);
    description.second.Write(reinterpret_cast<Handler*>(description.first),
                             view, allocator);
    viewsList.PushBack(view, allocator);
  }

  response.AddMember("views", viewsList, allocator);

  return true;
}

}  // namespace blink
