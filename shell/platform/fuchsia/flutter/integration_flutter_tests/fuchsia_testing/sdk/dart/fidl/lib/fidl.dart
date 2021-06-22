// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// The FIDL Dart package which provides mechanisms for interacting with the
/// FIDL IPC system. This package is a dependency of any Proxy or Binding
/// classes implemented by generated FIDL code. It is often used directly in
/// author code to retrieve type definitions (e.g. InterfaceHandle,
/// InterfaceRequest, etc.) for interacting with certain FIDL services.
export 'src/bits.dart';
export 'src/codec.dart';
export 'src/enum.dart';
export 'src/error.dart';
export 'src/interface.dart';
export 'src/interface_async.dart';
export 'src/message.dart';
export 'src/struct.dart';
export 'src/table.dart';
export 'src/types.dart';
export 'src/unknown_data.dart';
export 'src/xunion.dart';
