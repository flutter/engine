// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/service_protocol_handler.h"

#include "flutter/assets/directory_asset_bundle.h"
#include "flutter/fml/paths.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/shell/common/isolate_configuration.h"
#include "flutter/shell/common/run_configuration.h"

namespace shell {

std::shared_ptr<ServiceProtocolHandler>
ServiceProtocolHandler::CreateAttachedToShell(
    blink::TaskRunners task_runners,
    fml::WeakPtr<Rasterizer> rasterizer,
    fml::WeakPtr<Engine> engine,
    fml::WeakPtr<PlatformView> platform_view) {
  auto handler = std::shared_ptr<ServiceProtocolHandler>{
      new ServiceProtocolHandler(std::move(task_runners), std::move(rasterizer),
                                 std::move(engine), std::move(platform_view))};
  handler->Attach();
  return handler;
}

ServiceProtocolHandler::ServiceProtocolHandler(
    blink::TaskRunners p_task_runners,
    fml::WeakPtr<Rasterizer> p_rasterizer,
    fml::WeakPtr<Engine> p_engine,
    fml::WeakPtr<PlatformView> p_platform_view)
    : task_runners_(std::move(p_task_runners)),
      rasterizer_(std::move(p_rasterizer)),
      engine_(std::move(p_engine)),
      platform_view_(std::move(p_platform_view)),
      weak_factory_(this) {
  service_protocol_handlers_[blink::ServiceProtocol::kScreenshotExtensionName
                                 .ToString()] = {
      task_runners_.GetGPUTaskRunner(),
      std::bind(&ServiceProtocolHandler::OnServiceProtocolScreenshot, this,
                std::placeholders::_1, std::placeholders::_2)};
  service_protocol_handlers_[blink::ServiceProtocol::kScreenshotSkpExtensionName
                                 .ToString()] = {
      task_runners_.GetGPUTaskRunner(),
      std::bind(&ServiceProtocolHandler::OnServiceProtocolScreenshotSKP, this,
                std::placeholders::_1, std::placeholders::_2)};
  service_protocol_handlers_[blink::ServiceProtocol::kRunInViewExtensionName
                                 .ToString()] = {
      task_runners_.GetUITaskRunner(),
      std::bind(&ServiceProtocolHandler::OnServiceProtocolRunInView, this,
                std::placeholders::_1, std::placeholders::_2)};
  service_protocol_handlers_
      [blink::ServiceProtocol::kFlushUIThreadTasksExtensionName.ToString()] = {
          task_runners_.GetUITaskRunner(),
          std::bind(
              &ServiceProtocolHandler::OnServiceProtocolFlushUIThreadTasks,
              this, std::placeholders::_1, std::placeholders::_2)};
  service_protocol_handlers_
      [blink::ServiceProtocol::kSetAssetBundlePathExtensionName.ToString()] = {
          task_runners_.GetUITaskRunner(),
          std::bind(
              &ServiceProtocolHandler::OnServiceProtocolSetAssetBundlePath,
              this, std::placeholders::_1, std::placeholders::_2)};
}

ServiceProtocolHandler::~ServiceProtocolHandler() = default;

void ServiceProtocolHandler::Attach() {
  if (auto vm = blink::DartVM::ForProcessIfInitialized()) {
    vm->GetServiceProtocol().AddHandler(shared_from_this());
  }
}

void ServiceProtocolHandler::DetachFromShell() {
  rasterizer_ = {};
  engine_ = {};
  platform_view_ = {};
  if (auto vm = blink::DartVM::ForProcessIfInitialized()) {
    vm->GetServiceProtocol().RemoveHandler(shared_from_this());
  }
}

// |blink::ServiceProtocol::Handler|
fml::RefPtr<fml::TaskRunner>
ServiceProtocolHandler::GetServiceProtocolHandlerTaskRunner(
    fml::StringView method) const {
  auto found = service_protocol_handlers_.find(method.ToString());
  if (found != service_protocol_handlers_.end()) {
    return found->second.first;
  }
  return task_runners_.GetUITaskRunner();
}

// |blink::ServiceProtocol::Handler|
bool ServiceProtocolHandler::HandleServiceProtocolMessage(
    fml::StringView method,  // one if the extension names specified above.
    const ServiceProtocolMap& params,
    rapidjson::Document& response) {
  auto found = service_protocol_handlers_.find(method.ToString());
  if (found != service_protocol_handlers_.end()) {
    return found->second.second(params, response);
  }
  return false;
}

// |blink::ServiceProtocol::Handler|
blink::ServiceProtocol::Handler::Description
ServiceProtocolHandler::GetServiceProtocolDescription() const {
  if (!engine_) {
    return {};
  }
  return {
      engine_->GetUIIsolateMainPort(),
      engine_->GetUIIsolateName(),
  };
}

static void ServiceProtocolParameterError(rapidjson::Document& response,
                                          std::string error_details) {
  auto& allocator = response.GetAllocator();
  response.SetObject();
  const int64_t kInvalidParams = -32602;
  response.AddMember("code", kInvalidParams, allocator);
  response.AddMember("message", "Invalid params", allocator);
  {
    rapidjson::Value details(rapidjson::kObjectType);
    details.AddMember("details", error_details, allocator);
    response.AddMember("data", details, allocator);
  }
}

static void ServiceProtocolFailureError(rapidjson::Document& response,
                                        std::string message) {
  auto& allocator = response.GetAllocator();
  response.SetObject();
  const int64_t kJsonServerError = -32000;
  response.AddMember("code", kJsonServerError, allocator);
  response.AddMember("message", message, allocator);
}

bool ServiceProtocolHandler::OnServiceProtocolScreenshot(
    const blink::ServiceProtocol::Handler::ServiceProtocolMap& params,
    rapidjson::Document& response) {
  if (!rasterizer_) {
    ServiceProtocolFailureError(response,
                                "Attempted to screenshot a dead rasterizer.");
    return false;
  }
  FML_DCHECK(task_runners_.GetGPUTaskRunner()->RunsTasksOnCurrentThread());
  auto screenshot = rasterizer_->ScreenshotLastLayerTree(
      Rasterizer::ScreenshotType::CompressedImage, true);
  if (screenshot.data) {
    response.SetObject();
    auto& allocator = response.GetAllocator();
    response.AddMember("type", "Screenshot", allocator);
    rapidjson::Value image;
    image.SetString(static_cast<const char*>(screenshot.data->data()),
                    screenshot.data->size(), allocator);
    response.AddMember("screenshot", image, allocator);
    return true;
  }
  ServiceProtocolFailureError(response, "Could not capture image screenshot.");
  return false;
}

bool ServiceProtocolHandler::OnServiceProtocolScreenshotSKP(
    const blink::ServiceProtocol::Handler::ServiceProtocolMap& params,
    rapidjson::Document& response) {
  if (!rasterizer_) {
    ServiceProtocolFailureError(response,
                                "Attempted to screenshot a dead rasterizer.");
    return false;
  }
  FML_DCHECK(task_runners_.GetGPUTaskRunner()->RunsTasksOnCurrentThread());
  auto screenshot = rasterizer_->ScreenshotLastLayerTree(
      Rasterizer::ScreenshotType::SkiaPicture, true);
  if (screenshot.data) {
    response.SetObject();
    auto& allocator = response.GetAllocator();
    response.AddMember("type", "ScreenshotSkp", allocator);
    rapidjson::Value skp;
    skp.SetString(static_cast<const char*>(screenshot.data->data()),
                  screenshot.data->size(), allocator);
    response.AddMember("skp", skp, allocator);
    return true;
  }
  ServiceProtocolFailureError(response, "Could not capture SKP screenshot.");
  return false;
}

bool ServiceProtocolHandler::OnServiceProtocolRunInView(
    const blink::ServiceProtocol::Handler::ServiceProtocolMap& params,
    rapidjson::Document& response) {
  if (!engine_) {
    ServiceProtocolFailureError(response,
                                "Attempted to restart a dead engine.");
    return false;
  }

  FML_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  if (params.count("mainScript") == 0) {
    ServiceProtocolParameterError(response,
                                  "'mainScript' parameter is missing.");
    return false;
  }

  // TODO(chinmaygarde): In case of hot-reload from .dill files, the packages
  // file is ignored. Currently, the tool is passing a junk packages file to
  // pass this check. Update the service protocol interface and remove this
  // workaround.
  if (params.count("packagesFile") == 0) {
    ServiceProtocolParameterError(response,
                                  "'packagesFile' parameter is missing.");
    return false;
  }

  if (params.count("assetDirectory") == 0) {
    ServiceProtocolParameterError(response,
                                  "'assetDirectory' parameter is missing.");
    return false;
  }

  std::string main_script_path =
      fml::paths::FromURI(params.at("mainScript").ToString());
  std::string packages_path =
      fml::paths::FromURI(params.at("packagesFile").ToString());
  std::string asset_directory_path =
      fml::paths::FromURI(params.at("assetDirectory").ToString());

  auto main_script_file_mapping =
      std::make_unique<fml::FileMapping>(fml::OpenFile(
          main_script_path.c_str(), false, fml::FilePermission::kRead));

  auto isolate_configuration = IsolateConfiguration::CreateForKernel(
      std::move(main_script_file_mapping));

  RunConfiguration configuration(std::move(isolate_configuration));

  configuration.AddAssetResolver(
      std::make_unique<blink::DirectoryAssetBundle>(fml::OpenDirectory(
          asset_directory_path.c_str(), false, fml::FilePermission::kRead)));

  auto& allocator = response.GetAllocator();
  response.SetObject();
  if (engine_->Restart(std::move(configuration))) {
    response.AddMember("type", "Success", allocator);
    auto new_description = GetServiceProtocolDescription();
    rapidjson::Value view(rapidjson::kObjectType);
    new_description.Write(this, view, allocator);
    response.AddMember("view", view, allocator);
    return true;
  } else {
    FML_DLOG(ERROR) << "Could not run configuration in engine.";
    ServiceProtocolFailureError(response,
                                "Could not run configuration in engine.");
    return false;
  }

  FML_DCHECK(false);
  return false;
}

bool ServiceProtocolHandler::OnServiceProtocolFlushUIThreadTasks(
    const blink::ServiceProtocol::Handler::ServiceProtocolMap& params,
    rapidjson::Document& response) {
  FML_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());
  // This API should not be invoked by production code.
  // It can potentially starve the service isolate if the main isolate pauses
  // at a breakpoint or is in an infinite loop.
  //
  // It should be invoked from the VM Service and and blocks it until UI thread
  // tasks are processed.
  response.SetObject();
  response.AddMember("type", "Success", response.GetAllocator());
  return true;
}

