// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "lib/fostr/fidl/fuchsia.ui.gfx/amendments.h"

#include <iostream>

namespace fuchsia {
namespace ui {
namespace gfx {

std::ostream& operator<<(std::ostream& stream, const fuchsia::ui::gfx::Value::Tag& tag) {
  using Value = fuchsia::ui::gfx::Value;
  switch (tag) {
    case Value::Tag::kVector1:
      return stream << "vec1";
    case Value::Tag::kVector2:
      return stream << "vec2";
    case Value::Tag::kVector3:
      return stream << "vec3";
    case Value::Tag::kVector4:
      return stream << "vec4";
    case Value::Tag::kMatrix4x4:
      return stream << "mat4";
    case Value::Tag::kColorRgb:
      return stream << "rgb";
    case Value::Tag::kColorRgba:
      return stream << "rgba";
    case Value::Tag::kDegrees:
      return stream << "degrees";
    case Value::Tag::kQuaternion:
      return stream << "quat";
    case Value::Tag::kTransform:
      return stream << "transform";
    case Value::Tag::kVariableId:
      return stream << "variable";
    case Value::Tag::Invalid:
      return stream << "Invalid";
  }
}

}  // namespace gfx
}  // namespace ui
}  // namespace fuchsia
