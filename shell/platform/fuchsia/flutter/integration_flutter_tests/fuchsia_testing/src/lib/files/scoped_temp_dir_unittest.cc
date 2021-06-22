// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/files/scoped_temp_dir.h"

#include <fcntl.h>

#include <fbl/unique_fd.h>
#include <gtest/gtest.h>

#include "src/lib/files/directory.h"
#include "src/lib/files/file.h"
#include "src/lib/files/path.h"

namespace files {
namespace {

TEST(ScopedTempDir, Creation) {
  ScopedTempDir dir;

  EXPECT_TRUE(IsDirectory(dir.path()));
  EXPECT_NE("temp_dir_XXXXXX", GetBaseName(dir.path()));
}

TEST(ScopedTempDir, Deletion) {
  std::string path;
  {
    ScopedTempDir dir;
    path = dir.path();
  }

  EXPECT_FALSE(IsDirectory(path));
}

TEST(ScopedTempDir, NewTempFile) {
  ScopedTempDir dir;
  std::string path;
  EXPECT_TRUE(dir.NewTempFile(&path));
  EXPECT_FALSE(path.empty());
  EXPECT_TRUE(IsFile(path));
}

TEST(ScopedTempDir, CustomParent) {
  ScopedTempDir root_dir;
  std::string parent = root_dir.path() + "/a/b/c";
  std::string path;
  {
    ScopedTempDir dir(parent);
    path = dir.path();
    EXPECT_TRUE(IsDirectory(path));
    EXPECT_EQ(path.substr(0, parent.size()), parent);

    // Regression test - don't create temp_dir_XXXXXX dir next to the temp one.
    EXPECT_FALSE(files::IsDirectory(GetDirectoryName(path) + "/temp_dir_XXXXXX"));
  }

  // Verify that the tmp directory itself was deleted, but not the parent.
  EXPECT_FALSE(IsDirectory(path));
  EXPECT_TRUE(IsDirectory(parent));
}

TEST(ScopedTempDirAt, Creation) {
  ScopedTempDir named_dir;
  fbl::unique_fd root_fd(open(named_dir.path().c_str(), O_RDONLY));
  ASSERT_TRUE(root_fd.is_valid());

  ScopedTempDirAt dir(root_fd.get());

  EXPECT_TRUE(IsDirectoryAt(root_fd.get(), dir.path()));
  EXPECT_NE("temp_dir_XXXXXX", GetBaseName(dir.path()));
}

TEST(ScopedTempDirAt, Deletion) {
  ScopedTempDir named_dir;
  fbl::unique_fd root_fd(open(named_dir.path().c_str(), O_RDONLY));
  ASSERT_TRUE(root_fd.is_valid());

  std::string path;
  {
    ScopedTempDirAt dir(root_fd.get());
    path = dir.path();
  }

  EXPECT_FALSE(IsDirectoryAt(root_fd.get(), path));
}

TEST(ScopedTempDirAt, NewTempFile) {
  ScopedTempDir named_dir;
  fbl::unique_fd root_fd(open(named_dir.path().c_str(), O_RDONLY));
  ASSERT_TRUE(root_fd.is_valid());

  ScopedTempDirAt dir(root_fd.get());
  std::string path;
  EXPECT_TRUE(dir.NewTempFile(&path));
  EXPECT_FALSE(path.empty());
  EXPECT_TRUE(IsFileAt(root_fd.get(), path));
}

TEST(ScopedTempDirAt, CustomParent) {
  ScopedTempDir named_dir;
  fbl::unique_fd root_fd(open(named_dir.path().c_str(), O_RDONLY));
  std::string parent = "a/b/c";
  std::string path;
  {
    ScopedTempDirAt dir(root_fd.get(), parent);
    path = dir.path();
    EXPECT_TRUE(IsDirectoryAt(root_fd.get(), path));
    EXPECT_EQ(path.substr(0, parent.size()), parent);
  }

  // Verify that the tmp directory itself was deleted, but not the parent.
  EXPECT_FALSE(IsDirectoryAt(root_fd.get(), path));
  EXPECT_TRUE(IsDirectoryAt(root_fd.get(), parent));
}

}  // namespace
}  // namespace files
