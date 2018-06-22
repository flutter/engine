// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_SEMANTICS_LOCAL_CONTEXT_ACTION_H_
#define FLUTTER_LIB_UI_SEMANTICS_LOCAL_CONTEXT_ACTION_H_

#include "lib/tonic/dart_wrappable.h"
#include "lib/tonic/typed_data/float64_list.h"
#include "lib/tonic/typed_data/int32_list.h"
#include "lib/tonic/dart_library_natives.h"

namespace blink {

struct LocalContextAction {
  LocalContextAction();
  ~LocalContextAction();

  int32_t id = 0;
  int32_t textDirection = 0;  // 0=unknown, 1=rtl, 2=ltr
  std::string label;
};


// Contains local context actions that need to be updated.
//
// The keys in the map are stable action IDs, and the values contain
// semantic information for the action corresponding to that id.
using LocalContextActionUpdates = std::unordered_map<int32_t, LocalContextAction>;

}  // namespace blink

#endif  //FLUTTER_LIB_UI_SEMANTICS_LOCAL_CONTEXT_ACTION_H_
