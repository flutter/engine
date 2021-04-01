// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/semantics/attributed_string.h"

#include "flutter/fml/logging.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_binding_macros.h"

#include <memory>

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, AttributedString);

#define FOR_EACH_BINDING(V) V(AttributedString, initAttributedString)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void AttributedString::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register({FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

AttributedString::AttributedString() {}

AttributedString::~AttributedString() {}

void AttributedString::initAttributedString(
    Dart_Handle attributed_string_handle,
    std::string string,
    std::vector<NativeStringAttribute*> attributes) {
  UIDartState::ThrowIfUIOperationsProhibited();

  auto attributed_string = fml::MakeRefCounted<AttributedString>();
  attributed_string->AssociateWithDartWrapper(attributed_string_handle);

  attributed_string->string_ = string;
  for (auto& native_attribute : attributes) {
    attributed_string->attributes_.push_back(native_attribute->attribute_);
  }
}

const std::string& AttributedString::GetString() {
  return string_;
}

const StringAttributes& AttributedString::GetAttributes() {
  return attributes_;
}

}  // namespace flutter