bool ServiceProtocolHandler::OnServiceProtocolSetAssetBundlePath(
    const blink::ServiceProtocol::Handler::ServiceProtocolMap& params,
    rapidjson::Document& response) {
  if (!engine_) {
    ServiceProtocolFailureError(
        response, "Attempted to set asset bundle on dead engine.");
    return false;
  }
  FML_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());

  if (params.count("assetDirectory") == 0) {
    ServiceProtocolParameterError(response,
                                  "'assetDirectory' parameter is missing.");
    return false;
  }

  auto& allocator = response.GetAllocator();
  response.SetObject();

  auto asset_manager = std::make_shared<blink::AssetManager>();

  asset_manager->PushFront(std::make_unique<blink::DirectoryAssetBundle>(
      fml::OpenDirectory(params.at("assetDirectory").ToString().c_str(), false,
                         fml::FilePermission::kRead)));

  if (engine_->UpdateAssetManager(std::move(asset_manager))) {
    response.AddMember("type", "Success", allocator);
    auto new_description = GetServiceProtocolDescription();
    rapidjson::Value view(rapidjson::kObjectType);
    new_description.Write(this, view, allocator);
    response.AddMember("view", view, allocator);
    return true;
  } else {
    FML_DLOG(ERROR) << "Could not update asset directory.";
    ServiceProtocolFailureError(response, "Could not update asset directory.");
    return false;
  }

  FML_DCHECK(false);
  return false;
}

}  // namespace shell
