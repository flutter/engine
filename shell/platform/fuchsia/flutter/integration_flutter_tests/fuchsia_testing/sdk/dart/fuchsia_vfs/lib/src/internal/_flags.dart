// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:fidl_fuchsia_io/fidl_async.dart';

/// Common utilities for working with flags during open/clone.
class Flags {
  /// All known rights.
  static const int fsRights =
      openRightReadable | openRightWritable | openRightAdmin;

  /// All lower 16 bits are reserved for future rights extensions.
  static const int fsRightsSpace = 0x0000FFFF;

  /// Returns true if the OPEN_FLAG_NODE_REFERENCE bit is set in |flags|.
  static bool isNodeReference(int flags) {
    return (flags & openFlagNodeReference) != 0;
  }

  /// Returns true if the CLONE_FLAG_SAME_RIGHTS bit is set in |flags|.
  static bool shouldCloneWithSameRights(int flags) {
    return (flags & cloneFlagSameRights) != 0;
  }

  /// Returns true if the OPEN_FLAG_POSIX bit is set in |flags|.
  static bool isPosix(int flags) {
    return (flags & openFlagPosix) != 0;
  }

  /// Returns true if the rights flags in |flagsA| does not exceed
  /// those in |flagsB|.
  static bool stricterOrSameRights(int flagsA, int flagsB) {
    var rightsA = flagsA & fsRights;
    var rightsB = flagsB & fsRights;
    return (rightsA & ~rightsB) == 0;
  }

  /// Perform basic flags validation relevant to |Directory.Open| and
  /// |Node.Clone|.
  /// Returns false if the flags combination is invalid.
  static bool inputPrecondition(int flags) {
    // If the caller specified an unknown right, reject the request.
    if (((flags & fsRightsSpace) & ~fsRights) != 0) {
      return false;
    }
    // Explicitly reject NODE_REFERENCE together with any invalid flags.
    if ((flags & openFlagNodeReference) != 0) {
      const int validFlagsForNodeRef = openFlagNodeReference |
          openFlagDirectory |
          openFlagNotDirectory |
          openFlagDescribe;
      if ((flags & ~validFlagsForNodeRef) != 0) {
        return false;
      }
    }
    return true;
  }
}
