// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_FXL_FILES_EINTR_WRAPPER_H_
#define LIB_FXL_FILES_EINTR_WRAPPER_H_

#include "src/lib/fxl/build_config.h"

#include <errno.h>

#if defined(NDEBUG)

#define HANDLE_EINTR(x)                                     \
  ({                                                        \
    decltype(x) eintr_wrapper_result;                       \
    do {                                                    \
      eintr_wrapper_result = (x);                           \
    } while (eintr_wrapper_result == -1 && errno == EINTR); \
    eintr_wrapper_result;                                   \
  })

#else

#define HANDLE_EINTR(x)                                                                      \
  ({                                                                                         \
    int eintr_wrapper_counter = 0;                                                           \
    decltype(x) eintr_wrapper_result;                                                        \
    do {                                                                                     \
      eintr_wrapper_result = (x);                                                            \
    } while (eintr_wrapper_result == -1 && errno == EINTR && eintr_wrapper_counter++ < 100); \
    eintr_wrapper_result;                                                                    \
  })

#endif  // NDEBUG

#define IGNORE_EINTR(x)                                   \
  ({                                                      \
    decltype(x) eintr_wrapper_result;                     \
    do {                                                  \
      eintr_wrapper_result = (x);                         \
      if (eintr_wrapper_result == -1 && errno == EINTR) { \
        eintr_wrapper_result = 0;                         \
      }                                                   \
    } while (0);                                          \
    eintr_wrapper_result;                                 \
  })

#endif  // LIB_FXL_FILES_EINTR_WRAPPER_H_
