// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_binary_archive_metal.h"

#include <UIKit/UIKit.h>

#include <atomic>
#include <sstream>

#include "flutter/fml/closure.h"
#include "flutter/fml/file.h"
#include "flutter/fml/paths.h"
#include "flutter/shell/version/version.h"

namespace flutter {

static std::atomic_size_t sBinaryArchiveIndex = 0;

static std::string BinaryArchiveURLForIndex(size_t index) {
  auto items = [[NSFileManager defaultManager] URLsForDirectory:NSCachesDirectory
                                                      inDomains:NSUserDomainMask];
  if (items.count == 0) {
    return {};
  }

  std::stringstream stream;
  stream << "flutter_engine_" << GetFlutterEngineVersion() << "_" << GetSkiaVersion() << "_"
         << index << ".metallib";
  return fml::paths::JoinPaths({items[0].fileSystemRepresentation, stream.str()});
}
static std::string TemporaryBinaryArchiveURL(const std::string& binary_archive_url) {
  std::stringstream stream;
  stream << binary_archive_url << ".temp";
  return stream.str();
}

GPUBinaryArchiveMetal::GPUBinaryArchiveMetal(id<MTLDevice> device)
    : archive_index_(sBinaryArchiveIndex++),
      metallib_archive_path_(BinaryArchiveURLForIndex(archive_index_)) {
  InitializeArchive(device);

  // This object is created and collected on the platform thread with construction initiated by the
  // platform view. We need to look for opportunities to serialize the archive to disk at a time
  // when Flutter is not rendering frames. For this purpose, the did enter background notification
  // seems like a fine time as rendering frames here will terminate the process anyway.
  //
  // TODO: Verify if the owner of this object is collected on the platform thread or if its one of
  // the rendering surface owned objects which are collected on the raster thread. If this is
  // collected on the raster thread, the capture of `this` will be unsafe.
  notification_observer_.reset([[[NSNotificationCenter defaultCenter]
      addObserverForName:UIApplicationDidEnterBackgroundNotification
                  object:nil  // no receiver matching
                   queue:nil  // posting thread
              usingBlock:^(NSNotification* notification) {
                // TODO: This must be synchronized with collection of `this`.
                this->SerializeArchiveToDisk();
              }] retain]);
}

GPUBinaryArchiveMetal::~GPUBinaryArchiveMetal() {
  [[NSNotificationCenter defaultCenter] removeObserver:notification_observer_.get()];
  notification_observer_.reset();
}

bool GPUBinaryArchiveMetal::InitializeArchive(id<MTLDevice> device) {
  if (metallib_archive_path_.empty()) {
    return false;
  }
  auto descriptor =
      fml::scoped_nsobject<MTLBinaryArchiveDescriptor>([[MTLBinaryArchiveDescriptor alloc] init]);
  descriptor.get().url = fml::IsFile(metallib_archive_path_)
                             ? [NSURL fileURLWithPath:@(metallib_archive_path_.c_str())]
                             : nil;

  NSError* error = nil;
  binary_archive_.reset([device newBinaryArchiveWithDescriptor:descriptor.get() error:&error]);
  if (error != nil) {
    binary_archive_.reset();
    FML_LOG(ERROR) << "Could not create Metal Binary archive for engine run at index "
                   << archive_index_ << ". No pipeline state objects will be cached. Error: "
                   << error.localizedDescription.UTF8String;
    return false;
  }

  binary_archive_.get().label = @"Flutter Binary Archive";

  return true;
}

bool GPUBinaryArchiveMetal::SerializeArchiveToDisk() {
  if (binary_archive_ == nullptr) {
    // No binary archive to cache.
    return false;
  }
  std::scoped_lock serialization_lock(serialization_mutex_);
  // Clear the old archive and store the new one.

  // If the MTLBinaryArchive operated on a descriptor only, then it would be safe to directly unlink
  // the old descriptor here. However, we gave the full path to the archive in the constructor of
  // the descriptor. So the implementation could attempt to open the file descriptor again between
  // this point and the subsequent call to serialize. To be absolutely safe, write to a temporary
  // file and swap.

  auto temp_archive_string = TemporaryBinaryArchiveURL(metallib_archive_path_);
  if (temp_archive_string.empty()) {
    return false;
  }

  auto temp_archive_url = [NSURL fileURLWithPath:@(temp_archive_string.c_str())];
  NSError* error = nil;
  if (![binary_archive_.get() serializeToURL:temp_archive_url error:&error]) {
    FML_LOG(ERROR) << "Could not serialize Metal binary archive at index " << archive_index_
                   << ". Error: " << error.localizedDescription.UTF8String;
    return false;
  }

  // We are about to rename, flush pending writes to the the temporary archive.
  {
    // TODO: Is this necessary? Per the WriteAtomically implementation, one would think so, but we
    // don't own the descriptor on which write are being performed. Not sure this is safe.
#if 0
    if (::fsync(temp_archive_string.c_str()) != 0) {
      FML_LOG(ERROR) << "Could not fsync temporary Metal binary archive at index " << archive_index_
                     << ". Error: " << strerror(errno);
      return false;
    }
#endif
  }

  if (::rename(temp_archive_string.c_str(), metallib_archive_path_.c_str()) != 0) {
    FML_LOG(ERROR) << "Could not finish writing the Metal binary archive at index "
                   << archive_index_ << ". Error: " << strerror(errno);
    return false;
  }

  return true;
}

sk_cf_obj<GrMTLHandle> GPUBinaryArchiveMetal::GetBinaryArchiveHandle() const {
  return sk_cf_obj<GrMTLHandle>{[binary_archive_ retain]};
}

}  // namespace flutter
