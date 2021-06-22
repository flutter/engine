// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:fidl_fuchsia_ui_views/fidl_async.dart';
import 'package:fuchsia/fuchsia.dart';
import 'package:zircon/zircon.dart';

// ignore: avoid_classes_with_only_static_members
class ScenicContext {
  static Handle? _raw;

  // Gets a duplicate of the host [ViewRef] handle for the component.
  static ViewRef hostViewRef() => ViewRef(
      reference: EventPair((_raw ??= MxStartupInfo.takeViewRef())
          .duplicate(ZX.RIGHT_SAME_RIGHTS)));
}
