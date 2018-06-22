// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/semantics/local_context_action_update_builder.h"
#include "flutter/lib/ui/semantics/local_context_action_update.h"

#include <memory>

#include "flutter/lib/ui/painting/matrix.h"
#include "lib/tonic/converter/dart_converter.h"
#include "lib/tonic/dart_args.h"
#include "lib/tonic/dart_binding_macros.h"
#include "lib/tonic/dart_library_natives.h"

namespace blink {

IMPLEMENT_WRAPPERTYPEINFO(ui, LocalContextActionUpdate);

#define FOR_EACH_BINDING(V) V(LocalContextActionUpdate, dispose)

DART_BIND_ALL(LocalContextActionUpdate, FOR_EACH_BINDING)

fxl::RefPtr<LocalContextActionUpdate> LocalContextActionUpdate::create(
    LocalContextActionUpdates actions) {
  return fxl::MakeRefCounted<LocalContextActionUpdate>(std::move(actions));
}

LocalContextActionUpdate::LocalContextActionUpdate(LocalContextActionUpdates actions)
    : actions_(std::move(actions)) {}

LocalContextActionUpdate::~LocalContextActionUpdate() = default;

LocalContextActionUpdates LocalContextActionUpdate::takeActions() {
  return std::move(actions_);
}

void LocalContextActionUpdate::dispose() {
  ClearDartWrapper();
}

}  // namespace blink
