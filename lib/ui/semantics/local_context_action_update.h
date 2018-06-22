// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_SEMANTICS_LOCAL_CONTEXT_ACTION_UPDATE_H_
#define FLUTTER_LIB_UI_SEMANTICS_LOCAL_CONTEXT_ACTION_UPDATE_H_

#include "flutter/lib/ui/semantics/local_context_action.h"
#include "lib/tonic/dart_wrappable.h"

namespace blink {

class LocalContextActionUpdate : public fxl::RefCountedThreadSafe<LocalContextActionUpdate>,
                                 public tonic::DartWrappable {
  DEFINE_WRAPPERTYPEINFO();
  FRIEND_MAKE_REF_COUNTED(LocalContextActionUpdate);

 public:
  ~LocalContextActionUpdate() override;
  static fxl::RefPtr<LocalContextActionUpdate> create(LocalContextActionUpdates actions);

  LocalContextActionUpdates takeActions();

  void dispose();

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

 private:
  explicit LocalContextActionUpdate(LocalContextActionUpdates actions);

  LocalContextActionUpdates  actions_;
};

}  // namespace blink

#endif  // FLUTTER_LIB_UI_SEMANTICS_LOCAL_CONTEXT_ACTION_UPDATE_H_
