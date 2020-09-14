// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_ASSETS_H_
#define FLUTTER_LIB_UI_ASSETS_H_

#include "flutter/lib/ui/window/platform_configuration.h"
#include "flutter/assets/asset_manager.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/tonic/logging/dart_invoke.h"
#include "third_party/tonic/typed_data/typed_list.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"
#include "third_party/tonic/dart_library_natives.h"

namespace flutter {

class Assets {
    public:
    static void loadAssetBytes(Dart_NativeArguments args);

    static void RegisterNatives(tonic::DartLibraryNatives* natives);
};
} // flutter
#endif  // FLUTTER_LIB_UI_ASSETS_H_
