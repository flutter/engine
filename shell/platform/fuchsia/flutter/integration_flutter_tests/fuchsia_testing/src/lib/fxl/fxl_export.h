// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_FXL_FXL_EXPORT_H_
#define LIB_FXL_FXL_EXPORT_H_

#include "src/lib/fxl/build_config.h"

#ifdef OS_FUCHSIA
#define FXL_EXPORT __attribute__((visibility("default")))
#else
#define FXL_EXPORT
#endif

#endif  // LIB_FXL_FXL_EXPORT_H_
