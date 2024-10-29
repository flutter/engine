// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_CORE_BUFFER_VIEW_H_
#define FLUTTER_IMPELLER_CORE_BUFFER_VIEW_H_

#include <memory>
#include "impeller/core/range.h"

namespace impeller {

class DeviceBuffer;

struct BufferView {
 public:
  BufferView() : buffer_(nullptr), range_({}) {}

  BufferView(std::shared_ptr<const DeviceBuffer> buffer, Range range)
      : buffer_(std::move(buffer)), range_(range) {}

  Range GetRange() const { return range_; }
  const std::shared_ptr<const DeviceBuffer> GetBuffer() const {
    return buffer_;
  }

  constexpr explicit operator bool() const {
    return static_cast<bool>(buffer_);
  }

 private:
  std::shared_ptr<const DeviceBuffer> buffer_;
  Range range_;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_CORE_BUFFER_VIEW_H_
