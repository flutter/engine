// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/versions.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_library_natives.h"

using tonic::DartConverter;

namespace blink {
  Versions::Versions(const char* dart_version_,
                     const char* skia_version_,
                     const char* flutter_engine_version_)
                    : dart_version(dart_version_),
                      skia_version(skia_version_),
                      flutter_engine_version(flutter_engine_version_) {}

  Versions::Versions(const Versions& other) = default;

  Versions::~Versions() = default;

  std::vector<std::string> Versions::GetVersionsList() {
    return {dart_version, skia_version, flutter_engine_version};
  }

  void GetVersions(Dart_NativeArguments args) {
    const auto& versions_list = UIDartState::Current()->GetVersions().GetVersionsList();
    const auto& dart_val = DartConverter<std::vector<std::string>>::ToDart(versions_list);
    Dart_SetReturnValue(args, dart_val);
  }

  static void RegisterNatives(tonic::DartLibraryNatives* natives) {
    natives->Register({
      {"Versions_getVersions", GetVersions, 1, true}
    });
  }
}  // namespace blink
