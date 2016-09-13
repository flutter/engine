// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_GLUE_ALLOCATION_BUILDER_H_
#define FLUTTER_GLUE_ALLOCATION_BUILDER_H_

#include "lib/ftl/macros.h"
#include "lib/ftl/compiler_specific.h"

#include <sys/types.h>
#include <inttypes.h>

namespace glue {

class AllocationBuilder {
 public:
  AllocationBuilder();

  ~AllocationBuilder();

  uint32_t Size() const;

  FTL_WARN_UNUSED_RESULT
  bool Append(const uint8_t* data, uint32_t length);

  FTL_WARN_UNUSED_RESULT
  uint8_t* Take();

 private:
  uint8_t* buffer_;
  uint32_t buffer_length_;
  uint32_t data_length_;

  bool Reserve(uint32_t new_size);

  FTL_DISALLOW_COPY_AND_ASSIGN(AllocationBuilder);
};

}  // namespace glue

#endif  // FLUTTER_GLUE_ALLOCATION_BUILDER_H_
