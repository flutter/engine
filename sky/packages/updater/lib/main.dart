// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';
import 'dart:math';
import 'dart:typed_data';

import 'package:mojo/core.dart';
import 'package:sky/services.dart';
import 'package:path/path.dart' as path;
import 'package:yaml/yaml.dart' as yaml;

const String kManifestFile = 'sky.yaml';
const String kBundleFile = 'app.skyx';

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
    return _parts.length - other._parts.length;  // results in 1.0 < 1.0.0
  }
}

class PipeToFile {
  MojoDataPipeConsumer _consumer;
  MojoEventStream _eventStream;
  IOSink _outputStream;

  PipeToFile(this._consumer, String outputPath) {
    _eventStream = new MojoEventStream(_consumer.handle);
    _outputStream = new File(outputPath).openWrite();
  }

  Future<MojoResult> _doRead() async {
    ByteData thisRead = _consumer.beginRead();
    if (thisRead == null) {
      throw 'Data pipe beginRead failed: ${_consumer.status}';
    }
    // TODO(mpcomplete): Should I worry about the _eventStream listen callback
    // being invoked again before this completes?
    await _outputStream.add(thisRead.buffer.asUint8List());
    return _consumer.endRead(thisRead.lengthInBytes);
  }

  Future<MojoResult> drain() async {
    var completer = new Completer();
    // TODO(mpcomplete): Is it legit to pass an async callback to listen?
    _eventStream.listen((List<int> event) async {
      var mojoSignals = new MojoHandleSignals(event[1]);
      if (mojoSignals.isReadable) {
        var result = await _doRead();
        if (!result.isOk) {
          _eventStream.close();
          _eventStream = null;
          _outputStream.close();
          completer.complete(result);
        } else {
          _eventStream.enableReadEvents();
        }
      } else if (mojoSignals.isPeerClosed) {
        _eventStream.close();
        _eventStream = null;
        _outputStream.close();
        completer.complete(MojoResult.OK);
      } else {
        throw 'Unexpected handle event: $mojoSignals';
      }
    });
    return completer.future;
  }

  static Future<MojoResult> copyToFile(MojoDataPipeConsumer consumer, String outputPath) {
    var drainer = new PipeToFile(consumer, outputPath);
    return drainer.drain();
  }
}

class UpdateTask {
  UpdateTask() {}

  run() async {
    _dataDir = await getDataDir();

    await _readLocalManifest();
    yaml.YamlMap remoteManifest = await _fetchManifest();
    if (!_shouldUpdate(remoteManifest)) {
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

  yaml.YamlMap _currentManifest;
  String _dataDir;
  String _tempPath;

  _readLocalManifest() async {
    String manifestPath = path.join(_dataDir, kManifestFile);
    String manifestData = await new File(manifestPath).readAsString();
    _currentManifest = yaml.loadYaml(manifestData, sourceUrl: manifestPath);
  }

  Future<yaml.YamlMap> _fetchManifest() async {
    String manifestUrl = _currentManifest['update_url'] + '/' + kManifestFile;
    String manifestData = await fetchString(manifestUrl);
    return yaml.loadYaml(manifestData, sourceUrl: manifestUrl);
  }

  bool _shouldUpdate(yaml.YamlMap remoteManifest) {
    Version currentVersion = new Version(_currentManifest['version']);
    Version remoteVersion = new Version(remoteManifest['version']);
    return (currentVersion < remoteVersion);
  }

  Future<MojoResult> _fetchBundle() async {
    String bundleUrl = _currentManifest['update_url'] + '/' + kBundleFile;
    var response = await fetchUrl(bundleUrl);
    // TODO(mpcomplete): Use the cache dir. We need an equivalent of mkstemp().
    _tempPath = path.join(_dataDir, 'tmp.skyx');
    return PipeToFile.copyToFile(response.body, _tempPath);
  }

  _replaceBundle() async {
    String bundlePath = path.join(_dataDir, kBundleFile);
    await new File(_tempPath).rename(bundlePath);
  }
}

String cachedDataDir = null;
Future<String> getDataDir() async {
  if (cachedDataDir == null)
    cachedDataDir = await getAppDataDir();
  return cachedDataDir;
}

// TODO(mpcomplete): method for notifying caller. native? mojo?

void main() {
  var task = new UpdateTask();
  task.run();
}
