// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/files/glob.h"

#include <fcntl.h>
#include <lib/syslog/cpp/macros.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/lib/files/directory.h"
#include "src/lib/files/file.h"
#include "src/lib/files/scoped_temp_dir.h"
#include "src/lib/fxl/strings/substitute.h"

namespace files {
namespace {

class TestGlob : public testing::Test {
 protected:
  TestGlob() {
    // Test directory structure:
    // a
    // aa
    // b
    // c
    // d
    // e
    // subdir/
    //   abcd
    // z

    std::vector<std::string> names = {"a", "b", "c", "d", "z", "e", "aa"};
    for (const auto& name : names) {
      FX_CHECK(WriteFile(fxl::Substitute("$0/$1", dir_.path(), name), "a", 1));
    }
    FX_CHECK(CreateDirectory(fxl::Substitute("$0/subdir", dir_.path())));
    FX_CHECK(WriteFile(fxl::Substitute("$0/subdir/abcd", dir_.path()), "a", 1));
  }

  std::vector<std::string> PrependPath(const std::vector<std::string>& files) {
    std::vector<std::string> ret;
    for (const auto& file : files) {
      ret.push_back(fxl::Substitute("$0/$1", dir_.path(), file));
    }
    return ret;
  }

  ScopedTempDir dir_;
  const std::vector<std::string> sorted_names_in_dir_ = {"a", "aa", "b",      "c",
                                                         "d", "e",  "subdir", "z"};
};  // namespace

TEST_F(TestGlob, Empty) {
  Glob glob({});

  EXPECT_EQ(glob.size(), 0u);
  EXPECT_EQ(glob.begin(), glob.end());
}

TEST_F(TestGlob, EmptyDir) {
  ScopedTempDir dir2;
  auto path = fxl::Substitute("$0/*", dir2.path());
  Glob glob(path);

  EXPECT_EQ(glob.size(), 0u);
  EXPECT_EQ(glob.begin(), glob.end());
}

TEST_F(TestGlob, AllFiles) {
  auto path = fxl::Substitute("$0/*", dir_.path());
  Glob glob(path);
  EXPECT_EQ(glob.size(), sorted_names_in_dir_.size());
  std::vector<std::string> globbed = {glob.begin(), glob.end()};
  EXPECT_THAT(globbed, ::testing::ElementsAreArray(PrependPath(sorted_names_in_dir_)));
};

TEST_F(TestGlob, FilePrefix) {
  // Only "a*" files.
  auto path = fxl::Substitute("$0/a*", dir_.path());
  Glob glob(path);
  EXPECT_EQ(glob.size(), 2u);
  std::vector<std::string> globbed = {glob.begin(), glob.end()};
  std::vector<std::string> expected{"a", "aa"};

  EXPECT_THAT(globbed, ::testing::ElementsAreArray(PrependPath(expected)));
}

TEST_F(TestGlob, Subdirectory) {
  auto path = fxl::Substitute("$0/*/*", dir_.path());
  Glob glob(path);
  EXPECT_EQ(glob.size(), 1u);
  std::vector<std::string> globbed = {glob.begin(), glob.end()};
  std::vector<std::string> expected{"subdir/abcd"};

  EXPECT_THAT(globbed, ::testing::ElementsAreArray(PrependPath(expected)));
}

TEST_F(TestGlob, MultiplePaths) {
  auto path = fxl::Substitute("$0/*", dir_.path());
  auto path2 = fxl::Substitute("$0/*/*", dir_.path());
  Glob glob({path, path2});
  EXPECT_EQ(glob.size(), sorted_names_in_dir_.size() + 1);
  std::vector<std::string> globbed = {glob.begin(), glob.end()};
  std::vector<std::string> expected = sorted_names_in_dir_;
  expected.push_back("subdir/abcd");
  EXPECT_THAT(globbed, ::testing::ElementsAreArray(PrependPath(expected)));
}

TEST_F(TestGlob, MarkOption) {
  // Ensure that '/' is appended to directories only.
  auto path = fxl::Substitute("$0/*", dir_.path());
  Glob glob(path, {.mark = true});
  EXPECT_EQ(glob.size(), sorted_names_in_dir_.size());
  std::vector<std::string> globbed = {glob.begin(), glob.end()};
  auto expected = sorted_names_in_dir_;
  expected[expected.size() - 2] += "/";
  EXPECT_THAT(globbed, ::testing::ElementsAreArray(PrependPath(expected)));
  EXPECT_THAT(globbed[globbed.size() - 2], ::testing::EndsWith("subdir/"));
}

TEST_F(TestGlob, NoSortOption) {
  // Ensure we still retrieved the correct entries.
  auto path = fxl::Substitute("$0/*", dir_.path());
  Glob glob(path, {.no_sort = true});
  std::vector<std::string> globbed = {glob.begin(), glob.end()};
  EXPECT_THAT(globbed, ::testing::UnorderedElementsAreArray(PrependPath(sorted_names_in_dir_)));
}

TEST_F(TestGlob, Iterator) {
  // Ensure we still retrieved the correct entries.
  auto path = fxl::Substitute("$0/*", dir_.path());
  Glob glob(path);
  auto it = glob.begin();
  EXPECT_TRUE(it);
  EXPECT_FALSE(glob.end());

  auto first = *it;
  ++it;
  auto second = *it;
  --it;
  EXPECT_TRUE(it);
  EXPECT_EQ(*it, first);
  ++it;
  EXPECT_TRUE(it);
  EXPECT_EQ(*it, second);
}

}  // namespace
}  // namespace files
