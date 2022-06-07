// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_asset_manager.h"

#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/platform_configuration.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"

namespace flutter {

void FlutterAssetManager::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register({{"FlutterAssetManager_loadAsset",
                      FlutterAssetManager::loadAsset, 2, true}});
}

void FlutterAssetManager::loadAsset(Dart_NativeArguments args) {
  UIDartState::ThrowIfUIOperationsProhibited();
  Dart_Handle callback_handle = Dart_GetNativeArgument(args, 1);
  if (!Dart_IsClosure(callback_handle)) {
    Dart_SetReturnValue(args, tonic::ToDart("Callback must be a function"));
    return;
  }
  Dart_Handle asset_name_handle = Dart_GetNativeArgument(args, 0);
  uint8_t* chars = nullptr;
  intptr_t asset_length = 0;
  Dart_Handle result =
      Dart_StringToUTF8(asset_name_handle, &chars, &asset_length);
  if (Dart_IsError(result)) {
    Dart_SetReturnValue(args, tonic::ToDart("Asset must be valid UTF8"));
    return;
  }

  std::string asset_name = std::string{reinterpret_cast<const char*>(chars),
                                       static_cast<size_t>(asset_length)};

  std::shared_ptr<AssetManager> asset_manager = UIDartState::Current()
                                                    ->platform_configuration()
                                                    ->client()
                                                    ->GetAssetManager();
  std::unique_ptr<fml::Mapping> data = asset_manager->GetAsMapping(asset_name);

  Dart_Handle byte_buffer =
      tonic::DartByteData::Create(data->GetMapping(), data->GetSize());
  tonic::DartInvoke(callback_handle, {byte_buffer});
}

}  // namespace flutter
