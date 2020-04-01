// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:html' as html;
import 'dart:js';
import 'dart:js_util' as js_util;

class MobileUnitTestBindings {
  final Completer<bool> _allTestsPassed = Completer<bool>();

  void tearDown() {
    if (!_allTestsPassed.isCompleted) {
      _allTestsPassed.complete(true);
    }
  }

  void initialize() {
    Future<Map<String, dynamic>> callback(Map<String, String> params) async {
      final String command = params['command'];
      Map<String, String> response;
      switch (command) {
        case 'fetch_test_results':
          final bool allTestsPassedFlag = await _allTestsPassed.future;
          response = <String, String>{
            // TODO(nurhan): Send a more detailed error message. We can
            // use the same method we used in e2e package.
            'message': allTestsPassedFlag ? 'pass' : 'fail',
          };
          break;
        default:
          throw UnimplementedError('$command is not implemented');
      }
      return <String, dynamic>{
        'isError': false,
        'response': response,
      };
    }

    // Registers Web Service Extension for unit tests which will be opened in
    // the mobile browsers.
    js_util.setProperty(html.window, '\$feltDriver',
        allowInterop((dynamic message) async {
      final Map<String, String> params = Map<String, String>.from(
          jsonDecode(message as String) as Map<String, dynamic>);
      final Map<String, dynamic> result =
          Map<String, dynamic>.from(await callback(params));
      context['\$feltDriverResult'] = json.encode(result);
    }));
  }
}
