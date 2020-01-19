// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_COMPONENT_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_COMPONENT_H_

#include <fuchsia/intl/cpp/fidl.h>
#include <fuchsia/io/cpp/fidl.h>
#include <fuchsia/sys/cpp/fidl.h>
#include <lib/fidl/cpp/binding.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/sys/cpp/service_directory.h>
#include <lib/vfs/cpp/composed_service_dir.h>
#include <lib/vfs/cpp/pseudo_dir.h>

#include <memory>
#include <string>
#include <utility>  // For std::pair
#include <vector>

#include "flutter/fml/mapping.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/fml/unique_fd.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/fuchsia/flutter/compositor/scenic_window.h"
#include "flutter/shell/platform/fuchsia/flutter/compositor/scenic_window_manager.h"
#include "flutter/shell/platform/fuchsia/flutter/isolate_configurator.h"
#include "flutter/shell/platform/fuchsia/flutter/unique_fdio_ns.h"

namespace flutter_runner {

// This embedder component represents a componentized instance of a Flutter
// application.  It acts as the embedder for one or more Flutter engine
// instances.
//
// This object is bound to the message dispatcher of the thread it is created
// on.  It should only ever be accessed on that thread.
class Component final : public fuchsia::sys::ComponentController {
 public:
  using TerminationCallback = std::function<void(const Component*)>;

  Component(
      TerminationCallback termination_callback,
      fuchsia::sys::Package package,
      fuchsia::sys::StartupInfo startup_info,
      std::shared_ptr<sys::ServiceDirectory> runner_incoming_services,
      fidl::InterfaceRequest<fuchsia::sys::ComponentController> controller);
  ~Component();
  Component(const Component&) = delete;
  Component& operator=(const Component&) = delete;

  const std::string& GetDebugLabel() const;

#if !defined(DART_PRODUCT)
  void WriteProfileToTrace() const;
#endif  // !defined(DART_PRODUCT)

 private:
  // This structure represents a single running instance of the Flutter engine,
  // along with objects used to customize it.
  struct Embedder {
    const std::string& debug_label;

    fml::WeakPtr<Component> weak_component;

    IsolateConfigurator configurator;
    FlutterEngine engine;
    ScenicWindow* window;
  };

  // This structure represents the parameters neccesary to initialize a new
  // |Embedder|.
  struct EmbedderProjectArgs {
    std::string data_path;
    std::string log_tag;
    std::vector<std::string> dart_flags;
    std::vector<std::string> dart_entrypoint_args;
    std::unique_ptr<fml::FileMapping> vm_snapshot_data_mapping;
    std::unique_ptr<fml::FileMapping> vm_snapshot_instructions_mapping;
    std::unique_ptr<fml::FileMapping> isolate_snapshot_data_mapping;
    std::unique_ptr<fml::FileMapping> isolate_snapshot_instructions_mapping;
  };

  // |fuchsia::sys::ComponentController|
  void Kill() override;

  // |fuchsia::sys::ComponentController|
  void Detach() override;

  void OnWindowCreated(ScenicWindow* window);
  void OnWindowDestroyed(ScenicWindow* window);
  void DestroyEmbedder(Embedder* embedder);
  void DestroyEmbedderInternal(std::unique_ptr<Embedder> embedder);

  const std::string debug_label_;
  const std::string component_url_;

  EmbedderProjectArgs project_args_;

  ScenicWindowManager window_manager_;

  UniqueFDIONS fdio_ns_ = UniqueFDIONSCreate();
  fml::UniqueFD assets_directory_;

  fuchsia::intl::PropertyProviderPtr intl_property_provider_;
  fuchsia::io::DirectoryPtr directory_ptr_;
  fuchsia::io::NodePtr cloned_directory_ptr_;
  fidl::InterfaceRequest<fuchsia::io::Directory> directory_request_;
  fidl::Binding<fuchsia::sys::ComponentController> component_controller_;

  // The |Embedder|'s address cannot change, because it is used as the
  // `user_data` for Embedder API callbacks.  Store the embedders here as
  // |unique_ptr| to ensure constant addresses.
  std::vector<std::unique_ptr<Embedder>> flutter_embedders_;
  std::pair<bool, uint32_t> last_return_code_;

  std::unique_ptr<vfs::PseudoDir> outgoing_dir_;
  std::shared_ptr<sys::ServiceDirectory> runner_incoming_services_;
  std::shared_ptr<sys::ServiceDirectory> component_incoming_services_;

  TerminationCallback termination_callback_;

  fml::WeakPtrFactory<Component> weak_factory_;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_COMPONENT_H_
