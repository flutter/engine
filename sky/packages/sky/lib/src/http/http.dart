// Copyright (c) 2013, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

/// A [Future]-based library for making HTTP requests. It's based on
/// Dart's `http` package, but we've removed the dependency on mirrors
/// and added a `mojo`-based HTTP client.

import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';

import 'mojo_client.dart';
import 'response.dart';

/// Sends an HTTP HEAD request with the given headers to the given URL, which
/// can be a [Uri] or a [String].
Future<Response> head(url) =>
  _withClient((client) => client.head(url));

/// Sends an HTTP GET request with the given headers to the given URL, which can
/// be a [Uri] or a [String].
Future<Response> get(url) =>
  _withClient((client) => client.get(url));

/// Sends an HTTP POST request with the given headers and body to the given URL,
/// which can be a [Uri] or a [String].
///
/// [body] sets the body of the request.
Future<Response> post(url, { String body }) =>
  _withClient((client) => client.post(url, body: body));

/// Sends an HTTP PUT request with the given headers and body to the given URL,
/// which can be a [Uri] or a [String].
///
/// [body] sets the body of the request.
Future<Response> put(url, { String body }) =>
  _withClient((client) => client.put(url, body: body));

/// Sends an HTTP PATCH request with the given headers and body to the given
/// URL, which can be a [Uri] or a [String].
///
/// [body] sets the body of the request.
Future<Response> patch(url, { String body }) =>
  _withClient((client) => client.patch(url, body: body));

/// Sends an HTTP DELETE request with the given headers to the given URL, which
/// can be a [Uri] or a [String].
Future<Response> delete(url) =>
  _withClient((client) => client.delete(url));

/// Sends an HTTP GET request with the given headers to the given URL, which can
/// be a [Uri] or a [String], and returns a Future that completes to the body of
/// the response as a [String].
///
/// The Future will emit a [ClientException] if the response doesn't have a
/// success status code.
Future<String> read(url) =>
  _withClient((client) => client.read(url));

/// Sends an HTTP GET request with the given headers to the given URL, which can
/// be a [Uri] or a [String], and returns a Future that completes to the body of
/// the response as a list of bytes.
///
/// The Future will emit a [ClientException] if the response doesn't have a
/// success status code.
Future<Uint8List> readBytes(url) =>
  _withClient((client) => client.readBytes(url));

Future _withClient(Future fn(MojoClient)) {
  var client = new MojoClient();
  var future = fn(client);
  return future.whenComplete(client.close);
}
