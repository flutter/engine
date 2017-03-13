// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/linux/timerfd.h"

#if FML_TIMERFD_AVAILABLE == 0

#include <asm/unistd.h>
#include <sys/syscall.h>

int timerfd_create(int clockid, int flags) {
  return syscall(__NR_timerfd_create, clockid, flags);
}

int timerfd_settime(int ufc,
                    int flags,
                    const struct itimerspec* utmr,
                    struct itimerspec* otmr) {
  return syscall(__NR_timerfd_settime, ufc, flags, utmr, otmr);
}

#endif  // FML_TIMERFD_AVAILABLE == 0
