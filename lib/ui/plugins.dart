// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart.ui;

/// Functionality for Flutter plugin authors.
abstract class PluginUtilities {
  /// Get a closure instance for a specific static or top-level function given
  /// a name.
  ///
  /// **Warning:** if the function corresponding with `name` is not referenced
  /// explicitly (e.g., invoked, passed as a closure, etc.) it may be optimized
  /// out of the compiled application. To prevent this, avoid hardcoding names
  /// as [String]s and  prefer to use `PluginUtilities.getNameOfFunction` to
  /// retrieve the function's name.
  ///
  /// `name` is the name of the function we want a closure for. This must not
  /// be null.
  ///
  /// `libraryPath` is the path which points to the library which contains the
  /// function of interest. This path can be retrieved using
  /// [PluginUtilities.getPathForFunctionLibrary]. If not specified, the root
  /// library will be used for the function lookup.
  ///
  /// `className` is the name of the class in which function `name` is defined.
  /// This parameter is required for retrieving closures for static methods. If
  /// not provided, `name` is assumed to be referring to a top-level function.
  ///
  /// Returns a closure for `name` as a [Function] if the lookup was successful.
  /// Otherwise, null is returned on error.
  static Function getClosureByName({String name, String libraryPath, String className}) {
    assert(name != null, "'name' must not be null.");
    return _lookupClosure(name, libraryPath, className);
  }

  /// Get the path for the library which contains a given method.
  ///
  /// `closure` is the closure of the function that we want to find the library
  /// path for. This must not be null.
  ///
  /// Returns a [String] representing the path to the library which contains
  /// `closure`. Returns null if an error occurs or `closure` is not found.
  static String getPathForFunctionLibrary(Function closure) {
    assert(closure != null, "'closure' must not be null.");
    return _getFunctionLibraryUrl(closure);
  }

  /// Get the name of a [Function] as a [String].
  /// 
  /// `closure` must not be null.
  ///
  /// Returns a [String] representing the name of the function if the function
  /// exists, otherwise null is returned.
  static String getNameOfFunction(Function closure) {
    assert(closure != null, "'closure' must not be null.");
    return _getFunctionName(closure);
  }

  /// Get the name of a class containing [Function] as a [String].
  /// 
  /// `closure` must not be null.
  ///
  /// Returns a [String] representing the name of the function if the function
  /// exists and is a member of a class, otherwise null is returned.
  static String getNameOfFunctionClass(Function closure) {
    assert(closure != null, "'closure' must not be null.");
    return _getNameOfFunctionClass(closure);
  }
}
