// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application.h"

#include <sstream>

#include "flutter/shell/common/switches.h"
#include "lib/fsl/vmo/vector.h"
#include "lib/fxl/command_line.h"
#include "lib/fxl/synchronization/waitable_event.h"
#include "task_observers.h"

namespace flutter {

std::pair<std::unique_ptr<fsl::Thread>, std::unique_ptr<Application>>
Application::Create(
    Application::Delegate& delegate,
    app::ApplicationPackagePtr package,
    app::ApplicationStartupInfoPtr startup_info,
    f1dl::InterfaceRequest<app::ApplicationController> controller) {
  auto thread = std::make_unique<fsl::Thread>();
  std::unique_ptr<Application> application;

  fxl::AutoResetWaitableEvent latch;
  thread->TaskRunner()->PostTask([&]() mutable {
    application.reset(new Application(delegate,                 //
                                      std::move(package),       //
                                      std::move(startup_info),  //
                                      std::move(controller)     //
                                      ));
    latch.Signal();
  });
  thread->Run();
  latch.Wait();
  return {std::move(thread), std::move(application)};
}

static std::string DebugLabelForURL(const std::string url) {
  auto found = url.rfind("/");
  if (found == std::string::npos) {
    return url;
  } else {
    return {url, found + 1};
  }
}

Application::Application(Application::Delegate& delegate,
                         app::ApplicationPackagePtr package,
                         app::ApplicationStartupInfoPtr startup_info,
                         f1dl::InterfaceRequest<app::ApplicationController>
                             application_controller_request)
    : delegate_(delegate),
      debug_label_(DebugLabelForURL(startup_info->launch_info->url)),
      application_controller_(this) {
  application_controller_.set_error_handler([this]() { Kill(); });

  FXL_DCHECK(fdio_ns_.is_valid());
  // ApplicationLaunchInfo::url non-optional.
  auto& launch_info = startup_info->launch_info;

  // ApplicationLaunchInfo::arguments optional.
  if (auto& arguments = launch_info->arguments) {
    settings_ = shell::SettingsFromCommandLine(
        fxl::CommandLineFromIterators(arguments.begin(), arguments.end()));
  }

  // TODO: ApplicationLaunchInfo::out optional.

  // TODO: ApplicationLaunchInfo::err optional.

  // ApplicationLaunchInfo::service_request optional.
  if (launch_info->service_request) {
    service_provider_bridge_.ServeDirectory(
        std::move(launch_info->service_request));
  }

  // ApplicationLaunchInfo::services optional.
  if (launch_info->services) {
    service_provider_bridge_.AddBinding(std::move(launch_info->services));
  }

  // ApplicationLaunchInfo::flat_namespace optional.
  if (auto& flat_namespace = startup_info->flat_namespace) {
    for (size_t i = 0; i < flat_namespace->paths.size(); ++i) {
      const auto& path = flat_namespace->paths[i];
      if (path == "/svc") {
        continue;
      }

      zx::channel dir = std::move(flat_namespace->directories[i]);
      zx_handle_t dir_handle = dir.release();
      if (fdio_ns_bind(fdio_ns_.get(), path.data(), dir_handle) != ZX_OK) {
        FXL_DLOG(ERROR) << "Could not bind path to namespace: " << path;
        zx_handle_close(dir_handle);
      }
    }
  } else {
    FXL_DLOG(ERROR) << "There was no flat namespace.";
  }

  application_directory_.reset(fdio_ns_opendir(fdio_ns_.get()));
  FXL_DCHECK(application_directory_.is_valid());

  application_assets_directory_.reset(
      openat(application_directory_.get(), "pkg/data", O_RDONLY | O_DIRECTORY));

  // TODO: ApplicationLaunchInfo::additional_services optional.

  // ApplicationPackage::data: This is legacy FLX data. Ensure that we dont have
  // any.
  FXL_DCHECK(!package->data) << "Legacy FLX data must not be supplied.";

  // All launch arguments have been read. Perform service binding and
  // final settings configuration. The next call will be to create a view
  // for this application.

  service_provider_bridge_.AddService<mozart::ViewProvider>(
      std::bind(&Application::CreateShellForView, this, std::placeholders::_1));

  app::ServiceProviderPtr outgoing_services;
  outgoing_services_request_ = outgoing_services.NewRequest();
  service_provider_bridge_.set_backend(std::move(outgoing_services));

  // Setup the application controller binding.
  if (application_controller_request) {
    application_controller_.Bind(std::move(application_controller_request));
  }

  application_context_ =
      app::ApplicationContext::CreateFrom(std::move(startup_info));

  settings_.icu_data_path = "";

  settings_.using_blink = false;

  settings_.verbose_logging = true;

  settings_.assets_dir = application_assets_directory_.get();

  settings_.script_snapshot_path = "snapshot_blob.bin";

  settings_.log_tag = debug_label_ + std::string{"(flutter)"};

  settings_.task_observer_add =
      std::bind(&CurrentMessageLoopAddAfterTaskObserver, std::placeholders::_1,
                std::placeholders::_2);

  settings_.task_observer_remove = std::bind(
      &CurrentMessageLoopRemoveAfterTaskObserver, std::placeholders::_1);
}

Application::~Application() = default;

// |app::ApplicationController|
void Application::Kill() {
  if (last_return_code_.first) {
    for (auto wait_callback : wait_callbacks_) {
      wait_callback(last_return_code_.second);
    }
  }
  wait_callbacks_.clear();

  delegate_.OnApplicationTerminate(this);
  // WARNING: Don't do anything past this point as this instance may have been
  // collected.
}

// |app::ApplicationController|
void Application::Detach() {
  application_controller_.set_error_handler(nullptr);
}

// |app::ApplicationController|
void Application::Wait(const WaitCallback& callback) {
  wait_callbacks_.emplace_back(std::move(callback));
}

// |flutter::Engine::Delegate|
void Application::OnEngineTerminate(const Engine* shell_holder) {
  auto found = std::find_if(shell_holders_.begin(), shell_holders_.end(),
                            [shell_holder](const auto& holder) {
                              return holder.get() == shell_holder;
                            });

  if (found == shell_holders_.end()) {
    return;
  }

  // We may launch multiple shell in this application. However, we will
  // terminate when the last shell goes away. The error code return to the
  // application controller will be the last isolate that had an error.
  auto return_code = shell_holder->GetEngineReturnCode();
  if (return_code.first) {
    last_return_code_ = return_code;
  }

  shell_holders_.erase(found);

  if (shell_holders_.size() == 0) {
    Kill();
    // WARNING: Don't do anything past this point because the delegate may have
    // collected this instance via the termination callback.
  }
}

void Application::CreateShellForView(
    f1dl::InterfaceRequest<mozart::ViewProvider> view_provider_request) {
  shells_bindings_.AddBinding(this, std::move(view_provider_request));
}

// |mozart::ViewProvider|
void Application::CreateView(
    f1dl::InterfaceRequest<mozart::ViewOwner> view_owner,
    f1dl::InterfaceRequest<app::ServiceProvider>) {
  if (!application_context_) {
    FXL_DLOG(ERROR) << "Application context was invalid when attempting to "
                       "create a shell for a view provider request.";
    return;
  }

  // This method may be called multiple times. Care must be taken to ensure that
  // all arguments can be accessed or synthesized multiple times.
  // TODO(chinmaygarde): Figure out how to re-create the outgoing service
  // request handle.
  shell_holders_.emplace(std::make_unique<Engine>(
      *this,                                 // delegate
      debug_label_,                          // thread label
      *application_context_,                 // application context
      settings_,                             // settings
      std::move(view_owner),                 // view owner
      fdio_ns_,                              // FDIO namespace
      std::move(outgoing_services_request_)  // outgoing request
      ));
}

}  // namespace flutter
