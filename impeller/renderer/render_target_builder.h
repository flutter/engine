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

enum class RenderTargetType { kOffscreen, kOffscreenMSAA, kUnknown };

class RenderTargetBuilder {
 public:
  RenderTargetBuilder();

  ~RenderTargetBuilder();

  RenderTargetBuilder& SetSize(ISize size) {
    size_ = size;
    return *this;
  }

  RenderTargetBuilder& SetLabel(std::string label) {
    label_ = label;
    return *this;
  }

  RenderTargetBuilder& SetColorStorageMode(StorageMode mode) {
    color_storage_mode_ = mode;
    return *this;
  }

  RenderTargetBuilder& SetColorLoadAction(LoadAction action) {
    color_load_action_ = action;
    return *this;
  }

  RenderTargetBuilder& SetColorStoreAction(StoreAction action) {
    color_store_action_ = action;
    return *this;
  }

  RenderTargetBuilder& SetStencilStorageMode(StorageMode mode) {
    stencil_storage_mode_ = mode;
    return *this;
  }

  RenderTargetBuilder& SetStencilLoadAction(LoadAction action) {
    stencil_load_action_ = action;
    return *this;
  }

  RenderTargetBuilder& SetStencilStoreAction(StoreAction action) {
    stencil_store_action_ = action;
    return *this;
  }

  RenderTargetBuilder& SetColorResolveStorageMode(StorageMode mode) {
    color_resolve_storage_mode_ = mode;
    return *this;
  }

  RenderTargetBuilder& SetRenderTargetType(RenderTargetType type) {
    render_target_type_ = type;
    return *this;
  }

  RenderTarget Build(const Context& context) const;

 private:
  ISize size_;
  std::string label_;

  StorageMode color_storage_mode_ = StorageMode::kDeviceTransient;
  LoadAction color_load_action_ = LoadAction::kDontCare;
  StoreAction color_store_action_ = StoreAction::kDontCare;

  StorageMode stencil_storage_mode_ = StorageMode::kDeviceTransient;
  LoadAction stencil_load_action_ = LoadAction::kDontCare;
  StoreAction stencil_store_action_ = StoreAction::kDontCare;

  StorageMode color_resolve_storage_mode_ = StorageMode::kDeviceTransient;

  RenderTargetType render_target_type_ = RenderTargetType::kUnknown;

  RenderTarget CreateOffscreen(const Context& context) const;

  RenderTarget CreateOffscreenMSAA(const Context& context) const;

  FML_DISALLOW_COPY_AND_ASSIGN(RenderTargetBuilder);
};

}  // namespace impeller
