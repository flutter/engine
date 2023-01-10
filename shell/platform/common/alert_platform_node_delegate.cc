// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "alert_platform_node_delegate.h"

namespace flutter {

AlertPlatformNodeDelegate::AlertPlatformNodeDelegate(ui::AXPlatformNodeDelegate* parent_delegate) : parent_delegate_(parent_delegate) {
  data_.role = ax::mojom::Role::kAlert;
  data_.id = id_.Get();
}

AlertPlatformNodeDelegate::~AlertPlatformNodeDelegate() {}

gfx::AcceleratedWidget AlertPlatformNodeDelegate::GetTargetForNativeAccessibilityEvent() {
  if (parent_delegate_) {
    return parent_delegate_->GetTargetForNativeAccessibilityEvent();
  }
  return nullptr;
}

gfx::NativeViewAccessible AlertPlatformNodeDelegate::GetParent() {
  if (parent_delegate_) {
    return parent_delegate_->GetNativeViewAccessible();
  }
  return nullptr;
}

const ui::AXUniqueId& AlertPlatformNodeDelegate::GetUniqueId() const {
  return id_;
}

const ui::AXNodeData& AlertPlatformNodeDelegate::GetData() const {
  return data_;
}

void AlertPlatformNodeDelegate::SetText(std::u16string text) {
  data_.SetName(text);
  data_.SetDescription(text);
  data_.SetValue(text);
}

}
