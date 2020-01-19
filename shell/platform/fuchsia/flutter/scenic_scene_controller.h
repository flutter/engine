// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSDstyle license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_SCENIC_SCENE_CONTROLLER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_SCENIC_SCENE_CONTROLLER_H_

#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <lib/ui/scenic/cpp/resources.h>
#include <lib/ui/scenic/cpp/session.h>
#include <lib/ui/scenic/cpp/view_ref_pair.h>
//#include <zx/event.h>

#include "flutter/fml/closure.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/fml/task_runner.h"
#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter_runner {

// The embedder component that is responsible for manipulating the embedders'
// Scenic scene in response to new frames being created by the renderer.  It is
// sole owner and user of the |Session|.
class ScenicSceneController final {
 public:
  ScenicSceneController(
      std::string debug_label,
      fml::RefPtr<fml::TaskRunner> task_runner,
      fidl::InterfaceHandle<fuchsia::ui::scenic::Session> session,
      fuchsia::ui::views::ViewToken view_token,
      scenic::ViewRefPair view_ref_pair,
      fml::closure error_callback);
  ~ScenicSceneController();

  // Methods invoked via FlutterCompositor callbacks.  Called on the GPU thread.
  bool CreateBackingStore(const FlutterBackingStoreConfig* layer_config,
                          FlutterBackingStore* backing_store_out);
  bool CollectBackingStore(const FlutterBackingStore* backing_store);
  bool PresentLayers(const FlutterLayer** layers, size_t layer_count);

  void EnableWireframe(bool enable);
  void Present();

 private:
  void PresentSession();

  fml::RefPtr<fml::TaskRunner> task_runner_;

  scenic::Session session_;

  scenic::View root_view_;
  scenic::EntityNode root_node_;

  zx::event vsync_event_;

  // A flow event trace id for following |Session::Present| calls into
  // Scenic.  This will be incremented each |Session::Present| call.  By
  // convention, the Scenic side will also contain its own trace id that
  // begins at 0, and is incremented each |Session::Present| call.
  uint64_t next_present_trace_id_ = 0;
  uint64_t next_present_session_trace_id_ = 0;
  uint64_t processed_present_session_trace_id_ = 0;

  bool presentation_callback_pending_ = false;
  bool present_session_pending_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(ScenicSceneController);
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_SCENIC_SCENE_CONTROLLER_H_
