// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_COMPONENT_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_COMPONENT_H_

#include <fuchsia/intl/cpp/fidl.h>
#include <fuchsia/io/cpp/fidl.h>
#include <fuchsia/sys/cpp/fidl.h>
#include <lib/fdio/namespace.h>
#include <lib/fidl/cpp/binding.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/sys/cpp/outgoing_directory.h>
#include <lib/sys/cpp/service_directory.h>

#include <memory>
#include <string>
#include <utility>  // For std::pair
#include <vector>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer.h"
#include "flutter/shell/platform/fuchsia/utils/mapped_resource.h"
#include "flutter/shell/platform/fuchsia/utils/thread.h"

namespace flutter_runner {

// This object represents the set of all the |Thread|s used by an instance of
// the Flutter Engine.
struct ThreadHost {
 public:
  ThreadHost(std::string name_prefix)
      : platform_thread(name_prefix + ".platform"),
        gpu_thread(name_prefix + ".gpu") {}
  ThreadHost(const ThreadHost&) = delete;
  ThreadHost(ThreadHost&&) = default;
  ~ThreadHost() = default;

  ThreadHost& operator=(const ThreadHost&) = delete;
  ThreadHost& operator=(ThreadHost&&) = default;

  fx::Thread platform_thread;
  fx::Thread gpu_thread;
  // fx::Thread ui_thread;
  // fx::Thread io_thread;
};

// This object represents a componentized instance of the Flutter Engine.  It
// manages the |Thread|s used by the embedded Flutter Engine as well as the
// lifetime of the Engine itself.  The |fuchsia::sys::ComponentController|
// service provided to the |Component| manages the |Component|'s lifetime.
//
// Each |Component| is bound to its own thread and will create and destroy
// itself on that thread.  Access (including creation and destruction) is
// thread-safe but creation and destruction are synchronous operations.  Move
// is not permitted because it cannot be done in a thread-safe way without
// using locks.
//
// Each |Component| registers itself with its own |FlutterEngine| instance by
// passing its address as the `user_data` to `FlutterEngineInitialize` -- the
// |Component|'s address cannot ever change because of this action. If the
// address were to change, callbacks from the |FlutterEngine| (which all look
// up the appropriate |Component| to target by casting `user_data`) would fail.
//
// This object is non-copyable, and non-moveable.
class Component final : public fuchsia::sys::ComponentController {
 public:
  using Deleter = std::function<void(Component*)>;
  using TerminationCallback = std::function<void(const Component*)>;
  struct Context {
    fuchsia::sys::StartupInfo startup_info;

    std::string debug_label;
    std::string component_url;

    fidl::InterfaceRequest<fuchsia::sys::ComponentController>
        controller_request;
    std::shared_ptr<sys::ServiceDirectory> incoming_services;

    Renderer::FactoryCallback renderer_factory_callback;
    TerminationCallback termination_callback;
  };

  // This creation method creates the component on its own platform thread.
  // The Deleter in the returned pointer ensures that the component is also
  // destroyed on its own platform thread as well.
  static std::unique_ptr<Component, Deleter> Create(Context component_context);

  Component(const Component&) = delete;
  Component(Component&&) = delete;

  Component& operator=(const Component&) = delete;
  Component& operator=(Component&&) = delete;

#if !defined(DART_PRODUCT)
  void WriteProfileToTrace() const;
#endif  // !defined(DART_PRODUCT)

 private:
  Component(Context component_context, ThreadHost threads);
  ~Component();

  // |fuchsia::sys::ComponentController|
  void Kill() override;
  void Detach() override;

  void SetupOutgoingDirectory();
  void ParseStartupInfo(fuchsia::sys::StartupInfo startup_info);
  void LaunchFlutter();
  void TerminateFlutter();

  ThreadHost threads_;

  fx::ElfSnapshot aot_snapshot_;
  fx::MappedResource vm_snapshot_data_mapping_;
  fx::MappedResource vm_snapshot_instructions_mapping_;
  fx::MappedResource isolate_snapshot_data_mapping_;
  fx::MappedResource isolate_snapshot_instructions_mapping_;

  vfs::PseudoDir outgoing_directory_;
  fuchsia::io::DirectoryPtr flutter_directory_;
  fidl::Binding<fuchsia::sys::ComponentController> controller_binding_;

  std::string component_url_;
  std::string debug_label_;
  std::string assets_path_;
  std::vector<std::string> dart_flags_;
  std::vector<std::string> dart_entrypoint_args_;
  std::pair<bool, uint32_t> last_return_code_;

  std::shared_ptr<sys::ServiceDirectory> incoming_services_;
  std::unique_ptr<Renderer> renderer_;

  TerminationCallback termination_callback_;

  fdio_ns_t* component_namespace_ = nullptr;
  int assets_directory_ = -1;

  FlutterEngine engine_ = nullptr;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_COMPONENT_H_
