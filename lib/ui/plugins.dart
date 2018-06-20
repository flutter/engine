// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart.ui;

abstract class PluginUtilities {
  static Function getClosureByName({String name, Uri libraryUri, String className}) {
    if (name == null) {
      throw new ArgumentError.notNull('name');
    }
    return _lookupClosure(name, libraryUri?.toString(), className);
  }

  static String getPathForFunctionLibrary(Function closure) {
    if (closure == null) {
      throw new ArgumentError.notNull('closure');
    }
    return _getFunctionLibraryUrl(closure);
  }

  static String getNameOfFunction(Function closure) {
    if (closure == null) {
      throw new ArgumentError.notNull('closure');
    }
    return _getFunctionName(closure);
  }

  static String getNameOfFunctionClass(Function closure) {
    if(closure == null) {
      throw new ArgumentError.notNull('closure');
    }
    return _getNameOfFunctionClass(closure);
  }
}
