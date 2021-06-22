// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

library fuchsia_fakes;

import 'dart:io' as io;

import 'package:zircon/zircon.dart';

// ignore_for_file: prefer_constructors_over_static_methods, public_member_api_docs

class MxStartupInfo {
  static Handle takeEnvironment() => Handle.invalid();

  static Handle takeOutgoingServices() => Handle.invalid();

  static Handle takeViewRef() => Handle.invalid();
}

void exit(int returnCode) {
  io.exit(returnCode);
}
