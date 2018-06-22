// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_SEMANTICS_LOCAL_CONTEXT_ACTION_UPDATE_BUILDER_H_
#define FLUTTER_LIB_UI_SEMANTICS_LOCAL_CONTEXT_ACTION_UPDATE_BUILDER_H_

#include "flutter/lib/ui/semantics/local_context_action.h"
#include "flutter/lib/ui/semantics/local_context_action_update.h"
#include "flutter/lib/ui/semantics/semantics_update.h"
#include "lib/tonic/dart_wrappable.h"
#include "lib/tonic/dart_list.h"
#include "lib/tonic/typed_data/float64_list.h"
#include "lib/tonic/typed_data/int32_list.h"

namespace blink {

class LocalContextActionUpdateBuilder
    : public fxl::RefCountedThreadSafe<LocalContextActionUpdateBuilder>,
      public tonic::DartWrappable {
  DEFINE_WRAPPERTYPEINFO();
  FRIEND_MAKE_REF_COUNTED(LocalContextActionUpdateBuilder);

 public:
  static fxl::RefPtr<LocalContextActionUpdateBuilder> create() {
    return fxl::MakeRefCounted<LocalContextActionUpdateBuilder>();
  }

  ~LocalContextActionUpdateBuilder() override;

  void updateAction(int id,
                    int textDirection,
                    std::string label);

  fxl::RefPtr<LocalContextActionUpdate> build();

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

 private:
  explicit LocalContextActionUpdateBuilder();

  LocalContextActionUpdates actions_;
};

}  // namespace blink

#endif  // FLUTTER_LIB_UI_SEMANTICS_LOCAL_CONTEXT_ACTION_UPDATE_BUILDER_H_
