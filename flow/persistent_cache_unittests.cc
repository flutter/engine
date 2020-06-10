// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "flutter/assets/directory_asset_bundle.h"
#include "flutter/flow/persistent_cache.h"
#include "flutter/fml/file.h"
#include "flutter/fml/log_settings.h"
#include "flutter/fml/paths.h"
#include "flutter/fml/unique_fd.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {
namespace {

constexpr char kTestCacheVersion[] = "PersistentCacheTest";
constexpr char kTestCacheVersionOld[] = "PersistentCacheTest_old";

void CheckTextSkData(sk_sp<SkData> data, const std::string& expected) {
  std::string data_string(reinterpret_cast<const char*>(data->bytes()),
                          data->size());
  EXPECT_EQ(data_string, expected);
}

}  // namespace

#ifndef NDEBUG
TEST(PersistentCacheTest, NoCacheVersionDies) {
  PersistentCache::SetCacheVersion("");
  EXPECT_DEATH_IF_SUPPORTED(PersistentCache::ResetCacheForProcess(),
                            "Cache version unspecified");
}
#endif

TEST(PersistentCacheTest, DefaultCacheDirIsValid) {
  // Linux / Windows do not suport the default cache dir.
#if defined(OS_LINUX) || defined(OS_WINDOWS)
  GTEST_SKIP();
#endif

  fml::UniqueFD base_dir = fml::paths::GetCachesDirectory();
  ASSERT_TRUE(base_dir.is_valid());

  PersistentCache::SetCacheDirectoryPath("");
  PersistentCache::SetCacheVersion(kTestCacheVersion);
  PersistentCache::ResetCacheForProcess();

  auto current_dir = fml::OpenDirectoryReadOnly(base_dir, kTestCacheVersion);
  EXPECT_TRUE(current_dir.is_valid());

  // Cleanup
  fml::RemoveFilesInDirectory(base_dir);
}

TEST(PersistentCacheTest, OverrideCacheDirIsValid) {
  fml::ScopedTemporaryDirectory base_dir;
  ASSERT_TRUE(base_dir.fd().is_valid());

  PersistentCache::SetCacheDirectoryPath(base_dir.path());
  PersistentCache::SetCacheVersion(kTestCacheVersion);
  PersistentCache::ResetCacheForProcess();

  auto current_dir =
      fml::OpenDirectoryReadOnly(base_dir.fd(), kTestCacheVersion);
  EXPECT_TRUE(current_dir.is_valid());
}

TEST(PersistentCacheTest, CanRemoveOldPersistentCache) {
  fml::ScopedTemporaryDirectory base_dir;
  ASSERT_TRUE(base_dir.fd().is_valid());

  // Create the "old cache directory" before initializing the cache.
  auto old_created = fml::CreateDirectory(base_dir.fd(), {kTestCacheVersionOld},
                                          fml::FilePermission::kReadWrite);
  EXPECT_TRUE(old_created.is_valid());

  // Initializing the cache should remove the old directory and create the new
  // one.
  PersistentCache::SetCacheDirectoryPath(base_dir.path());
  PersistentCache::SetCacheVersion(kTestCacheVersion);
  PersistentCache::ResetCacheForProcess();

  auto current_dir =
      fml::OpenDirectoryReadOnly(base_dir.fd(), kTestCacheVersion);
  auto old_dir =
      fml::OpenDirectoryReadOnly(base_dir.fd(), kTestCacheVersionOld);
  EXPECT_TRUE(current_dir.is_valid());
  EXPECT_FALSE(old_dir.is_valid());

  // Ensure that only the current cache version remains.
  fml::VisitFiles(base_dir.fd(), [](const fml::UniqueFD& directory,
                                    const std::string& filename) {
    EXPECT_EQ(strcmp(filename.c_str(), kTestCacheVersion), 0);
    return true;
  });
}

TEST(PersistentCacheTest, CanLoadSkSLsFromCacheDir) {
  fml::ScopedTemporaryDirectory base_dir;
  ASSERT_TRUE(base_dir.fd().is_valid());

  auto sksl_dir = fml::CreateDirectory(
      base_dir.fd(), {kTestCacheVersion, PersistentCache::kSkSLSubdirName},
      fml::FilePermission::kReadWrite);
  const std::string x = "x";
  const std::string y = "y";
  auto x_data = std::make_unique<fml::DataMapping>(
      std::vector<uint8_t>{x.begin(), x.end()});
  auto y_data = std::make_unique<fml::DataMapping>(
      std::vector<uint8_t>{y.begin(), y.end()});
  ASSERT_TRUE(fml::WriteAtomically(sksl_dir, "IE", *x_data));
  ASSERT_TRUE(fml::WriteAtomically(sksl_dir, "II", *y_data));

  PersistentCache::SetCacheDirectoryPath(base_dir.path());
  PersistentCache::SetCacheVersion(kTestCacheVersion);
  PersistentCache::ResetCacheForProcess();

  // Test that the cache can load the SkSLs.
  {
    auto shaders = PersistentCache::GetCacheForProcess()->LoadSkSLs();
    EXPECT_EQ(shaders.size(), 2u);

    // Make sure that the 2 shaders are sorted by their keys. Their keys should
    // be "A" and "B" (decoded from "II" and "IE").
    if (shaders[0].first->bytes()[0] == 'B') {
      std::swap(shaders[0], shaders[1]);
    }

    CheckTextSkData(shaders[0].first, "A");
    CheckTextSkData(shaders[1].first, "B");
    CheckTextSkData(shaders[0].second, "x");
    CheckTextSkData(shaders[1].second, "y");
  }
}

