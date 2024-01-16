#include "flutter/shell/common/shorebird/snapshots_data_handle.h"

#include "third_party/dart/runtime/include/dart_native_api.h"

namespace flutter {

static std::unique_ptr<fml::Mapping> DataMapping(const DartSnapshot& snapshot) {
  auto ptr = snapshot.GetDataMapping();
  return std::make_unique<fml::NonOwnedMapping>(ptr,
                                                Dart_SnapshotDataSize(ptr));
}

static std::unique_ptr<fml::Mapping> InstructionsMapping(
    const DartSnapshot& snapshot) {
  auto ptr = snapshot.GetInstructionsMapping();
  return std::make_unique<fml::NonOwnedMapping>(ptr,
                                                Dart_SnapshotInstrSize(ptr));
}

// The size of the snapshot data is the sum of the sizes of the blobs.
size_t SnapshotsDataHandle::FullSize() const {
  size_t size = 0;
  for (const auto& blob : blobs_) {
    size += blob->GetSize();
  }
  return size;
}

// The offset into the snapshots data blobs as though they were a single
// contiguous buffer.
size_t SnapshotsDataHandle::AbsoluteOffsetForIndex(BlobsIndex index) {
  if (index.blob >= blobs_.size()) {
    FML_LOG(WARNING) << "Blob index " << index.blob
                     << " is larger than the number of blobs (" << blobs_.size()
                     << "). Returning full size (" << FullSize() << ")";
    return FullSize();
  }
  if (index.offset > blobs_[index.blob]->GetSize()) {
    FML_LOG(WARNING) << "Offset for blob " << index.blob << " (" << index.offset
                     << ") is larger than the blob size ("
                     << blobs_[index.blob]->GetSize()
                     << "). Returning index start of next blob";
    return AbsoluteOffsetForIndex({index.blob + 1, 0});
  }
  size_t offset = 0;
  for (size_t i = 0; i < index.blob; i++) {
    offset += blobs_[i]->GetSize();
  }
  offset += index.offset;
  return offset;
}

BlobsIndex SnapshotsDataHandle::IndexForAbsoluteOffset(int64_t offset,
                                                       BlobsIndex start_index) {
  size_t start_offset = AbsoluteOffsetForIndex(start_index);
  if (offset < 0) {
    if ((size_t)abs(offset) > start_offset) {
      FML_LOG(WARNING)
          << "Offset is before the beginning of SnapshotsData. Returning 0, 0";
      return {0, 0};
    }
  } else if (offset + start_offset >= FullSize()) {
    FML_LOG(WARNING) << "Target offset is past the end of SnapshotsData ("
                     << offset + start_offset << ", blobs size:" << FullSize()
                     << "). Returning last blob index and offset";
    return {blobs_.size(), blobs_.back()->GetSize()};
  }

  size_t dest_offset = start_offset + offset;
  BlobsIndex index = {0, 0};
  for (const auto& blob : blobs_) {
    if (dest_offset < blob->GetSize()) {
      // The remaining offset is within this blob.
      index.offset = dest_offset;
      break;
    }

    index.blob++;
    dest_offset -= blob->GetSize();
  }
  return index;
}

std::unique_ptr<SnapshotsDataHandle> SnapshotsDataHandle::createForSnapshots(
    const DartSnapshot& vm_snapshot,
    const DartSnapshot& isolate_snapshot) {
  // This needs to match the order in which the blobs are written out in
  // analyze_snapshot --dump_blobs
  std::vector<std::unique_ptr<fml::Mapping>> blobs;
  blobs.push_back(DataMapping(vm_snapshot));
  blobs.push_back(DataMapping(isolate_snapshot));
  blobs.push_back(InstructionsMapping(vm_snapshot));
  blobs.push_back(InstructionsMapping(isolate_snapshot));
  return std::make_unique<SnapshotsDataHandle>(std::move(blobs));
}

uintptr_t SnapshotsDataHandle::Read(uint8_t* buffer, uintptr_t length) {
  uintptr_t bytes_read = 0;
  // Copy current blob from current offset and possibly into the next blob
  // until we have read length bytes.
  while (bytes_read < length) {
    if (current_index_.blob >= blobs_.size()) {
      // We have read all blobs.
      break;
    }
    intptr_t remaining_blob_length =
        blobs_[current_index_.blob]->GetSize() - current_index_.offset;
    if (remaining_blob_length <= 0) {
      // We have read all bytes in this blob.
      current_index_.blob++;
      current_index_.offset = 0;
      continue;
    }
    intptr_t bytes_to_read = fmin(length - bytes_read, remaining_blob_length);
    memcpy(buffer + bytes_read,
           blobs_[current_index_.blob]->GetMapping() + current_index_.offset,
           bytes_to_read);
    bytes_read += bytes_to_read;
    current_index_.offset += bytes_to_read;
  }

  return bytes_read;
}

int64_t SnapshotsDataHandle::Seek(int64_t offset, int32_t whence) {
  BlobsIndex start_index;
  switch (whence) {
    case SEEK_CUR:
      start_index = current_index_;
      break;
    case SEEK_SET:
      start_index = {0, 0};
      break;
    case SEEK_END:
      start_index = {blobs_.size(), blobs_.back()->GetSize()};
      break;
    default:
      FML_CHECK(false) << "Unrecognized whence value in Seek: " << whence;
  }
  current_index_ = IndexForAbsoluteOffset(offset, start_index);
  return current_index_.offset;
}

}  // namespace flutter