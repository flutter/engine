// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LIB_FSL_VMO_STRINGS_H_
#define SRC_LIB_FSL_VMO_STRINGS_H_

#include <fuchsia/mem/cpp/fidl.h>
#include <lib/zx/vmo.h>

#include <string>
#include <string_view>

#include "src/lib/fsl/vmo/sized_vmo.h"
#include "src/lib/fxl/fxl_export.h"

namespace fsl {

// Make a new shared buffer with the contents of a string.
FXL_EXPORT bool VmoFromString(std::string_view string, SizedVmo* handle_ptr);

// Make a new shared buffer with the contents of a string.
FXL_EXPORT bool VmoFromString(std::string_view string, fuchsia::mem::Buffer* buffer_ptr);

// Copy the contents of a shared buffer into a string.
FXL_EXPORT bool StringFromVmo(const SizedVmo& handle, std::string* string_ptr);

// Copy the contents of a shared buffer into a string.
FXL_EXPORT bool StringFromVmo(const fuchsia::mem::Buffer& handle, std::string* string_ptr);

// Copy the contents of a shared buffer upto |num_bytes| into a string.
// |num_bytes| should be <= |handle.size|.
FXL_EXPORT bool StringFromVmo(const fuchsia::mem::Buffer& handle, size_t num_bytes,
                              std::string* string_ptr);

}  // namespace fsl

#endif  // SRC_LIB_FSL_VMO_STRINGS_H_
