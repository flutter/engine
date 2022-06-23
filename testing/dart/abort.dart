// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';

/// This file is used to test flutter_tester --run-forever
void main() {
  print('Sending SIGABRT');
  Process.killPid(pid, ProcessSignal.sigabrt);
}
