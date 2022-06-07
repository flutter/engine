// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_SERVICES_FLUTTER_ASSET_MANAGER_H_
#define FLUTTER_LIB_UI_SERVICES_FLUTTER_ASSET_MANAGER_H_

#include <cstdint>

#include "flutter/fml/macros.h"
#include "flutter/lib/ui/dart_wrapper.h"
#include "third_party/skia/include/core/SkData.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/tonic/logging/dart_invoke.h"
#include "third_party/tonic/typed_data/typed_list.h"

namespace flutter {

//------------------------------------------------------------------------------
/// A class that provides access to asset loading for dart:ui.
class FlutterAssetManager {
  public:
    /// Loads an asset into memory and makes it available to dart via a
    /// Uint8List.
    ///
    /// Internally this performs a copy to ensure thread-safety and to allow
    /// the buffer to be writable.
    ///
    /// The zero indexed argument is a String corresponding to the asset
    /// to load.
    ///
    /// The first indexed argument is expected to be a void callback to signal
    /// when the copy has completed.
    static void loadAsset(Dart_NativeArguments args);

    static void RegisterNatives(tonic::DartLibraryNatives* natives);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_SERVICES_FLUTTER_ASSET_MANAGER_H_