TEST(PersistentCacheTest, CanLoadSkSLsFromAsset) {
  // Temp dir for the assets.
  fml::ScopedTemporaryDirectory base_dir;
  ASSERT_TRUE(base_dir.fd().is_valid());

  PersistentCache::SetCacheDirectoryPath(base_dir.path());
  PersistentCache::SetCacheVersion(kTestCacheVersion);
  PersistentCache::ResetCacheForProcess();

  // The SkSL key is Base32 encoded. "IE" is the encoding of "A" and "II" is the
  // encoding of "B".
  //
  // The SkSL data is Base64 encoded. "eA==" is the encoding of "x" and "eQ=="
  // is the encoding of "y".
  const std::string kTestJson =
      "{\n"
      "  \"data\": {\n"
      "    \"IE\": \"eA==\",\n"
      "    \"II\": \"eQ==\"\n"
      "  }\n"
      "}\n";

  auto data = std::make_unique<fml::DataMapping>(
      std::vector<uint8_t>{kTestJson.begin(), kTestJson.end()});
  fml::WriteAtomically(base_dir.fd(), PersistentCache::kAssetFileName, *data);

  // Reset the asset manager and ensure no SkSLs are loaded.
  PersistentCache::SetAssetManager(nullptr);
  EXPECT_EQ(PersistentCache::GetCacheForProcess()->LoadSkSLs().size(), 0u);

  auto asset_manager = std::make_shared<AssetManager>();
  asset_manager->PushBack(
      std::make_unique<DirectoryAssetBundle>(fml::OpenDirectory(
          base_dir.path().c_str(), false, fml::FilePermission::kRead)));
  PersistentCache::SetAssetManager(asset_manager);

  // Test that the cache can load the SkSLs.
  {
    auto shaders = PersistentCache::GetCacheForProcess()->LoadSkSLs();
    EXPECT_EQ(shaders.size(), 2u);

    // Make sure that the 2 shaders are sorted by their keys. Their keys should
    // be "A" and "B" (decoded from "II" and "IE").
    if (shaders[0].first->bytes()[0] == 'B') {
      std::swap(shaders[0], shaders[1]);
    }

    CheckTextSkData(shaders[0].first, "A");
    CheckTextSkData(shaders[1].first, "B");
    CheckTextSkData(shaders[0].second, "x");
    CheckTextSkData(shaders[1].second, "y");
  }
}

// TODO()
// TEST_F(PersistentcacheTest, CanPurgePersistentCache) {
//   fml::ScopedTemporaryDirectory base_dir;
//   ASSERT_TRUE(base_dir.fd().is_valid());

//   auto cache_dir = fml::CreateDirectory(
//       base_dir.fd(),
//       {"flutter_engine", GetFlutterEngineVersion(), "skia",
//       GetSkiaVersion()}, fml::FilePermission::kReadWrite);
//   PersistentCache::SetCacheDirectoryPath(base_dir.path());
//   PersistentCache::ResetCacheForProcess();

//   // Generate a dummy persistent cache.
//   fml::DataMapping test_data(std::string("test"));
//   ASSERT_TRUE(fml::WriteAtomically(cache_dir, "test", test_data));
//   auto file = fml::OpenFileReadOnly(cache_dir, "test");
//   ASSERT_TRUE(file.is_valid());

//   // Run engine with purge_persistent_cache to remove the dummy cache.
//   auto settings = CreateSettingsForFixture();
//   settings.purge_persistent_cache = true;
//   auto config = RunConfiguration::InferFromSettings(settings);
//   std::unique_ptr<Shell> shell = CreateShell(settings);
//   RunEngine(shell.get(), std::move(config));

//   // Verify that the dummy is purged.
//   file = fml::OpenFileReadOnly(cache_dir, "test");
//   ASSERT_FALSE(file.is_valid());
// }

}  // namespace testing
}  // namespace flutter
