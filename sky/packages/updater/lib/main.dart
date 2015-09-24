// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';
import 'dart:math';

//import 'package:mojo/mojo/url_response.mojom.dart';
//import 'package:sky/material.dart';
//import 'package:sky/rendering.dart';
import 'package:sky/services.dart';
import 'package:path/path.dart' as path;
import 'package:yaml/yaml.dart' as yaml;
//import 'package:sky/widgets.dart';

class Version {
  Version(String versionStr) :
    _parts = versionStr.split('.').map((val) => int.parse(val)).toList();

  List<int> _parts;

  bool operator<(Version other) => _compare(other) < 0;
  bool operator==(Version other) => _compare(other) == 0;
  bool operator>(Version other) => _compare(other) > 0;

  int _compare(Version other) {
    int length = min(_parts.length, other._parts.length);
    for (int i = 0; i < length; ++i) {
      if (_parts[i] < other._parts[i])
        return -1;
      if (_parts[i] > other._parts[i])
        return 1;
    }
    return _parts.length - other._parts.length;  // results in 1.0.0 < 1.0
  }
}

class UpdateTask {
  UpdateTask() {}

  run() async {
    await _readLocalManifest();
    yaml.YamlMap remoteManifest = await _fetchManifest();
    if (_shouldUpdate(remoteManifest)) {
      print("Update skipped. No new version.");
      return;
    }
    await _fetchBundle();
  }

  _readLocalManifest() async {
    String dataDir = await getDataDir();
    String manifestPath = path.join(dataDir, 'sky.yaml');
    String manifestData = await new File(manifestPath).readAsString();
    print("manifestData: $manifestData");
    _currentManifest = yaml.loadYaml(manifestData, sourceUrl: manifestPath);
  }

  Future<yaml.YamlMap> _fetchManifest() async {
    String manifestUrl = _currentManifest['update_url'] + '/sky.yaml';
    String manifestData = await fetchString(manifestUrl);
    print("remote manifestData: $manifestData");
    return yaml.loadYaml(manifestData, sourceUrl: manifestUrl);
  }

  bool _shouldUpdate(yaml.YamlMap remoteManifest) {
    Version currentVersion = new Version(_currentManifest['version']);
    Version remoteVersion = new Version(remoteManifest['version']);
    return (currentVersion < remoteVersion);
  }

  _fetchBundle() async {
    String bundleUrl = _currentManifest['update_url'] + '/app.skyx';
    var data = await fetchBody(bundleUrl);
    print("Got: ${data.body.lengthInBytes}");
  }

  yaml.YamlMap _currentManifest;
}

String cachedDataDir = null;
Future<String> getDataDir() async {
  if (cachedDataDir == null)
    cachedDataDir = await getAppDataDir();
  return cachedDataDir;
}

// parse local manifest; get update_url
// fetch remote manifest
// compare versions
// download new app.skyx
// replace old app.skyx
// notify caller we're done
//
// need:
// local data_dir (for manifest)
// local tmp_dir (for downloads)
// method for notifying caller. native? mojo?

runTest() async {
  String dataDir = await getDataDir();
  String manifestPath = path.join(dataDir, 'sky.yaml');
  String manifestData = await new File(manifestPath).readAsString();
  print("manifestData: $manifestData");
  var manifestYaml = yaml.loadYaml(manifestData, sourceUrl: manifestPath);
  print('yaml: $doc');
  print(doc['update_url']);

//  String data = await fetchString("http://mpcomplete.org");
//  print("fetched: $data");
}

void main() {
  var task = new UpdateTask();
  task.run();
}
