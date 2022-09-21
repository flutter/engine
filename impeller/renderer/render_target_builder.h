// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <map>
#include <optional>
#include "context.h"
#include "flutter/fml/macros.h"
#include "impeller/geometry/size.h"
#include "impeller/renderer/allocator.h"
#include "impeller/renderer/formats.h"
#include "render_target.h"

namespace impeller {

enum class RenderTargetType { Offscreen, OffscreenMSAA, kUnknown };

class RenderTargetBuilder {
 private:
  ISize size_;
  std::string label_;

  StorageMode color_storage_mode_;
  LoadAction color_load_action_;
  StoreAction color_store_action_;

  StorageMode stencil_storage_mode_;
  LoadAction stencil_load_action_;
  StoreAction stencil_store_action_;

  StorageMode color_resolve_storage_mode_;

  RenderTargetType render_target_type_;

  RenderTarget CreateOffscreen(const Context& context);

  RenderTarget CreateOffscreenMSAA(const Context& context);

 public:
  RenderTargetBuilder();

  ~RenderTargetBuilder();

  RenderTargetBuilder& setSize(ISize size) {
    size_ = size;
    return *this;
  }

  RenderTargetBuilder& setLabel(std::string label) {
    label_ = label;
    return *this;
  }

  RenderTargetBuilder& setColorStorageMode(StorageMode mode) {
    color_storage_mode_ = mode;
    return *this;
  }

  RenderTargetBuilder& setColorLoadAction(LoadAction action) {
    color_load_action_ = action;
    return *this;
  }

  RenderTargetBuilder& setColorStoreAction(StoreAction action) {
    color_store_action_ = action;
    return *this;
  }

  RenderTargetBuilder& setStencilStorageMode(StorageMode mode) {
    stencil_storage_mode_ = mode;
    return *this;
  }

  RenderTargetBuilder& setStencilLoadAction(LoadAction action) {
    stencil_load_action_ = action;
    return *this;
  }

  RenderTargetBuilder& setStencilStoreAction(StoreAction action) {
    stencil_store_action_ = action;
    return *this;
  }

  RenderTargetBuilder& setColorResolveStorageMode(StorageMode mode) {
    color_resolve_storage_mode_ = mode;
    return *this;
  }

  RenderTargetBuilder& setRenderTargetType(RenderTargetType type) {
    render_target_type_ = type;
    return *this;
  }

  RenderTarget build(const Context& context);
};

}  // namespace impeller