// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TOOLKIT_ANDROID_HARDWARE_BUFFER_H_
#define FLUTTER_IMPELLER_TOOLKIT_ANDROID_HARDWARE_BUFFER_H_

#include "flutter/fml/unique_object.h"
#include "impeller/geometry/size.h"
#include "impeller/toolkit/android/proc_table.h"

namespace impeller::android {

enum class HardwareBufferFormat {
  //----------------------------------------------------------------------------
  /// This format is guaranteed to be supported on all versions of Android.
  ///
  /// Why have many format when one format do trick.
  ///
  kR8G8B8A8UNormInt,
};

using HardwareBufferUsage = uint8_t;

enum class HardwareBufferUsageFlags : HardwareBufferUsage {
  kFrameBufferAttachment = 1u << 0u,
  kCompositorOverlay = 1u << 1u,
  kSampledImage = 1u << 2u,
};

struct HardwareBufferDescriptor {
  HardwareBufferFormat format = HardwareBufferFormat::kR8G8B8A8UNormInt;
  ISize size;
  HardwareBufferUsage usage = 0u;

  static HardwareBufferDescriptor MakeForSwapchainImage(ISize size);

  bool IsAllocatable() const;

  constexpr bool operator==(const HardwareBufferDescriptor& o) const {
    return format == o.format && size == o.size && usage == o.usage;
  }

  constexpr bool operator!=(const HardwareBufferDescriptor& o) const {
    return !(*this == o);
  }
};

//------------------------------------------------------------------------------
/// @brief      A wrapper for AHardwareBuffer
///             https://developer.android.com/ndk/reference/group/a-hardware-buffer
///
///             This wrapper creates and owns a handle to a managed hardware
///             buffer. That is, there is no ability to take a reference to an
///             externally created hardware buffer.
///
///             This wrapper is only available on Android API 29 and above.
///
class HardwareBuffer {
 public:
  explicit HardwareBuffer(HardwareBufferDescriptor descriptor);

  ~HardwareBuffer();

  HardwareBuffer(const HardwareBuffer&) = delete;

  HardwareBuffer& operator=(const HardwareBuffer&) = delete;

  bool IsValid() const;

  AHardwareBuffer* GetHandle() const;

  const HardwareBufferDescriptor& GetDescriptor() const;

  const AHardwareBuffer_Desc& GetAndroidDescriptor() const;

 private:
  struct UniqueAHardwareBufferTraits {
    static AHardwareBuffer* InvalidValue() { return nullptr; }

    static bool IsValid(AHardwareBuffer* value) {
      return value != InvalidValue();
    }

    static void Free(AHardwareBuffer* value) {
      GetProcTable().AHardwareBuffer_release(value);
    }
  };

  const HardwareBufferDescriptor descriptor_;
  const AHardwareBuffer_Desc android_descriptor_;
  fml::UniqueObject<AHardwareBuffer*, UniqueAHardwareBufferTraits> buffer_;
  bool is_valid_ = false;
};

}  // namespace impeller::android

#endif  // FLUTTER_IMPELLER_TOOLKIT_ANDROID_HARDWARE_BUFFER_H_
