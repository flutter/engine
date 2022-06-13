// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/assets/directory_asset_bundle.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DirectoryAssetBundle, MappingIsReadWrite) {
  fml::ScopedTemporaryDirectory temp_dir;
  const char* filename = "foo.txt";
  fml::MallocMapping write_mapping(static_cast<uint8_t*>(calloc(1, 4)), 4);
  fml::WriteAtomically(temp_dir.fd(), filename, write_mapping);
  fml::UniqueFD descriptor =
      fml::OpenDirectory(temp_dir.path().c_str(), /*create_if_necessary=*/false,
                         fml::FilePermission::kRead);
  std::unique_ptr<AssetResolver> bundle =
      std::make_unique<DirectoryAssetBundle>(
          std::move(descriptor), /*is_valid_after_asset_manager_change=*/true);
  EXPECT_TRUE(bundle->IsValid());
  std::unique_ptr<fml::Mapping> read_mapping = bundle->GetAsMapping(filename);
  ASSERT_TRUE(read_mapping);

#if defined(OS_FUCHSIA) || defined(FML_OS_WIN)
  ASSERT_FALSE(read_mapping->GetMutableMapping());
#else
  ASSERT_TRUE(read_mapping->GetMutableMapping());
  EXPECT_EQ(read_mapping->GetSize(), 4u);
  read_mapping->GetMutableMapping()[0] = 'A';
  EXPECT_EQ(read_mapping->GetMapping()[0], 'A');
  std::unique_ptr<fml::Mapping> read_after_write_mapping =
      bundle->GetAsMapping(filename);
  EXPECT_EQ(read_after_write_mapping->GetMapping()[0], '\0');
#endif
}

}  // namespace testing
}  // namespace flutter
