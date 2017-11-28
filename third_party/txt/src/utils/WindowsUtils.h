// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_H
#define WINDOWS_UTILS_H

#if defined(_WIN32)
#define DISABLE_TEST_WINDOWS(TEST_NAME) DISABLED_##TEST_NAME
#define FRIEND_TEST_WINDOWS_DISABLED_EXPANDED(SUITE, TEST_NAME) FRIEND_TEST(SUITE, TEST_NAME)
#define FRIEND_TEST_WINDOWS_DISABLED(SUITE, TEST_NAME) \
  FRIEND_TEST_WINDOWS_DISABLED_EXPANDED(SUITE, DISABLE_TEST_WINDOWS(TEST_NAME))

#define NOMINMAX
#include <windows.h>
#include <BaseTsd.h>
#include <intrin.h>

#undef ERROR

inline unsigned int clz_win(unsigned int num) {
  unsigned long r = 0;
  _BitScanReverse(&r, num);
  return r;
}

inline unsigned int clzl_win(unsigned long num) {
  unsigned long r = 0;
  _BitScanReverse64(&r, num);
  return r;
}

inline unsigned int ctz_win(unsigned int num) {
  unsigned long r = 0;
  _BitScanForward(&r, num);
  return r;
}

typedef SSIZE_T ssize_t;

#else
#define DISABLE_TEST_WINDOWS(TEST_NAME) TEST_NAME
#define FRIEND_TEST_WINDOWS_DISABLED(SUITE, TEST_NAME) FRIEND_TEST(SUITE, TEST_NAME)
#endif  // defined(_WIN32)
#endif  // WINDOWS_UTILS_H
