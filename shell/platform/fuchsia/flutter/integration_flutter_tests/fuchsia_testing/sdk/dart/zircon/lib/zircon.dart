// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// The Zircon Dart package contains several libraries required to support
/// Fuchsia’s lower layer features, E.g. Zircon channels are used for the
/// transport of the FIDL IPC system. Requiring this package directly should be
/// rare for most mod or agent authors.
library zircon;

import 'dart:async';
import 'dart:typed_data';

import 'src/fakes/zircon_fakes.dart' if (dart.library.zircon) 'dart:zircon';

export 'src/fakes/zircon_fakes.dart' if (dart.library.zircon) 'dart:zircon';

part 'src/channel.dart';
part 'src/channel_reader.dart';
part 'src/constants.dart';
part 'src/errors.dart';
part 'src/eventpair.dart';
part 'src/handle_wrapper.dart';
part 'src/socket.dart';
part 'src/socket_reader.dart';
part 'src/vmo.dart';
