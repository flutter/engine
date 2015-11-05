// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';

import 'package:mojo/core.dart' as mojo;
import 'package:mojo/mojo/http_header.mojom.dart' as mojo;
import 'package:mojo/mojo/url_request.mojom.dart' as mojo;
import 'package:mojo/mojo/url_response.mojom.dart' as mojo;
import 'package:mojo_services/mojo/network_service.mojom.dart' as mojo;
import 'package:mojo_services/mojo/url_loader.mojom.dart' as mojo;

import 'package:flutter/src/services/shell.dart';

import 'response.dart';

mojo.NetworkServiceProxy _initNetworkService() {
  mojo.NetworkServiceProxy networkService = new mojo.NetworkServiceProxy.unbound();
  shell.connectToService("mojo:authenticated_network_service", networkService);
  return networkService;
}

final mojo.NetworkServiceProxy _networkService = _initNetworkService();

Future<mojo.UrlResponse> fetchUrl(String url, { String method: "GET", String body }) async {
  mojo.UrlLoaderProxy loader = new mojo.UrlLoaderProxy.unbound();
  mojo.UrlRequest request = new mojo.UrlRequest()
    ..url = Uri.base.resolve(url).toString()
    ..method = method
    ..autoFollowRedirects = true;
  if (body != null) {
    mojo.MojoDataPipe pipe = new mojo.MojoDataPipe();
    request.body = <mojo.MojoDataPipeConsumer>[pipe.consumer];
    ByteData data = new ByteData.view(UTF8.encode(body).buffer);
    mojo.DataPipeFiller.fillHandle(pipe.producer, data);
  }
  try {
    _networkService.ptr.createUrlLoader(loader);
    return (await loader.ptr.start(request)).response;
  } catch (e) {
    print("NetworkService unavailable $e");
    return null;
  } finally {
    loader.close();
  }
}

/// A `mojo`-based HTTP client
class MojoClient {

  Future<Response> head(url) => _send("HEAD", url);

  Future<Response> get(url) => _send("GET", url);

  Future<Response> post(url, { String body }) => _send("POST", url, body);

  Future<Response> put(url, { String body }) => _send("PUT", url, body);

  Future<Response> patch(url, { String body }) => _send("PATCH", url, body);

  Future<Response> delete(url) => _send("DELETE", url);

  Future<String> read(url) {
    return get(url).then((response) {
      _checkResponseSuccess(url, response);
      return response.body;
    });
  }

  Future<Uint8List> readBytes(url) {
    return get(url).then((response) {
      _checkResponseSuccess(url, response);
      return response.bodyBytes;
    });
  }

  Future<Response> _send(String method, url, [ body ]) async {
    mojo.UrlResponse response = await fetchUrl(url, method: method, body: body);
    ByteData data = await mojo.DataPipeDrainer.drainHandle(response.body);
    Uint8List bodyBytes = new Uint8List.view(data.buffer);
    return new Response(bodyBytes, response.statusCode);
  }

  void _checkResponseSuccess(url, Response response) {
    if (response.statusCode < 400) return;
    var message = "Request to $url failed with status ${response.statusCode}";
    if (response.status_line != null) {
      message = "$message: ${response.status_line}";
    }
    if (url is String) url = Uri.parse(url);
    throw new Exception("$message.", url);
  }

  void close() {}
}
