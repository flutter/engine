// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOPAZ_RUNTIME_DART_UTILS_TEMPFS_H_
#define TOPAZ_RUNTIME_DART_UTILS_TEMPFS_H_

#include <lib/fdio/namespace.h>

// Utility functions that set up /tmp for the dart_runner and flutter_runner.

namespace dart_utils {

// Set up a memfs bound to /tmp in the process-wide namespace that has the
// lifetime of the process.
void SetupRunnerTemp();

// Take the memfs mapped into the process-wide namespace for /tmp, and map it to
// /tmp in the given namespace.
void SetupComponentTemp(fdio_ns_t* ns);

}  // namespace dart_utils

#endif  // TOPAZ_RUNTIME_DART_UTILS_TEMPFS_H_
