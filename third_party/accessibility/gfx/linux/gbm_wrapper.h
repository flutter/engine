// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_LINUX_GBM_WRAPPER_H_
#define UI_GFX_LINUX_GBM_WRAPPER_H_

#include <memory>

#include "ui/gfx/linux/gbm_device.h"

namespace ax {

std::unique_ptr<ax::GbmDevice> CreateGbmDevice(int fd);

}  // namespace ax

#endif  // UI_GFX_LINUX_GBM_WRAPPER_H_
