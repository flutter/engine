// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:fidl_fuchsia_ui_views/fidl_async.dart';
import 'package:zircon/zircon.dart';

/// Helper object representing a View/ViewHolder token pair.
class ViewTokenPair {
  /// Token for a Scenic |View|.
  final ViewToken viewToken;

  /// Token for a Scenic |ViewHolder|.
  final ViewHolderToken viewHolderToken;

  /// Constructor.
  ViewTokenPair() : this._fromTokens(EventPairPair());

  /// Helper constructor to create from an |EventPairPair| of tokens.
  ViewTokenPair._fromTokens(EventPairPair tokens)
      : assert(tokens.status == ZX.OK),
        viewToken = ViewToken(value: tokens.first!),
        viewHolderToken = ViewHolderToken(value: tokens.second!);
}
