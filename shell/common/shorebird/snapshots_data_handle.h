#ifndef SHELL_COMMON_SHOREBIRD_SNAPSHOTS_DATA_HANDLE_H_
#define SHELL_COMMON_SHOREBIRD_SNAPSHOTS_DATA_HANDLE_H_

#include "flutter/fml/file.h"
#include "flutter/runtime/dart_snapshot.h"
#include "third_party/dart/runtime/include/dart_tools_api.h"

namespace flutter {

// An offset into an indexed collection of buffers. blob is the index of the
// buffer, and offset is the offset into that buffer.
struct BlobsIndex {
  size_t blob;
  size_t offset;
};

// Implements a POSIX file I/O interface which allows us to provide the four
// data blobs of a Dart snapshot (vm_data, vm_instructions, isolate_data,
// isolate_instructions) to Rust as though it were a single piece of memory.
class SnapshotsDataHandle {
 public:
  // This would ideally be private, but we need to be able to call it from the
  // static createForSnapshots method.
  explicit SnapshotsDataHandle(std::vector<std::unique_ptr<fml::Mapping>> blobs)
      : blobs_(std::move(blobs)) {}

  static std::unique_ptr<SnapshotsDataHandle> createForSnapshots(
      const DartSnapshot& vm_snapshot,
      const DartSnapshot& isolate_snapshot);

  uintptr_t Read(uint8_t* buffer, uintptr_t length);
  int64_t Seek(int64_t offset, int32_t whence);

  // The sum of all the blobs' sizes.
  size_t FullSize() const;

 private:
  size_t AbsoluteOffsetForIndex(BlobsIndex index);
  BlobsIndex IndexForAbsoluteOffset(int64_t offset, BlobsIndex startIndex);

  BlobsIndex current_index_ = {0, 0};
  std::vector<std::unique_ptr<fml::Mapping>> blobs_;
};

}  // namespace flutter

#endif  // SHELL_COMMON_SHOREBIRD_SNAPSHOTS_DATA_HANDLE_H_
