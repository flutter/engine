// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart.ui;

abstract class PluginUtilities {
  static Function getClosureByName({String name, Uri libraryUri}) {
    if (name == null) {
      throw new ArgumentError("'name' must be provided.");
    }
    return _lookupClosure(name, libraryUri?.toString());
  }

  static String getPathForFunctionLibrary(Function closure) {
    if (closure == null) {
      throw new ArgumentError("'closure' cannot be null.");
    }
    return _lookupClosureLibrary(closure);
  }

  static String getNameOfFunction(Function closure) {
  	if (closure == null) {
  		throw new ArgumentError("'closure' cannot be null.");
  	}
  	return _getFunctionName(closure);
  }
}
