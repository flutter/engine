// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';
import 'dart:math';

//import 'package:mojo/mojo/url_response.mojom.dart';
//import 'package:sky/material.dart';
//import 'package:sky/rendering.dart';
import 'package:mojo/core.dart';
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

class PipeToFile {
  MojoDataPipeConsumer _consumer;
  MojoEventStream _eventStream;
  File _outputFile;

  DataPipeDrainer(this._consumer, String outputPath) {
    _eventStream = new MojoEventStream(_consumer.handle);
    _outputFile = new File(outputPath);
  }

  Future<MojoResult> _doRead() async {
    ByteData thisRead = _consumer.beginRead();
    if (thisRead == null) {
      throw 'Data pipe beginRead failed: ${_consumer.status}';
    }
    await _outputFile.writeAsBytes(thisRead.buffer.asUint8List());
    return _consumer.endRead(thisRead.lengthInBytes);
  }

  Future<MojoResult> drain() {
    var completer = new Completer();
    _eventStream.listen((List<int> event) {
      var mojoSignals = new MojoHandleSignals(event[1]);
      if (mojoSignals.isReadable) {
        var result = _doRead();
        if (!result.isOk) {
          _eventStream.close();
          _eventStream = null;
          completer.complete(result);
        } else {
          _eventStream.enableReadEvents();
        }
      } else if (mojoSignals.isPeerClosed) {
        _eventStream.close();
        _eventStream = null;
        completer.complete(MojoResult.OK);
      } else {
        throw 'Unexpected handle event: $mojoSignals';
      }
    });
    return completer.future;
  }

  static Future<MojoResult> copyToFile(MojoDataPipeConsumer consumer, String outputPath) {
    var drainer = new DataPipeDrainer(consumer, outputPath);
    return drainer.drain();
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
    MojoResult result = await _fetchBundle();
    if (!result.isOk) {
      print("Update failed while fetching new skyx bundle.");
      return;
    }
    await _replaceBundle();
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

  Future<MojoResult> _fetchBundle() async {
    String bundleUrl = _currentManifest['update_url'] + '/app.skyx';
    var response = await fetchUrl(bundleUrl);
    String outputPath = path.join(dataDir, 'tmp.skyx');
    return PipeToFile.copyToFile(response.body, outputPath);
  }

  _replaceBundle() async {
    String fromPath = path.join(dataDir, 'tmp.skyx');
    String toPath = path.join(dataDir, 'app.skyx');
    await new File(fromPath).rename(toPath);
  }

  yaml.YamlMap _currentManifest;
}

String cachedDataDir = null;
Future<String> getDataDir() async {
  if (cachedDataDir == null)
    cachedDataDir = await getAppDataDir();
  return cachedDataDir;
}

// download new app.skyx to file
// replace old app.skyx
// notify caller we're done
//
// need:
// method for notifying caller. native? mojo?

void main() {
  var task = new UpdateTask();
  task.run();
}
