// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file names the OS we're currently building on.

#ifndef LIB_FXL_BUILD_CONFIG_H_
#define LIB_FXL_BUILD_CONFIG_H_

#if defined(__Fuchsia__)
#define OS_FUCHSIA 1
#elif defined(__APPLE__)
#define OS_MACOSX 1
#elif defined(__linux__)
#define OS_LINUX 1
#else
#error Please add support for your platform in lib/fxl/build_config.h
#endif

#endif  // LIB_FXL_BUILD_CONFIG_H_
