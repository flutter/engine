#include <memory>
#include <string>
#include <vector>

#include "flutter/shell/common/shorebird/snapshots_data_handle.h"

#include "flutter/fml/mapping.h"
#include "flutter/runtime/dart_snapshot.h"
#include "flutter/testing/testing.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "testing/fixture_test.h"

namespace flutter {
namespace testing {

std::unique_ptr<SnapshotsDataHandle> MakeHandle(
    std::vector<std::string>& blobs) {
  // Map the strings into non-owned mappings:
  std::vector<std::unique_ptr<fml::Mapping>> mappings = {};
  for (auto& blob : blobs) {
    std::unique_ptr<fml::Mapping> mapping =
        std::make_unique<fml::NonOwnedMapping>(
            reinterpret_cast<const uint8_t*>(blob.data()), blob.size());
    mappings.push_back(std::move(mapping));
  }
  auto handle =
      std::make_unique<flutter::SnapshotsDataHandle>(std::move(mappings));
  return handle;
}

TEST(SnapshotsDataHandle, Read) {
  std::vector<std::string> blobs = {"abc", "def", "ghi", "jkl"};
  std::unique_ptr<SnapshotsDataHandle> blobs_handle = MakeHandle(blobs);

  const size_t buffer_size = 12;
  uint8_t buffer[buffer_size];
  std::fill(buffer, buffer + buffer_size, 0);
  blobs_handle->Read(buffer, 6);

  EXPECT_EQ(buffer[0], 'a');
  EXPECT_EQ(buffer[1], 'b');
  EXPECT_EQ(buffer[2], 'c');
  EXPECT_EQ(buffer[3], 'd');
  EXPECT_EQ(buffer[4], 'e');
  EXPECT_EQ(buffer[5], 'f');

  // Only the first 6 bytes should have been read.
  EXPECT_EQ(buffer[6], 0);
}

TEST(SnapshotsDataHandle, ReadAfterSeekWithPositiveOffset) {
  std::vector<std::string> blobs = {"abc", "def", "ghi", "jkl"};
  std::unique_ptr<SnapshotsDataHandle> blobs_handle = MakeHandle(blobs);

  const size_t buffer_size = 20;
  uint8_t buffer[buffer_size];
  std::fill(buffer, buffer + buffer_size, 0);

  blobs_handle->Seek(4, SEEK_CUR);
  blobs_handle->Read(buffer, 6);

  EXPECT_EQ(buffer[0], 'e');
  EXPECT_EQ(buffer[1], 'f');
  EXPECT_EQ(buffer[2], 'g');
  EXPECT_EQ(buffer[3], 'h');
  EXPECT_EQ(buffer[4], 'i');
  EXPECT_EQ(buffer[5], 'j');

  // Only the first 6 bytes should have been read.
  EXPECT_EQ(buffer[6], 0);
}

TEST(SnapshotsDataHandle, ReadAfterSeekWithNegativeOffset) {
  std::vector<std::string> blobs = {"abc", "def", "ghi", "jkl"};
  std::unique_ptr<SnapshotsDataHandle> blobs_handle = MakeHandle(blobs);

  const size_t buffer_size = 20;
  uint8_t buffer[buffer_size];
  std::fill(buffer, buffer + buffer_size, 0);

  blobs_handle->Read(buffer, 5);
  EXPECT_EQ(buffer[0], 'a');
  EXPECT_EQ(buffer[1], 'b');
  EXPECT_EQ(buffer[2], 'c');
  EXPECT_EQ(buffer[3], 'd');
  EXPECT_EQ(buffer[4], 'e');
  EXPECT_EQ(buffer[5], 0);

  // Reset buffer
  std::fill(buffer, buffer + buffer_size, 0);

  // Read 5, seeked back 4, should start reading at offset 1 ('b')
  blobs_handle->Seek(-4, SEEK_CUR);
  blobs_handle->Read(buffer, 6);

  EXPECT_EQ(buffer[0], 'b');
  EXPECT_EQ(buffer[1], 'c');
  EXPECT_EQ(buffer[2], 'd');
  EXPECT_EQ(buffer[3], 'e');
  EXPECT_EQ(buffer[4], 'f');
  EXPECT_EQ(buffer[5], 'g');
  EXPECT_EQ(buffer[6], 0);
}

TEST(SnapshotsDataHandle, SeekPastEnd) {
  std::vector<std::string> blobs = {"abc", "def", "ghi", "jkl"};
  std::unique_ptr<SnapshotsDataHandle> blobs_handle = MakeHandle(blobs);

  const size_t buffer_size = 20;
  uint8_t buffer[buffer_size];
  std::fill(buffer, buffer + buffer_size, 0);

  // Seek 1 past the end
  blobs_handle->Seek(blobs_handle->FullSize() + 1, SEEK_CUR);

  // Seek back 2 bytes and read 2 bytes
  blobs_handle->Seek(-2, SEEK_CUR);
  blobs_handle->Read(buffer, 2);

  EXPECT_EQ(buffer[0], 'k');
  EXPECT_EQ(buffer[1], 'l');
}

TEST(SnapshotsDataHandle, SeekBeforeBeginning) {
  std::vector<std::string> blobs = {"abc", "def", "ghi", "jkl"};
  std::unique_ptr<SnapshotsDataHandle> blobs_handle = MakeHandle(blobs);

  const size_t buffer_size = 20;
  uint8_t buffer[buffer_size];
  std::fill(buffer, buffer + buffer_size, 0);

  // Seek before the start of the blobs and read the first 2 bytes.
  blobs_handle->Seek(-2, SEEK_CUR);
  blobs_handle->Read(buffer, 2);

  EXPECT_EQ(buffer[0], 'a');
  EXPECT_EQ(buffer[1], 'b');
}

TEST(SnapshotsDataHandle, SeekFromBeginning) {
  std::vector<std::string> blobs = {"abc", "def", "ghi", "jkl"};
  std::unique_ptr<SnapshotsDataHandle> blobs_handle = MakeHandle(blobs);

  const size_t buffer_size = 20;
  uint8_t buffer[buffer_size];
  std::fill(buffer, buffer + buffer_size, 0);

  // Seek 10 bytes from current (the beginning)
  blobs_handle->Seek(10, SEEK_CUR);

  // Seek 2 bytes from the beginning and read 2 bytes
  blobs_handle->Seek(2, SEEK_SET);
  blobs_handle->Read(buffer, 2);

  EXPECT_EQ(buffer[0], 'c');
  EXPECT_EQ(buffer[1], 'd');
}

TEST(SnapshotsDataHandle, SeekFromEnd) {
  std::vector<std::string> blobs = {"abc", "def", "ghi", "jkl"};
  std::unique_ptr<SnapshotsDataHandle> blobs_handle = MakeHandle(blobs);

  const size_t buffer_size = 20;
  uint8_t buffer[buffer_size];
  std::fill(buffer, buffer + buffer_size, 0);

  // Seek 2 bytes from the end and read 2 bytes
  blobs_handle->Seek(-2, SEEK_END);
  blobs_handle->Read(buffer, 2);

  EXPECT_EQ(buffer[0], 'k');
  EXPECT_EQ(buffer[1], 'l');
}

}  // namespace testing
}  // namespace flutter
