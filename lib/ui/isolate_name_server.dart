// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart.ui;

abstract class IsolateNameServer {
  // Looks up the [SendPort] associated with a given name. Returns null
  // if the name does not exist.
  static SendPort lookupPortByName(String name) =>
      _lookupPortByNameNative(name);

  // Registers a SendPort with a given name. Returns true if registration is
  // successful, false if the name entry already exists.
  static bool registerPortWithName(SendPort port, String name) {
    if (port == null) {
      throw new ArgumentError("'port' cannot be null.");
    }
    return _registerPortWithNameNative(port, name);
  }

  // Removes a name to SendPort mapping given a name. Returns true if the
  // mapping was successfully removed, false if the mapping does not exist.
  static bool removePortNameMapping(String name) =>
      _removePortNameMappingNative(name);

  static SendPort _lookupPortByNameNative(String name)
      native 'IsolateNameServerNatives_LookupPortByName';
  static bool _registerPortWithNameNative(SendPort port, String name)
      native 'IsolateNameServerNatives_RegisterPortWithName';
  static bool _removePortNameMappingNative(String name)
      native 'IsolateNameServerNatives_RemovePortNameMapping';
}
