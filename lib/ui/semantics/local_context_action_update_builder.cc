// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/semantics/local_context_action_update_builder.h"

#include "lib/tonic/converter/dart_converter.h"
#include "lib/tonic/dart_args.h"
#include "lib/tonic/dart_binding_macros.h"
#include "lib/tonic/dart_library_natives.h"

namespace blink {

static void LocalContextActionUpdateBuilder_constructor(Dart_NativeArguments args) {
  DartCallConstructor(&LocalContextActionUpdateBuilder::create, args);
}

IMPLEMENT_WRAPPERTYPEINFO(ui, LocalContextActionUpdateBuilder);

#define FOR_EACH_BINDING(V)             \
  V(LocalContextActionUpdateBuilder, updateAction) \
  V(LocalContextActionUpdateBuilder, build)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void LocalContextActionUpdateBuilder::RegisterNatives(
    tonic::DartLibraryNatives* natives) {
  natives->Register({{"LocalContextActionUpdateBuilder_constructor",
                     LocalContextActionUpdateBuilder_constructor, 1, true},
                     FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

LocalContextActionUpdateBuilder::LocalContextActionUpdateBuilder() = default;

LocalContextActionUpdateBuilder::~LocalContextActionUpdateBuilder() = default;

void LocalContextActionUpdateBuilder::updateAction(int id,
                                          int textDirection,
                                          std::string label) {
  LocalContextAction action;
  action.id = id;
  action.textDirection = textDirection;
  action.label = label;
  actions_[id] = action;
}

fxl::RefPtr<LocalContextActionUpdate> LocalContextActionUpdateBuilder::build() {
  return LocalContextActionUpdate::create(std::move(actions_));
}

}  // namespace blink
