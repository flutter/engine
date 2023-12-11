// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/compiler/source_options.h"
#include "impeller/compiler/switches.h"

namespace impeller {
namespace compiler {

std::optional<ShaderBundleConfig> ParseShaderBundleConfig(
    const std::string& json_config,
    std::ostream& error_stream);

bool GenerateShaderBundle(Switches& switches, SourceOptions& options);

}  // namespace compiler
}  // namespace impeller
