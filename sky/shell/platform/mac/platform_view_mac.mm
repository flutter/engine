// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/platform/mac/platform_view_mac.h"

namespace sky {
namespace shell {

PlatformView* PlatformView::Create(const Config& config) {
  return new PlatformViewMac(config);
}

PlatformViewMac::PlatformViewMac(const Config& config)
    : PlatformView(config), weak_factory_(this) {}

PlatformViewMac::~PlatformViewMac() {
  weak_factory_.InvalidateWeakPtrs();
}

base::WeakPtr<sky::shell::PlatformView>
PlatformViewMac::GetWeakViewPtr() {
  return weak_factory_.GetWeakPtr();
}

uint64_t PlatformViewMac::DefaultFramebuffer() const {
  CHECK(false);
  return 0;
}

bool PlatformViewMac::ContextMakeCurrent() {
  CHECK(false);
  return false;
}

bool PlatformViewMac::SwapBuffers() {
  CHECK(false);
  return false;
}

}  // namespace shell
}  // namespace sky
