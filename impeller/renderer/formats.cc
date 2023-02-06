// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/formats.h"

#include "impeller/base/strings.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/texture.h"

namespace impeller {

constexpr bool StoreActionNeedsResolveTexture(StoreAction action) {
  switch (action) {
    case StoreAction::kDontCare:
    case StoreAction::kStore:
      return false;
    case StoreAction::kMultisampleResolve:
    case StoreAction::kStoreAndMultisampleResolve:
      return true;
  }
}

bool Attachment::IsValid() const {
  if (!texture || !texture->IsValid()) {
    VALIDATION_LOG << "Attachment has no texture.";
    return false;
  }

  if (StoreActionNeedsResolveTexture(store_action)) {
    if (!resolve_texture || !resolve_texture->IsValid()) {
      VALIDATION_LOG << "Store action needs resolve but no valid resolve "
                        "texture specified.";
      return false;
    }
  }

  if (resolve_texture) {
    if (store_action != StoreAction::kMultisampleResolve &&
        store_action != StoreAction::kStoreAndMultisampleResolve) {
      VALIDATION_LOG << "A resolve texture was specified, but the store action "
                        "doesn't include multisample resolve.";
      return false;
    }

    if (texture->GetTextureDescriptor().storage_mode ==
            StorageMode::kDeviceTransient &&
        store_action == StoreAction::kStoreAndMultisampleResolve) {
      VALIDATION_LOG
          << "The multisample texture cannot be transient when "
             "specifying the StoreAndMultisampleResolve StoreAction.";
    }
  }

  auto storage_mode = resolve_texture
                          ? resolve_texture->GetTextureDescriptor().storage_mode
                          : texture->GetTextureDescriptor().storage_mode;

  if (storage_mode == StorageMode::kDeviceTransient) {
    if (load_action == LoadAction::kLoad) {
      VALIDATION_LOG << "The LoadAction cannot be Load when attaching a device "
                        "transient " +
                            std::string(resolve_texture ? "resolve texture."
                                                        : "texture.");
      return false;
    }
    if (store_action != StoreAction::kDontCare) {
      VALIDATION_LOG << "The StoreAction must be DontCare when attaching a "
                        "device transient " +
                            std::string(resolve_texture ? "resolve texture."
                                                        : "texture.");
      return false;
    }
  }

  return true;
}

const char* PixelFormatToString(PixelFormat format) {
  switch (format) {
    case PixelFormat::kUnknown:
      return "kUnknown";
    case PixelFormat::kA8UNormInt:
      return "A8UNormInt";
    case PixelFormat::kR8UNormInt:
      return "kR8UNormInt";
    case PixelFormat::kR8G8UNormInt:
      return "kR8G8UNormInt";
    case PixelFormat::kR8G8B8A8UNormInt:
      return "kR8G8B8A8UNormInt";
    case PixelFormat::kR8G8B8A8UNormIntSRGB:
      return "kR8G8B8A8UNormIntSRGB";
    case PixelFormat::kB8G8R8A8UNormInt:
      return "kB8G8R8A8UNormInt";
    case PixelFormat::kB8G8R8A8UNormIntSRGB:
      return "kB8G8R8A8UNormIntSRGB";
    case PixelFormat::kR32G32B32A32Float:
      return "kR32G32B32A32Float";
    case PixelFormat::kR16G16B16A16Float:
      return "kR16G16B16A16Float";
    case PixelFormat::kS8UInt:
      return "kS8UInt";
    case PixelFormat::kD32FloatS8UInt:
      return "kD32FloatS8UInt";
  }

  FML_UNREACHABLE();
}

}  // namespace impeller
