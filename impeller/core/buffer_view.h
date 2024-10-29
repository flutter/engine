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
  BufferView() : buffer_(nullptr), raw_buffer_(nullptr), range_({}) {}

  BufferView(DeviceBuffer* buffer, Range range)
      : buffer_(), raw_buffer_(buffer), range_(range) {}

  BufferView(std::shared_ptr<const DeviceBuffer> buffer, Range range)
      : buffer_(std::move(buffer)), raw_buffer_(nullptr), range_(range) {}

  Range GetRange() const { return range_; }

  const DeviceBuffer* GetBuffer() const {
    return raw_buffer_ ? raw_buffer_ : buffer_.get();
  }

  std::shared_ptr<const DeviceBuffer> TakeBuffer() {
    raw_buffer_ = buffer_.get();
    return std::move(buffer_);
  }

  constexpr explicit operator bool() const { return buffer_ || raw_buffer_; }

 private:
  std::shared_ptr<const DeviceBuffer> buffer_;
  const DeviceBuffer* raw_buffer_;
  Range range_;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_CORE_BUFFER_VIEW_H_
