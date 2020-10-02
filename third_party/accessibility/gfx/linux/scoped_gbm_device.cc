// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/linux/scoped_gbm_device.h"

namespace ax {

void GbmDeviceDeleter::operator()(gbm_device* device) {
  if (device)
    gbm_device_destroy(device);
}

}  // namespace ax
