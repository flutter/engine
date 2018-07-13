// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.view;

public final class FlutterCallbackInformation {
  final public String callbackName;
  final public String callbackClassName;
  final public String callbackLibraryPath;

  public static FlutterCallbackInformation lookupCallbackInformation(long handle) {
    return nativeLookupCallbackInformation(handle);
  }

  private FlutterCallbackInformation(String callbackName, String callbackClassName, String callbackLibraryPath) {
    this.callbackName = callbackName;
    this.callbackClassName = callbackClassName;
    this.callbackLibraryPath = callbackLibraryPath;
  }

  private static native FlutterCallbackInformation nativeLookupCallbackInformation(long handle);
}
