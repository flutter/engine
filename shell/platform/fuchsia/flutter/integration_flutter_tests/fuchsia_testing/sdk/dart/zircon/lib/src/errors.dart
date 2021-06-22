// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of zircon;

// ignore_for_file: public_member_api_docs

// TODO: ZirconApiError should be an exception.
class ZirconApiError extends Error {
  final String message;
  ZirconApiError(this.message) : super();

  @override
  String toString() => message;
}
