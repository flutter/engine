// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSDstyle license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_SCENIC_COMPOSITOR_CONNECTION_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_SCENIC_COMPOSITOR_CONNECTION_H_

#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/sys/cpp/service_directory.h>
#include <lib/ui/scenic/cpp/view_ref_pair.h>

#include "flutter/fml/closure.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/fml/task_runner.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/fuchsia/flutter/scenic_platform_handler.h"
#include "flutter/shell/platform/fuchsia/flutter/scenic_scene_controller.h"

namespace flutter_runner {

// The embedder component that is responsible for interacting with the Scenic
// compositor on Fuchsia.
class ScenicCompositorConnection final {
 public:
  ScenicCompositorConnection(
      std::string debug_label,
      std::shared_ptr<sys::ServiceDirectory> runner_services,
      fml::RefPtr<fml::TaskRunner> platform_task_runner,
      fml::RefPtr<fml::TaskRunner> render_task_runner,
      fuchsia::ui::views::ViewToken view_token,
      fml::closure error_callback);
  ~ScenicCompositorConnection();

  FlutterCompositor GetCompositorCallbacks();

 private:
  std::optional<ScenicPlatformHandler> platform_handler_;
  std::optional<ScenicSceneController> scene_controller_;

  FML_DISALLOW_COPY_AND_ASSIGN(ScenicCompositorConnection);
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_SCENIC_COMPOSITOR_CONNECTION_H_
