// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/files/path.h"

#include <fcntl.h>

#include <fbl/unique_fd.h>
#include <gtest/gtest.h>

#include "src/lib/files/directory.h"
#include "src/lib/files/scoped_temp_dir.h"
#include "src/lib/fxl/build_config.h"

namespace files {
namespace {

void ExpectPlatformPath(std::string expected, std::string actual) { EXPECT_EQ(expected, actual); }

TEST(Path, SimplifyPath) {
  ExpectPlatformPath(".", SimplifyPath(""));
  ExpectPlatformPath(".", SimplifyPath("."));
  ExpectPlatformPath("..", SimplifyPath(".."));
  ExpectPlatformPath("...", SimplifyPath("..."));

  ExpectPlatformPath("/", SimplifyPath("/"));
  ExpectPlatformPath("/", SimplifyPath("/."));
  ExpectPlatformPath("/", SimplifyPath("/.."));
  ExpectPlatformPath("/...", SimplifyPath("/..."));

  ExpectPlatformPath("foo", SimplifyPath("foo"));
  ExpectPlatformPath("foo", SimplifyPath("foo/"));
  ExpectPlatformPath("foo", SimplifyPath("foo/."));
  ExpectPlatformPath("foo", SimplifyPath("foo/./"));
  ExpectPlatformPath(".", SimplifyPath("foo/.."));
  ExpectPlatformPath(".", SimplifyPath("foo/../"));
  ExpectPlatformPath("foo/...", SimplifyPath("foo/..."));
  ExpectPlatformPath("foo/...", SimplifyPath("foo/.../"));
  ExpectPlatformPath("foo/.b", SimplifyPath("foo/.b"));
  ExpectPlatformPath("foo/.b", SimplifyPath("foo/.b/"));

  ExpectPlatformPath("/foo", SimplifyPath("/foo"));
  ExpectPlatformPath("/foo", SimplifyPath("/foo/"));
  ExpectPlatformPath("/foo", SimplifyPath("/foo/."));
  ExpectPlatformPath("/foo", SimplifyPath("/foo/./"));
  ExpectPlatformPath("/", SimplifyPath("/foo/.."));
  ExpectPlatformPath("/", SimplifyPath("/foo/../"));
  ExpectPlatformPath("/foo/...", SimplifyPath("/foo/..."));
  ExpectPlatformPath("/foo/...", SimplifyPath("/foo/.../"));
  ExpectPlatformPath("/foo/.b", SimplifyPath("/foo/.b"));
  ExpectPlatformPath("/foo/.b", SimplifyPath("/foo/.b/"));

  ExpectPlatformPath("foo/bar", SimplifyPath("foo/bar"));
  ExpectPlatformPath("foo/bar", SimplifyPath("foo/bar/"));
  ExpectPlatformPath("foo/bar", SimplifyPath("foo/./bar"));
  ExpectPlatformPath("foo/bar", SimplifyPath("foo/./bar/"));
  ExpectPlatformPath("bar", SimplifyPath("foo/../bar"));
  ExpectPlatformPath("bar", SimplifyPath("foo/baz/../../bar"));
  ExpectPlatformPath("bar", SimplifyPath("foo/../bar/"));
  ExpectPlatformPath("foo/.../bar", SimplifyPath("foo/.../bar"));
  ExpectPlatformPath("foo/.../bar", SimplifyPath("foo/.../bar/"));
  ExpectPlatformPath("foo/.b/bar", SimplifyPath("foo/.b/bar"));
  ExpectPlatformPath("foo/.b/bar", SimplifyPath("foo/.b/bar/"));

  ExpectPlatformPath("/foo/bar", SimplifyPath("/foo/bar"));
  ExpectPlatformPath("/foo/bar", SimplifyPath("/foo/bar/"));
  ExpectPlatformPath("/foo/bar", SimplifyPath("/foo/./bar"));
  ExpectPlatformPath("/foo/bar", SimplifyPath("/foo/./bar/"));
  ExpectPlatformPath("/bar", SimplifyPath("/foo/../bar"));
  ExpectPlatformPath("/bar", SimplifyPath("/foo/../bar/"));
  ExpectPlatformPath("/foo/.../bar", SimplifyPath("/foo/.../bar"));
  ExpectPlatformPath("/foo/.../bar", SimplifyPath("/foo/.../bar/"));
  ExpectPlatformPath("/foo/.b/bar", SimplifyPath("/foo/.b/bar"));
  ExpectPlatformPath("/foo/.b/bar", SimplifyPath("/foo/.b/bar/"));

  ExpectPlatformPath("../foo", SimplifyPath("../foo"));
  ExpectPlatformPath("../../bar", SimplifyPath("../foo/../../bar"));
  ExpectPlatformPath("/bar", SimplifyPath("/foo/../../bar"));

  // Already clean
  ExpectPlatformPath(".", SimplifyPath(""));
  ExpectPlatformPath("abc", SimplifyPath("abc"));
  ExpectPlatformPath("abc/def", SimplifyPath("abc/def"));
  ExpectPlatformPath("a/b/c", SimplifyPath("a/b/c"));
  ExpectPlatformPath(".", SimplifyPath("."));
  ExpectPlatformPath("..", SimplifyPath(".."));
  ExpectPlatformPath("../..", SimplifyPath("../.."));
  ExpectPlatformPath("../../abc", SimplifyPath("../../abc"));
  ExpectPlatformPath("/abc", SimplifyPath("/abc"));
  ExpectPlatformPath("/", SimplifyPath("/"));

  // Remove trailing slash
  ExpectPlatformPath("abc", SimplifyPath("abc/"));
  ExpectPlatformPath("abc/def", SimplifyPath("abc/def/"));
  ExpectPlatformPath("a/b/c", SimplifyPath("a/b/c/"));
  ExpectPlatformPath(".", SimplifyPath("./"));
  ExpectPlatformPath("..", SimplifyPath("../"));
  ExpectPlatformPath("../..", SimplifyPath("../../"));
  ExpectPlatformPath("/abc", SimplifyPath("/abc/"));

  // Remove doubled slash
  ExpectPlatformPath("abc/def/ghi", SimplifyPath("abc//def//ghi"));
  ExpectPlatformPath("/abc", SimplifyPath("//abc"));
  ExpectPlatformPath("/abc", SimplifyPath("///abc"));
  ExpectPlatformPath("/abc", SimplifyPath("//abc//"));
  ExpectPlatformPath("abc", SimplifyPath("abc//"));

  // Remove . elements
  ExpectPlatformPath("abc/def", SimplifyPath("abc/./def"));
  ExpectPlatformPath("/abc/def", SimplifyPath("/./abc/def"));
  ExpectPlatformPath("abc", SimplifyPath("abc/."));

  // Remove .. elements
  ExpectPlatformPath("abc/def/jkl", SimplifyPath("abc/def/ghi/../jkl"));
  ExpectPlatformPath("abc/jkl", SimplifyPath("abc/def/../ghi/../jkl"));
  ExpectPlatformPath("abc", SimplifyPath("abc/def/.."));
  ExpectPlatformPath(".", SimplifyPath("abc/def/../.."));
  ExpectPlatformPath("/", SimplifyPath("/abc/def/../.."));
  ExpectPlatformPath("..", SimplifyPath("abc/def/../../.."));
  ExpectPlatformPath("/", SimplifyPath("/abc/def/../../.."));
  ExpectPlatformPath("../../mno", SimplifyPath("abc/def/../../../ghi/jkl/../../../mno"));
  ExpectPlatformPath("/mno", SimplifyPath("/../mno"));

  // Combinations
  ExpectPlatformPath("def", SimplifyPath("abc/./../def"));
  ExpectPlatformPath("def", SimplifyPath("abc//./../def"));
  ExpectPlatformPath("../../def", SimplifyPath("abc/../../././../def"));
}

TEST(Path, AbsolutePath) {
  EXPECT_EQ("/foo/bar", AbsolutePath("/foo/bar"));
  EXPECT_EQ("/foo/bar/", AbsolutePath("/foo/bar/"));
  ASSERT_EQ(0, chdir("/tmp"));
  EXPECT_EQ(GetCurrentDirectory() + "/foo", AbsolutePath("foo"));
  EXPECT_EQ(GetCurrentDirectory(), AbsolutePath(""));

  // Test paths from the root directory.
  ASSERT_EQ(0, chdir("/"));
  EXPECT_EQ("/foo/bar", AbsolutePath("/foo/bar"));
  EXPECT_EQ("/foo/bar", AbsolutePath("foo/bar"));
}

TEST(Path, GetDirectoryName) {
  EXPECT_EQ("foo", GetDirectoryName("foo/"));
  EXPECT_EQ("foo/bar", GetDirectoryName("foo/bar/"));
  EXPECT_EQ("foo", GetDirectoryName("foo/bar"));
  EXPECT_EQ("foo/bar", GetDirectoryName("foo/bar/.."));
  EXPECT_EQ("foo/bar/..", GetDirectoryName("foo/bar/../.."));
  EXPECT_EQ("", GetDirectoryName("foo"));
  EXPECT_EQ("/", GetDirectoryName("/"));
  EXPECT_EQ("", GetDirectoryName("a"));
  EXPECT_EQ("/", GetDirectoryName("/a"));
  EXPECT_EQ("/a", GetDirectoryName("/a/"));
  EXPECT_EQ("a", GetDirectoryName("a/"));
}

TEST(Path, GetBaseName) {
  EXPECT_EQ("", GetBaseName("foo/"));
  EXPECT_EQ("", GetBaseName("foo/bar/"));
  EXPECT_EQ("bar", GetBaseName("foo/bar"));
  EXPECT_EQ("..", GetBaseName("foo/bar/.."));
  EXPECT_EQ("..", GetBaseName("foo/bar/../.."));
  EXPECT_EQ("foo", GetBaseName("foo"));
  EXPECT_EQ("", GetBaseName("/"));
  EXPECT_EQ("a", GetBaseName("a"));
  EXPECT_EQ("a", GetBaseName("/a"));
  EXPECT_EQ("", GetBaseName("/a/"));
  EXPECT_EQ("", GetBaseName("a/"));
}

TEST(Path, DeletePath) {
  ScopedTempDir dir;

  std::string sub_dir = dir.path() + "/dir";
  CreateDirectory(sub_dir);
  EXPECT_TRUE(IsDirectory(sub_dir));
  EXPECT_TRUE(DeletePath(sub_dir, false));
  EXPECT_FALSE(IsDirectory(sub_dir));
}

TEST(Path, DeletePathAt) {
  ScopedTempDir dir;
  fbl::unique_fd root(open(dir.path().c_str(), O_RDONLY));
  ASSERT_TRUE(root.is_valid());

  std::string sub_dir = "dir";
  CreateDirectoryAt(root.get(), sub_dir);
  EXPECT_TRUE(IsDirectoryAt(root.get(), sub_dir));
  EXPECT_TRUE(DeletePathAt(root.get(), sub_dir, false));
  EXPECT_FALSE(IsDirectoryAt(root.get(), sub_dir));
}

TEST(Path, DeletePathRecursively) {
  ScopedTempDir dir;

  std::string sub_dir = dir.path() + "/dir";
  CreateDirectory(sub_dir);
  EXPECT_TRUE(IsDirectory(sub_dir));

  std::string sub_sub_dir1 = sub_dir + "/dir1";
  CreateDirectory(sub_sub_dir1);
  EXPECT_TRUE(IsDirectory(sub_sub_dir1));
  std::string sub_sub_dir2 = sub_dir + "/dir2";
  CreateDirectory(sub_sub_dir2);
  EXPECT_TRUE(IsDirectory(sub_sub_dir2));

  EXPECT_FALSE(DeletePath(sub_dir, false));
  EXPECT_TRUE(IsDirectory(sub_dir));
  EXPECT_TRUE(IsDirectory(sub_sub_dir1));

  EXPECT_TRUE(DeletePath(sub_dir, true));
  EXPECT_FALSE(IsDirectory(sub_dir));
  EXPECT_FALSE(IsDirectory(sub_sub_dir1));
}

TEST(Path, DeletePathRecursivelyAt) {
  ScopedTempDir dir;
  fbl::unique_fd root(open(dir.path().c_str(), O_RDONLY));
  ASSERT_TRUE(root.is_valid());

  std::string sub_dir = "dir";
  CreateDirectoryAt(root.get(), sub_dir);
  EXPECT_TRUE(IsDirectoryAt(root.get(), sub_dir));

  std::string sub_sub_dir1 = sub_dir + "/dir1";
  CreateDirectoryAt(root.get(), sub_sub_dir1);
  EXPECT_TRUE(IsDirectoryAt(root.get(), sub_sub_dir1));
  std::string sub_sub_dir2 = sub_dir + "/dir2";
  CreateDirectoryAt(root.get(), sub_sub_dir2);
  EXPECT_TRUE(IsDirectoryAt(root.get(), sub_sub_dir2));

  EXPECT_FALSE(DeletePathAt(root.get(), sub_dir, false));
  EXPECT_TRUE(IsDirectoryAt(root.get(), sub_dir));
  EXPECT_TRUE(IsDirectoryAt(root.get(), sub_sub_dir1));

  EXPECT_TRUE(DeletePathAt(root.get(), sub_dir, true));
  EXPECT_FALSE(IsDirectoryAt(root.get(), sub_dir));
  EXPECT_FALSE(IsDirectoryAt(root.get(), sub_sub_dir1));
}

TEST(Path, JoinPath) {
  EXPECT_EQ(JoinPath("foo", ""), "foo");
  EXPECT_EQ(JoinPath("foo", "bar"), "foo/bar");
  EXPECT_EQ(JoinPath("foo", "bar/"), "foo/bar/");
  EXPECT_EQ(JoinPath("foo", "/bar"), "foo/bar");
  EXPECT_EQ(JoinPath("foo", "/bar/"), "foo/bar/");

  EXPECT_EQ(JoinPath("foo/", ""), "foo/");
  EXPECT_EQ(JoinPath("foo/", ""), "foo/");
  EXPECT_EQ(JoinPath("foo/", "bar"), "foo/bar");
  EXPECT_EQ(JoinPath("foo/", "bar/"), "foo/bar/");
  EXPECT_EQ(JoinPath("foo/", "/bar"), "foo/bar");
  EXPECT_EQ(JoinPath("foo/", "/bar/"), "foo/bar/");

  EXPECT_EQ(JoinPath("/foo", ""), "/foo");
  EXPECT_EQ(JoinPath("/foo", "bar"), "/foo/bar");
  EXPECT_EQ(JoinPath("/foo", "bar/"), "/foo/bar/");
  EXPECT_EQ(JoinPath("/foo", "/bar"), "/foo/bar");
  EXPECT_EQ(JoinPath("/foo", "/bar/"), "/foo/bar/");

  EXPECT_EQ(JoinPath("/foo/", ""), "/foo/");
  EXPECT_EQ(JoinPath("/foo/", "bar"), "/foo/bar");
  EXPECT_EQ(JoinPath("/foo/", "bar/"), "/foo/bar/");
  EXPECT_EQ(JoinPath("/foo/", "/bar"), "/foo/bar");
  EXPECT_EQ(JoinPath("/foo/", "/bar/"), "/foo/bar/");

  EXPECT_EQ(JoinPath("", ""), "");
  EXPECT_EQ(JoinPath("", "bar"), "bar");
  EXPECT_EQ(JoinPath("", "bar/"), "bar/");
  EXPECT_EQ(JoinPath("", "/bar"), "/bar");
  EXPECT_EQ(JoinPath("", "/bar/"), "/bar/");

  EXPECT_EQ(JoinPath("/foo/bar/baz/", "/blah/blink/biz"), "/foo/bar/baz/blah/blink/biz");
}

}  // namespace
}  // namespace files
