// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Metal/Metal.h>

#include <mutex>
#include <string>

#include "flutter/fml/macros.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "third_party/skia/include/gpu/mtl/GrMtlTypes.h"

#define FLUTTER_METAL_BINARY_ARCHIVE_AVAILABLE SK_API_AVAILABLE(macos(11.0), ios(14.0))

namespace flutter {

//------------------------------------------------------------------------------
/// @brief      A archive of pipeline state objects that can be serialized to disk.
///
///             Pipeline state objects will be collected as the application is running and will be
///             serialized to disk when the application is backgrounded.
///
///             Binary caches are serialized to a file in the caches directory with the following
///             format: flutter_engine_<flutter engine version>_<skia_version>_<engine instance
///             ID>.metallib.
///
///             Binary cache contents are specific to a particular GPU family and are not portable.
///
///             Limitations:
///             1: The archive will only be read the next time the application launched
///                and not when it is foregrounded.
///             2: Only the pipeline state objects for the first instance of the Flutter engine will
///                be archived and reused. This is because there is no way to provide an array of
///                binary archives to Skia when setting up the GrContext.
///
class FLUTTER_METAL_BINARY_ARCHIVE_AVAILABLE GPUBinaryArchiveMetal {
 public:
  GPUBinaryArchiveMetal(id<MTLDevice> device);

  ~GPUBinaryArchiveMetal();

  sk_cf_obj<GrMTLHandle> GetBinaryArchiveHandle() const;

 private:
  const size_t archive_index_;
  const std::string metallib_archive_path_;
  fml::scoped_nsprotocol<id<MTLBinaryArchive>> binary_archive_;
  std::mutex serialization_mutex_;
  fml::scoped_nsprotocol<id<NSObject>> notification_observer_;

  bool InitializeArchive(id<MTLDevice> device);

  //----------------------------------------------------------------------------
  /// @brief      Serializes the binary archive to disk. This can only be done
  ///             once. Once serialized, adding more pipeline state objects to
  ///             the binary archive at the given URL is undefined behavior.
  ///
  /// @return     If the binary archive was written to disk successfully.
  ///
  bool SerializeArchiveToDisk();

  FML_DISALLOW_COPY_AND_ASSIGN(GPUBinaryArchiveMetal);
};

}  // namespace flutter
