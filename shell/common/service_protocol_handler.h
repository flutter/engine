// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_SERVICE_PROTOCOL_HANDLER_H_
#define FLUTTER_SHELL_COMMON_SERVICE_PROTOCOL_HANDLER_H_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "flutter/common/task_runners.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/runtime/service_protocol.h"
#include "flutter/shell/common/engine.h"
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/common/rasterizer.h"

namespace shell {

// Usually, the service protocol has a weak reference to all registered
// handlers. However, it needs to acquire a strong reference before it can
// invoke each handler. This object ensures that the service protocol sees a
// shared reference to a service protocol method handler (i.e, the shell) while
// the platform gets a unique reference to a shell. The shell owns this object
// and all access to this object happen on the platform thread.
class ServiceProtocolHandler
    : public blink::ServiceProtocol::Handler,
      public std::enable_shared_from_this<ServiceProtocolHandler> {
 public:
  static std::shared_ptr<ServiceProtocolHandler> CreateAttachedToShell(
      blink::TaskRunners task_runners,
      fml::WeakPtr<Rasterizer> rasterizer,
      fml::WeakPtr<Engine> engine,
      fml::WeakPtr<PlatformView> platform_view);

  virtual ~ServiceProtocolHandler();

  void DetachFromShell();

 private:
  using ServiceProtocolCallback = std::function<bool(
      const blink::ServiceProtocol::Handler::ServiceProtocolMap&,
      rapidjson::Document&)>;

  blink::TaskRunners task_runners_;
  fml::WeakPtr<Rasterizer> rasterizer_;
  fml::WeakPtr<Engine> engine_;
  fml::WeakPtr<PlatformView> platform_view_;
  std::unordered_map<std::string,  // method
                     std::pair<fml::RefPtr<fml::TaskRunner>,
                               ServiceProtocolCallback>  // task-runner/function
                     >
      service_protocol_handlers_;
  fml::WeakPtrFactory<ServiceProtocolHandler> weak_factory_;

  ServiceProtocolHandler(blink::TaskRunners task_runners,
                         fml::WeakPtr<Rasterizer> rasterizer,
                         fml::WeakPtr<Engine> engine,
                         fml::WeakPtr<PlatformView> platform_view);

  void Attach();

  // |blink::ServiceProtocol::Handler|
  fml::RefPtr<fml::TaskRunner> GetServiceProtocolHandlerTaskRunner(
      fml::StringView method) const override;

  // |blink::ServiceProtocol::Handler|
  bool HandleServiceProtocolMessage(
      fml::StringView method,  // one if the extension names specified above.
      const ServiceProtocolMap& params,
      rapidjson::Document& response) override;

  // |blink::ServiceProtocol::Handler|
  blink::ServiceProtocol::Handler::Description GetServiceProtocolDescription()
      const override;

  // Service protocol handler
  bool OnServiceProtocolScreenshot(
      const blink::ServiceProtocol::Handler::ServiceProtocolMap& params,
      rapidjson::Document& response);

  // Service protocol handler
  bool OnServiceProtocolScreenshotSKP(
      const blink::ServiceProtocol::Handler::ServiceProtocolMap& params,
      rapidjson::Document& response);

  // Service protocol handler
  bool OnServiceProtocolRunInView(
      const blink::ServiceProtocol::Handler::ServiceProtocolMap& params,
      rapidjson::Document& response);

  // Service protocol handler
  bool OnServiceProtocolFlushUIThreadTasks(
      const blink::ServiceProtocol::Handler::ServiceProtocolMap& params,
      rapidjson::Document& response);

  // Service protocol handler
  bool OnServiceProtocolSetAssetBundlePath(
      const blink::ServiceProtocol::Handler::ServiceProtocolMap& params,
      rapidjson::Document& response);

  FML_DISALLOW_COPY_AND_ASSIGN(ServiceProtocolHandler);
};

}  // namespace shell

#endif  // FLUTTER_SHELL_COMMON_SERVICE_PROTOCOL_HANDLER_H_
