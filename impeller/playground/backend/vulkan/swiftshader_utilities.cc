// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/playground/backend/vulkan/swiftshader_utilities.h"

#include <cstdlib>

#include "flutter/fml/file.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/paths.h"

namespace impeller {

static void FindSwiftShaderICDAtKnownPaths() {
  static constexpr const char* kSwiftShaderICDJSON = "vk_swiftshader_icd.json";
  static constexpr const char* kVulkanICDFileNamesEnvVariableKey =
      "VK_ICD_FILENAMES";
  const auto executable_directory_path =
      fml::paths::GetExecutableDirectoryPath();
  FML_CHECK(executable_directory_path.first);
  const auto executable_directory =
      fml::OpenDirectory(executable_directory_path.second.c_str(), false,
                         fml::FilePermission::kRead);
  FML_CHECK(executable_directory.is_valid());
  if (fml::FileExists(executable_directory, kSwiftShaderICDJSON)) {
    const auto icd_path = fml::paths::JoinPaths(
        {executable_directory_path.second, kSwiftShaderICDJSON});
    auto result = setenv(kVulkanICDFileNamesEnvVariableKey,  //
                         icd_path.c_str(),                   //
                         1                                   // overwrite
    );
    FML_CHECK(result == 0)
        << "Could not set the environment variable to use SwiftShader.";
  } else {
    FML_CHECK(false)
        << "Was asked to use SwiftShader but could not find the installable "
           "client driver (ICD) for the locally built SwiftShader.";
  }
}

void SetupSwiftshaderOnce(bool use_swiftshader) {
  static bool swiftshader_preference = false;
  static std::once_flag sOnceInitializer;
  std::call_once(sOnceInitializer, [use_swiftshader]() {
    if (use_swiftshader) {
      FindSwiftShaderICDAtKnownPaths();
      swiftshader_preference = use_swiftshader;
    }
  });
  FML_CHECK(swiftshader_preference == use_swiftshader)
      << "The option to use SwiftShader in a process can only be set once and "
         "may be changed later.";
}

}  // namespace impeller
