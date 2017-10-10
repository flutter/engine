// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FUNCTION_PROFILER_FUNCTION_PROFILER_H_
#define FLUTTER_FUNCTION_PROFILER_FUNCTION_PROFILER_H_

#if __has_attribute(no_instrument_function)
#define NO_INSTRUMENT_FUNCTION __attribute__((no_instrument_function))
#else  // __has_attribute(no_instrument_function)
#define NO_INSTRUMENT_FUNCTION
#endif  // __has_attribute(no_instrument_function)

#endif  // FLUTTER_FUNCTION_PROFILER_FUNCTION_PROFILER_H_
