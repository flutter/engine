// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/core/host_buffer.h"

#include <cstring>
#include <tuple>

#include "impeller/core/allocator.h"
#include "impeller/core/buffer_view.h"
#include "impeller/core/device_buffer.h"
#include "impeller/core/device_buffer_descriptor.h"
#include "impeller/core/formats.h"

namespace impeller {

constexpr size_t kAllocatorBlockSize = 1024000;  // 1024 Kb.

std::shared_ptr<HostBuffer> HostBuffer::Create(
    const std::shared_ptr<Allocator>& allocator) {
  return std::shared_ptr<HostBuffer>(new HostBuffer(allocator));
}

HostBuffer::HostBuffer(const std::shared_ptr<Allocator>& allocator) {
  state_->allocator = allocator;
  DeviceBufferDescriptor desc;
  desc.size = kAllocatorBlockSize;
  desc.storage_mode = StorageMode::kHostVisible;
  state_->device_buffers.push_back(allocator->CreateBuffer(desc));
}

HostBuffer::~HostBuffer() = default;

void HostBuffer::SetLabel(std::string label) {
  state_->label = std::move(label);
}

BufferView HostBuffer::Emplace(const void* buffer,
                               size_t length,
                               size_t align) {
  auto [data, range, device_buffer] = state_->Emplace(buffer, length, align);
  if (!device_buffer) {
    return {};
  }
  return BufferView{std::move(device_buffer), data, range};
}

BufferView HostBuffer::Emplace(const void* buffer, size_t length) {
  auto [data, range, device_buffer] = state_->Emplace(buffer, length);
  if (!device_buffer) {
    return {};
  }
  return BufferView{std::move(device_buffer), data, range};
}

BufferView HostBuffer::Emplace(size_t length,
                               size_t align,
                               const EmplaceProc& cb) {
  auto [data, range, device_buffer] = state_->Emplace(length, align, cb);
  if (!device_buffer) {
    return {};
  }
  return BufferView{std::move(device_buffer), data, range};
}

std::shared_ptr<const DeviceBuffer> HostBuffer::GetDeviceBuffer(
    Allocator& allocator) const {
  return nullptr;
}

void HostBuffer::Reset() {
  state_->Reset();
}

void HostBuffer::HostBufferState::MaybeCreateNewBuffer() {
  if (current_buffer + 1 >= device_buffers.size()) {
    DeviceBufferDescriptor desc;
    desc.size = kAllocatorBlockSize;
    desc.storage_mode = StorageMode::kHostVisible;
    device_buffers.push_back(allocator->CreateBuffer(desc));
  }
  current_buffer++;
  offset = 0;
}

std::tuple<uint8_t*, Range, std::shared_ptr<DeviceBuffer>>
HostBuffer::HostBufferState::Emplace(size_t length,
                                     size_t align,
                                     const EmplaceProc& cb) {
  if (!cb) {
    return {};
  }
  auto old_length = GetLength();
  if (old_length + length > kAllocatorBlockSize) {
    MaybeCreateNewBuffer();
  }

  cb(GetCurrentBuffer()->OnGetContents() + old_length);

  offset += length;
  return std::make_tuple(GetCurrentBuffer()->OnGetContents(),
                         Range{old_length, length}, GetCurrentBuffer());
}

std::tuple<uint8_t*, Range, std::shared_ptr<DeviceBuffer>>
HostBuffer::HostBufferState::Emplace(const void* buffer, size_t length) {
  auto old_length = GetLength();
  if (old_length + length > kAllocatorBlockSize) {
    MaybeCreateNewBuffer();
  }

  if (buffer) {
    ::memmove(GetCurrentBuffer()->OnGetContents() + old_length, buffer, length);
  }
  offset += length;
  return std::make_tuple(GetCurrentBuffer()->OnGetContents(),
                         Range{old_length, length}, GetCurrentBuffer());
}

std::tuple<uint8_t*, Range, std::shared_ptr<DeviceBuffer>>
HostBuffer::HostBufferState::Emplace(const void* buffer,
                                     size_t length,
                                     size_t align) {
  if (align == 0 || (GetLength() % align) == 0) {
    return Emplace(buffer, length);
  }

  {
    auto [buffer, range, device_buffer] =
        Emplace(nullptr, align - (GetLength() % align));
    if (!buffer) {
      return {};
    }
  }

  return Emplace(buffer, length);
}

void HostBuffer::HostBufferState::Reset() {
  offset = 0u;
  current_buffer++;
}

}  // namespace impeller
