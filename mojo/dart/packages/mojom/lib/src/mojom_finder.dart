// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of generate;

class PackageInfo {
  final String name;
  final Directory packageDir;
  final Directory importDir;
  final List<File> mojomFiles;
  PackageInfo(this.name, this.packageDir, this.importDir, this.mojomFiles);
}

/// This class finds .mojom files under [_mojomRootDir] that have a
/// 'DartPackage' annotation. It locates the packages named in the annotations
/// and generates a list of [PackageInfo] records.
class MojomFinder {
  static final Stopwatch _stopwatch = new Stopwatch();
  static dev.Counter _mojomMs;

  Directory _mojomRootDir;
  Directory _dartRootDir;
  List<String> _skip;
  Map<String, String> _packageLocations;

  MojomFinder(this._mojomRootDir, this._dartRootDir, this._skip) {
    _packageLocations = new Map<String, String>();
    if (_mojomMs == null) {
      _mojomMs = new dev.Counter("mojom searching",
          "Time(ms) searching for .mojom files with DartPackage annotations.");
      dev.Metrics.register(_mojomMs);
    }
  }

  /// Look for .mojom files in the tree that have a DartPackage annotation.
  /// Return a list of PackageInfo records describing packages and the .mojom
  /// files that they own.
  Future<List<PackageInfo>> find() async {
    _stopwatch.start();
    var packageInfos = new Map<String, PackageInfo>();
    var mojomRootList = _mojomRootDir.list(recursive: true, followLinks: false);
    await for (var entry in mojomRootList) {
      if (entry is! File) continue;
      if (_shouldSkip(entry)) continue;
      if (!isMojom(entry.path)) continue;

      String package = await _extractDartPackageAttribute(entry);
      if (package == null) continue;

      log.info("MojomFinder: found package $package");
      if (packageInfos.containsKey(package)) {
        packageInfos[package].mojomFiles.add(entry);
        continue;
      }

      Directory dartSourceDir = await _findDartSourceDir(package);
      if (dartSourceDir == null) {
        log.warning(".mojom had annotation '$package', but no package by that "
            "name under $_dartRootDir could be found.");
        continue;
      }
      log.info("MojomFinder: found $package at $dartSourceDir");

      var packageInfo =
          new PackageInfo(package, dartSourceDir, _mojomRootDir, [entry]);
      packageInfos[package] = packageInfo;
    }
    _stopwatch.stop();
    _mojomMs.value += _stopwatch.elapsedMilliseconds;
    _stopwatch.reset();
    return packageInfos.values.toList();
  }

  /// Extract a DartPackage attribute from a .mojom file.
  Future<String> _extractDartPackageAttribute(File mojom) async {
    String contents = await mojom.readAsString();
    int dpIndex = contents.indexOf('DartPackage');
    if (dpIndex == -1) return null;

    // There must be a '[' before 'DartPackage', and there can't be a ']'
    // in between.
    int openSbIndex = contents.lastIndexOf('[', dpIndex);
    if (openSbIndex == -1) return null;
    int closeSbIndex = contents.lastIndexOf(']', dpIndex);
    if (closeSbIndex > openSbIndex) return null;

    int eqIndex = contents.indexOf('=', dpIndex);
    if (eqIndex == -1) return null;
    int openQuoteIndex = contents.indexOf('"', eqIndex);
    if (openQuoteIndex == -1) return null;
    int closeQuoteIndex = -1;
    int searchIndex = openQuoteIndex + 1;
    while (closeQuoteIndex == -1) {
      closeQuoteIndex = contents.indexOf('"', searchIndex);
      if (closeQuoteIndex == -1) break;
      if (contents[closeQuoteIndex - 1] == '\\') {
        searchIndex = closeQuoteIndex + 1;
        closeQuoteIndex = -1;
      }
    }
    if (closeQuoteIndex == -1) return null;
    return contents.substring(openQuoteIndex + 1, closeQuoteIndex);
  }

  /// Finds where the Dart package named [package] lives. Looks immediately
  /// under [_dartRootDir].
  Future<Directory> _findDartSourceDir(String package) async {
    var packagePath = path.join(_dartRootDir.path, package);
    Directory packageDir = new Directory(packagePath);
    log.info("Looking for dart package: $packagePath");
    if (await packageDir.exists()) {
      return packageDir;
    } else {
      log.info("$packagePath not found");
    }
  }

  bool _shouldSkip(File f) => containsPrefix(f.path, _skip);
}
