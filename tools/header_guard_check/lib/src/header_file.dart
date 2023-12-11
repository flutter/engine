// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:meta/meta.dart';
import 'package:path/path.dart' as p;

/// Represents a C++ header file, i.e. a file on disk that ends in `.h`.
@immutable
final class HeaderFile {
  /// Path to the file on disk.
  final String path;

  const HeaderFile(this.path);
}
