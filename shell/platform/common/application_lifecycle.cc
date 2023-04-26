// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/common/application_lifecycle.h"

#include <cassert>
#include <string>

namespace flutter {

const char* AppLifecycleStateToString(AppLifecycleState state) {
  switch (state) {
    case kAppLifecycleStateDetached:
      return "AppLifecycleState.detached";
    case kAppLifecycleStateResumed:
      return "AppLifecycleState.resumed";
    case kAppLifecycleStateInactive:
      return "AppLifecycleState.inactive";
    case kAppLifecycleStateHidden:
      return "AppLifecycleState.hidden";
    case kAppLifecycleStatePaused:
      return "AppLifecycleState.paused";
    default:
      assert(false && "Lifecycle state not recognized");
      break;
  }
}

AppLifecycleState StringToAppLifecycleState(const char* char_value) {
  std::string value = char_value;
  if (value == "AppLifecycleState.detached") {
    return kAppLifecycleStateDetached;
  }
  if (value == "AppLifecycleState.resumed") {
    return kAppLifecycleStateResumed;
  }
  if (value == "AppLifecycleState.inactive") {
    return kAppLifecycleStateInactive;
  }
  if (value == "AppLifecycleState.hidden") {
    return kAppLifecycleStateHidden;
  }
  if (value == "AppLifecycleState.paused") {
    return kAppLifecycleStatePaused;
  }
  assert(false && "Lifecycle state string not recognized");
  return kAppLifecycleStateDetached;
}

}  // namespace flutter
