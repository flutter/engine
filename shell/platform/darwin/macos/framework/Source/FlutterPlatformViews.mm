// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace flutter {
void FlutterPlatformViewsControllerMacOS::OnMethodCall(FlutterMethodCall* call, FlutterResult& result) {
  if ([[call method] isEqualToString:@"create"]) {
      NSLog(@"create noop");
  } else if ([[call method] isEqualToString:@"dispose"]) {
      NSLog(@"dispose noop");
  } else if ([[call method] isEqualToString:@"acceptGesture"]) {
      NSLog(@"noop");
  } else if ([[call method] isEqualToString:@"rejectGesture"]) {
      NSLog(@"noop");
  } else {
      NSLog(@"noop");
  }
}

}  // namespace flutter