// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
import 'dart:convert';

import 'utils.dart';

class MacOSInfo {
  Future<void> collectInformation() async {
    final String systemProfileJson = await evalProcess(
        'system_profiler', ['SPApplicationsDataType', '-json']);

    final Map<String, dynamic> json =
        jsonDecode(systemProfileJson) as Map<String, dynamic>;
    final List<dynamic> systemProfile = json.values.first as List<dynamic>;
    for (int i = 0; i < systemProfile.length; i++) {
      final Map<String, dynamic> application =
          systemProfile[i] as Map<String, dynamic>;
      final String applicationName = application['_name'] as String;
      if (applicationName.contains('Safari')) {
        print('application: $applicationName '
            'fullInfo: ${application.toString()}');
        print('version: ${application['version']}');
      }
    }
  }
}
