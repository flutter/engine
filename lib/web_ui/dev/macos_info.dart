// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
import 'dart:convert';

import 'utils.dart';

class MacOSInfo {
  /// Default Safari Version.
  String safariVersion = 'Safari 12';

  Future<void> collectInformation() async {
    final String systemProfileJson = await evalProcess(
        'system_profiler', ['SPApplicationsDataType', '-json']);
    print('INFO: system_profiler run, information on Safari');

    Map<String, dynamic> json = jsonDecode(systemProfileJson);
    final List<dynamic> systemProfile = json.values.first;
    for (int i = 0; i < systemProfile.length; i++) {
      final Map<String, dynamic> application = systemProfile[i];
      final String applicationName = application['_name'];
      if (applicationName.contains('Safari')) {
        print('application: $applicationName '
            'fullInfo: ${application.toString()}');
        print('version: ${application['version']}');
      }
    }
  }
}
