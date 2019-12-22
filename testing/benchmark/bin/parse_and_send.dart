// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:io';

import 'package:git/git.dart';
import 'package:metrics_center/flutter.dart';

const String kNameKey = 'name';
const String kTimeUnitKey = 'time_unit';
const List<String> kNonNumericalValueSubResults = <String>[
  kNameKey,
  kTimeUnitKey,
  'iterations',
  'big_o',
];

Future<String> _getGitRevision() async {
  final GitDir gitDir = await GitDir.fromExisting('../../');
  // Somehow gitDir.currentBranch() doesn't work in Cirrus with "fatal: 'HEAD' -
  // not a valid ref". Therefore, we use "git log" to get the revision manually.
  final ProcessResult logResult =
      await gitDir.runCommand(<String>['log', '--pretty=format:%H', '-n', '1']);
  if (logResult.exitCode != 0) {
    throw 'Unexpected exit code ${logResult.exitCode}';
  }
  return logResult.stdout.toString();
}

void _parseAnItem(
  Map<String, dynamic> item,
  List<FlutterEngineMetricPoint> points,
  Map<String, String> context,
  String gitRevision,
) {
  final String name = item[kNameKey];
  final Map<String, String> timeUnitMap = <String, String>{
    kUnitKey: item[kTimeUnitKey]
  };
  for (String subResult in item.keys) {
    if (!kNonNumericalValueSubResults.contains(subResult)) {
      num rawValue;
      try {
        rawValue = item[subResult];
      } catch (e) {
        print('$subResult: ${item[subResult]} (${item[subResult].runtimeType}) '
            'is not a number');
        rethrow;
      }

      final double value = rawValue is int ? rawValue.toDouble() : rawValue;
      points.add(
        FlutterEngineMetricPoint(name, value, gitRevision,
            moreTags: <String, String>{kSubResultKey: subResult}
              ..addAll(context)
              ..addAll(subResult.endsWith('time')
                  ? timeUnitMap
                  : <String, String>{})),
      );
    }
  }
}

Future<List<FlutterEngineMetricPoint>> _parse(String jsonFileName) async {
  final String gitRevision = await _getGitRevision();

  final Map<String, dynamic> jsonResult =
      jsonDecode(File(jsonFileName).readAsStringSync());

  final Map<String, dynamic> rawContext = jsonResult['context'];
  final Map<String, String> context = rawContext.map<String, String>(
    (String k, dynamic v) => MapEntry<String, String>(k, v.toString()),
  );
  final List<FlutterEngineMetricPoint> points = <FlutterEngineMetricPoint>[];

  for (Map<String, dynamic> item in jsonResult['benchmarks']) {
    _parseAnItem(item, points, context, gitRevision);
  }

  return points;
}

Future<void> main(List<String> args) async {
  if (args.length != 1) {
    throw 'Must have one argument: <benchmark_json_file>';
  }
  final List<FlutterEngineMetricPoint> points = await _parse(args[0]);
  // The data will be sent to the Datastore of the GCP project specified through
  // environment variable BENCHMARK_GCP_CREDENTIALS. The engine Cirrus job has
  // currently configured the GCP project to flutter-cirrus for test. We'll
  // eventually migrate to flutter-infra project once the test is done.
  final FlutterDestination destination =
      await FlutterDestination.makeFromCredentialsJson(
    jsonDecode(Platform.environment['BENCHMARK_GCP_CREDENTIALS']),
  );
  await destination.update(points);
}
