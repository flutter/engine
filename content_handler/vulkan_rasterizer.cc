// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/content_handler/vulkan_rasterizer.h"

#include <fcntl.h>
#include <magenta/device/vfs.h>
#include <mxio/watcher.h>
#include <unistd.h>

#include <chrono>
#include <thread>
#include <utility>

#include "flutter/glue/trace_event.h"
#include "lib/ftl/files/unique_fd.h"

namespace flutter_runner {

static bool WaitForFirstDisplayDriver() {
  static constexpr char kDisplayDriverClass[] = "/dev/class/display";

  ftl::UniqueFD fd(open(kDisplayDriverClass, O_DIRECTORY | O_RDONLY));
  if (fd.get() < 0) {
    FTL_DLOG(ERROR) << "Failed to open " << kDisplayDriverClass;
    return false;
  }

  // Create the directory watch channel.
  vfs_watch_dir_t wd;
  wd.mask = VFS_WATCH_MASK_ALL;
  wd.options = 0;

  mx::channel watcher;
  mx_status_t status = mx_channel_create(0, &wd.channel, watcher.reset_and_get_address());
  if (status != MX_OK) {
    FTL_DLOG(ERROR) << "Failed to create channel";
    return false;
  }

  status = ioctl_vfs_watch_dir(fd.get(), &wd);
  if (status != MX_OK) {
    FTL_DLOG(ERROR) << "Failed to create directory watcher for "
                    << kDisplayDriverClass;
    return false;
  }

  mx_signals_t pending;
  // Wait for 1 second for the display driver to appear before falling back to
  // software.
  status = watcher.wait_one(MX_CHANNEL_READABLE | MX_CHANNEL_PEER_CLOSED,
                            mx_deadline_after(MX_SEC(1)), &pending);
  if (status != MX_OK) {
    FTL_DLOG(ERROR) << "Failed to wait on file watcher channel ";
    return false;
  }

  return pending & MX_CHANNEL_READABLE;
}

VulkanRasterizer::VulkanRasterizer() : compositor_context_(nullptr) {
  valid_ = WaitForFirstDisplayDriver();
  if (valid_) {
    surface_producer_ = std::make_unique<VulkanSurfaceProducer>();
  }
}

VulkanRasterizer::~VulkanRasterizer() = default;

bool VulkanRasterizer::IsValid() const {
  return valid_ && surface_producer_ && surface_producer_->IsValid();
}

void VulkanRasterizer::SetSession(
    fidl::InterfaceHandle<mozart2::Session> session,
    mx::eventpair import_token) {
  session_connection_ = std::make_unique<SessionConnection>(
      std::move(session), std::move(import_token));
}

void VulkanRasterizer::Draw(std::unique_ptr<flow::LayerTree> layer_tree,
                            ftl::Closure callback) {
  FTL_DCHECK(callback != nullptr);

  if (layer_tree == nullptr) {
    FTL_LOG(ERROR) << "Layer tree was not valid.";
    callback();
    return;
  }

  if (!session_connection_) {
    FTL_LOG(ERROR) << "Session was not valid.";
    callback();
    return;
  }

  compositor_context_.engine_time().SetLapTime(layer_tree->construction_time());

  flow::CompositorContext::ScopedFrame frame = compositor_context_.AcquireFrame(
      nullptr, nullptr, true /* instrumentation enabled */);
  {
    // Preroll the Flutter layer tree. This allows Flutter to perform pre-paint
    // optimizations.
    TRACE_EVENT0("flutter", "Preroll");
    layer_tree->Preroll(frame);
  }

  // TODO: Move this into the session connection.
  flow::SceneUpdateContext scene_update_context(session_connection_->session(),
                                                surface_producer_.get());

  {
    // Traverse the Flutter layer tree so that the necessary session ops to
    // represent the frame are enqueued in the underlying session.
    TRACE_EVENT0("flutter", "UpdateScene");
    layer_tree->UpdateScene(session_connection_->session(),
                            scene_update_context,
                            session_connection_->root_node());
  }

  {
    // Draw the contents of the scene to a surface.
    // We do this after publishing to take advantage of pipelining.
    TRACE_EVENT0("flutter", "ExecutePaintTasks");
    scene_update_context.ExecutePaintTasks(frame);
  }

  {
    // Perform the surface producer step.
    TRACE_EVENT0("flutter", "FinishFrame/Tick");
    if (!surface_producer_->FinishFrame()) {
      FTL_LOG(ERROR) << "Failed to Finish Frame";
      surface_producer_->Tick();
      callback();
      return;
    }
  }

  {
    // Flush all pending session ops.
    TRACE_EVENT0("flutter", "SessionPresent");
    session_connection_->Present([
      producer = surface_producer_.get(),  //
      callback = std::move(callback)       //
    ]() {
      producer->Tick();
      callback();
    });
  }
}

}  // namespace flutter_runner
