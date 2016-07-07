// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/platform_view.h"

#include "base/bind.h"
#include "base/location.h"
#include "base/single_thread_task_runner.h"
#include "sky/shell/rasterizer.h"

namespace sky {
namespace shell {

PlatformView::Config::Config() {}

PlatformView::Config::~Config() {}

PlatformView::PlatformView(const PlatformView::Config& config,
                           SurfaceConfig surface_config)
    : config_(config), surface_config_(surface_config) {}

PlatformView::~PlatformView() {}

void PlatformView::ConnectToEngine(mojo::InterfaceRequest<SkyEngine> request) {
  config_.ui_task_runner->PostTask(
      FROM_HERE, base::Bind(&UIDelegate::ConnectToEngine, config_.ui_delegate,
                            base::Passed(&request)));
}

void PlatformView::NotifyCreated() {
  // Tell the delegate that the output surface was created. As an argument,
  // for the gpu_continuation parameter, configure a closure that sets up the
  // completes the rasterizer connection.

  CHECK(config_.rasterizer != nullptr);

  auto delegate = config_.ui_delegate;
  auto rasterizer = config_.rasterizer->GetWeakRasterizerPtr();
  auto weak_this = GetWeakViewPtr();
  auto gpu_continuation = base::Bind(&Rasterizer::Setup,  // method
                                     rasterizer,          // target
                                     weak_this);
  config_.ui_task_runner->PostTask(
      FROM_HERE, base::Bind(&UIDelegate::OnOutputSurfaceCreated, delegate,
                            gpu_continuation));
}

void PlatformView::NotifyDestroyed() {
  // Tell the delegate that the output surface was destroyed. As an argument,
  // for the gpu_continuation parameter, configure a closure that tears down the
  // the rasterizer.
  CHECK(config_.rasterizer != nullptr);

  auto delegate = config_.ui_delegate;
  auto rasterizer = config_.rasterizer->GetWeakRasterizerPtr();
  auto gpu_continuation = base::Bind(&Rasterizer::Teardown, rasterizer);
  config_.ui_task_runner->PostTask(
      FROM_HERE, base::Bind(&UIDelegate::OnOutputSurfaceDestroyed, delegate,
                            gpu_continuation));
}

}  // namespace shell
}  // namespace sky
