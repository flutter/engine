// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/gpu/device_buffer.h"

#include "fml/mapping.h"
#include "impeller/core/device_buffer.h"
#include "impeller/core/device_buffer_descriptor.h"
#include "impeller/core/formats.h"
#include "impeller/core/platform.h"
#include "impeller/core/range.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"
#include "tonic/converter/dart_converter.h"

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(gpu, DeviceBuffer);

DeviceBuffer::DeviceBuffer(
    std::shared_ptr<impeller::DeviceBuffer> device_buffer)
    : device_buffer_(std::move(device_buffer)) {}

DeviceBuffer::~DeviceBuffer() = default;

Dart_Handle DeviceBuffer::Overwrite(const tonic::DartByteData& source_bytes,
                                    size_t destination_offset_in_bytes) {
  if (!device_buffer_->CopyHostBuffer(
          reinterpret_cast<const uint8_t*>(source_bytes.data()),
          impeller::Range(0, source_bytes.length_in_bytes()),
          destination_offset_in_bytes)) {
    return tonic::ToDart("Failed to overwrite device buffer.");
  }
  return Dart_Null();
}

}  // namespace flutter

//----------------------------------------------------------------------------
/// Exports
///

void InternalFlutterGpu_DeviceBuffer_Initialize(Dart_Handle wrapper,
                                                flutter::Context* gpu_context,
                                                int storage_mode,
                                                int size_in_bytes) {
  impeller::DeviceBufferDescriptor desc;
  desc.storage_mode = static_cast<impeller::StorageMode>(storage_mode);
  desc.size = size_in_bytes;
  auto device_buffer =
      gpu_context->GetContext()->GetResourceAllocator()->CreateBuffer(desc);

  auto res =
      fml::MakeRefCounted<flutter::DeviceBuffer>(std::move(device_buffer));
  res->AssociateWithDartWrapper(wrapper);
}

void InternalFlutterGpu_DeviceBuffer_InitializeWithHostData(
    Dart_Handle wrapper,
    flutter::Context* gpu_context,
    Dart_Handle byte_data) {
  auto data = tonic::DartByteData(byte_data);
  auto mapping = fml::NonOwnedMapping(reinterpret_cast<uint8_t*>(data.data()),
                                      data.length_in_bytes());
  auto device_buffer =
      gpu_context->GetContext()->GetResourceAllocator()->CreateBufferWithCopy(
          mapping);

  auto res =
      fml::MakeRefCounted<flutter::DeviceBuffer>(std::move(device_buffer));
  res->AssociateWithDartWrapper(wrapper);
}

Dart_Handle InternalFlutterGpu_DeviceBuffer_Overwrite(
    flutter::DeviceBuffer* device_buffer,
    Dart_Handle source_byte_data,
    int destination_offset_in_bytes) {
  if (destination_offset_in_bytes < 0) {
    return tonic::ToDart("destinationOffsetInBytes must be positive.");
  }
  return device_buffer->Overwrite(tonic::DartByteData(source_byte_data),
                                  destination_offset_in_bytes);
}
