// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// This library generates Mojo bindings for a Dart package.

library generate;

import 'dart:async';
import 'dart:convert';
import 'dart:developer' as dev;
import 'dart:io';

import 'package:mojom/src/utils.dart';
import 'package:path/path.dart' as path;

part 'mojom_finder.dart';

class MojomGenerator {
  static dev.Counter _genMs;
  final bool _errorOnDuplicate;
  final bool _dryRun;
  final Directory _mojoSdk;

  Map<String, String> _duplicateDetection;
  int _generationMs;

  MojomGenerator(this._mojoSdk,
      {bool errorOnDuplicate: true, bool profile: false, bool dryRun: false})
      : _errorOnDuplicate = errorOnDuplicate,
        _dryRun = dryRun,
        _generationMs = 0,
        _duplicateDetection = new Map<String, String>() {
    if (_genMs == null) {
      _genMs = new dev.Counter("mojom generation",
          "Time spent waiting for bindings generation script in ms.");
      dev.Metrics.register(_genMs);
    }
  }

  generate(PackageInfo info) async {
    // Count the .mojom files, and find the modification time of the most
    // recently modified one.
    int mojomCount = info.mojomFiles.length;
    DateTime newestMojomTime;
    for (File mojom in info.mojomFiles) {
      DateTime mojomTime = await getModificationTime(mojom);
      if ((newestMojomTime == null) || newestMojomTime.isBefore(mojomTime)) {
        newestMojomTime = mojomTime;
      }
    }

    // Count the .mojom.dart files, and find the modification time of the
    // least recently modified one.
    List mojomDartInfo = await _findOldestMojomDart(info.packageDir);
    DateTime oldestMojomDartTime = null;
    int mojomDartCount = 0;
    if (mojomDartInfo != null) {
      oldestMojomDartTime = mojomDartInfo[0];
      mojomDartCount = mojomDartInfo[1];
    }

    // If we don't have enough .mojom.dart files, or if a .mojom file is
    // newer than the oldest .mojom.dart file, then regenerate.
    if ((mojomDartCount < mojomCount) ||
        _shouldRegenerate(newestMojomTime, oldestMojomDartTime)) {
      for (File mojom in info.mojomFiles) {
        await _generateForMojom(
            mojom, info.importDir, info.packageDir, info.name);
      }
      // Delete any .mojom.dart files that are still older than mojomTime.
      await _deleteOldMojomDart(info.packageDir, newestMojomTime);
    }
  }

  /// Under [package]/lib, returns the oldest modification time for a
  /// .mojom.dart file.
  _findOldestMojomDart(Directory package) async {
    int mojomDartCount = 0;
    DateTime oldestModTime;
    Directory libDir = new Directory(path.join(package.path, 'lib'));
    if (!await libDir.exists()) return null;
    await for (var file in libDir.list(recursive: true, followLinks: false)) {
      if (file is! File) continue;
      if (!isMojomDart(file.path)) continue;
      DateTime modTime = (await file.stat()).modified;
      if ((oldestModTime == null) || oldestModTime.isAfter(modTime)) {
        oldestModTime = modTime;
      }
      mojomDartCount++;
    }
    return [oldestModTime, mojomDartCount];
  }

  // Delete .mojom.dart files under [package] that are [olderThanThis].
  _deleteOldMojomDart(Directory package, DateTime olderThanThis) async {
    Directory libDir = new Directory(path.join(package.path, 'lib'));
    assert(await libDir.exists());
    await for (var file in libDir.list(recursive: true, followLinks: false)) {
      if (file is! File) continue;
      if (!isMojomDart(file.path)) continue;
      DateTime modTime = (await file.stat()).modified;
      if (modTime.isBefore(olderThanThis)) {
        log.warning("Deleting stale .mojom.dart: $file");
        await file.delete();
      }
    }
  }

  /// If the .mojoms file or the newest .mojom is newer than the oldest
  /// .mojom.dart, then regenerate everything.
  bool _shouldRegenerate(DateTime mojomTime, DateTime mojomDartTime) {
    return (mojomTime == null) ||
        (mojomDartTime == null) ||
        mojomTime.isAfter(mojomDartTime);
  }

  _runBindingsGeneration(String script, List<String> arguments) async {
    var result;
    var stopwatch = new Stopwatch()..start();
    result = await Process.run(script, arguments);
    stopwatch.stop();
    _genMs.value += stopwatch.elapsedMilliseconds;
    return result;
  }

  _generateForMojom(File mojom, Directory importDir, Directory destination,
      String packageName) async {
    if (!isMojom(mojom.path)) return;
    log.info("_generateForMojom($mojom)");

    final script = path.join(
        _mojoSdk.path, 'tools', 'bindings', 'mojom_bindings_generator.py');
    final sdkInc = path.normalize(path.join(_mojoSdk.path, '..', '..'));
    final outputDir = await destination.createTemp();
    final output = outputDir.path;
    final arguments = [
      '--use_bundled_pylibs',
      '-g',
      'dart',
      '-o',
      output,
      // TODO(zra): Are other include paths needed?
      '-I',
      sdkInc,
      '-I',
      importDir.path,
      mojom.path
    ];

    log.info('Generating $mojom');
    log.info('$script ${arguments.join(" ")}');
    log.info('dryRun = $_dryRun');
    if (!_dryRun) {
      final result = await _runBindingsGeneration(script, arguments);
      if (result.exitCode != 0) {
        log.info("bindings generation result = ${result.exitCode}");
        await outputDir.delete(recursive: true);
        throw new GenerationError("$script failed:\n"
            "code: ${result.exitCode}\n"
            "stderr: ${result.stderr}\n"
            "stdout: ${result.stdout}");
      } else {
        log.info("bindings generation result = 0");
      }

      // Generated .mojom.dart is under $output/dart-pkg/$PACKAGE/lib/$X
      // Move $X to |destination|/lib/$X.
      // Throw an exception if $PACKGE != [packageName].
      final generatedDirName = path.join(output, 'dart-pkg');
      final generatedDir = new Directory(generatedDirName);
      log.info("generatedDir= $generatedDir");
      assert(await generatedDir.exists());
      await for (var genpack in generatedDir.list()) {
        if (genpack is! Directory) continue;
        log.info("genpack = $genpack");
        var libDir = new Directory(path.join(genpack.path, 'lib'));
        var name = path.relative(genpack.path, from: generatedDirName);
        log.info("Found generated lib dir: $libDir");

        if (packageName != name) {
          await outputDir.delete(recursive: true);
          throw new GenerationError(
              "Tried to generate for package $name in package $packageName");
        }

        var copyDest = new Directory(path.join(destination.path, 'lib'));
        log.info("Copy $libDir to $copyDest");
        await _copyBindings(copyDest, libDir);
      }

      await outputDir.delete(recursive: true);
    }
  }

  /// Searches for .mojom.dart files under [sourceDir] and copies them to
  /// [destDir].
  _copyBindings(Directory destDir, Directory sourceDir) async {
    var sourceList = sourceDir.list(recursive: true, followLinks: false);
    await for (var mojom in sourceList) {
      if (mojom is! File) continue;
      if (!isMojomDart(mojom.path)) continue;
      log.info("Found $mojom");

      final relative = path.relative(mojom.path, from: sourceDir.path);
      final dest = path.join(destDir.path, relative);
      final destDirectory = new Directory(path.dirname(dest));

      if (_errorOnDuplicate && _duplicateDetection.containsKey(dest)) {
        String original = _duplicateDetection[dest];
        throw new GenerationError(
            'Conflict: Both ${original} and ${mojom.path} supply ${dest}');
      }
      _duplicateDetection[dest] = mojom.path;

      log.info('Copying $mojom to $dest');
      if (!_dryRun) {
        final File source = new File(mojom.path);
        final File destFile = new File(dest);
        if (await destFile.exists()) {
          await destFile.delete();
        }
        log.info("Ensuring $destDirectory exists");
        await destDirectory.create(recursive: true);
        await source.copy(dest);
        await markFileReadOnly(dest);
      }
    }
  }
}

/// Given the location of the Mojo SDK and a root directory from which to begin
/// a search. Find .mojom files, and generate bindings for the relevant
/// packages.
class TreeGenerator {
  final MojomGenerator _generator;
  final Directory _mojoSdk;
  final Directory _mojomRootDir;
  final Directory _dartRootDir;
  final List<String> _skip;
  final bool _dryRun;

  Set<String> _processedPackages;

  int errors;

  TreeGenerator(
      Directory mojoSdk, this._mojomRootDir, this._dartRootDir, this._skip,
      {bool dryRun: false})
      : _mojoSdk = mojoSdk,
        _dryRun = dryRun,
        _generator = new MojomGenerator(mojoSdk, dryRun: dryRun),
        _processedPackages = new Set<String>(),
        errors = 0;

  generate() async {
    var mojomFinder = new MojomFinder(_mojomRootDir, _dartRootDir, _skip);
    List<PackageInfo> packageInfos = await mojomFinder.find();
    for (PackageInfo info in packageInfos) {
      await _runGenerate(info);
    }
  }

  _runGenerate(PackageInfo info) async {
    try {
      log.info('Generating bindings for ${info.name}');
      await _generator.generate(info);
      log.info('Done generating bindings for ${info.name}');
    } on GenerationError catch (e) {
      log.severe('Bindings generation failed for package ${info.name}: $e');
      errors += 1;
    }
  }

  bool _shouldSkip(File f) => containsPrefix(f.path, _skip);
}

/// Given the root of a directory tree to check, and the root of a directory
/// tree containing the canonical generated bindings, checks that the files
/// match, and recommends actions to take in case they don't. The canonical
/// directory should contain a subdirectory for each package that might be
/// encountered while traversing the directory tree being checked.
class TreeChecker {
  final Directory _mojoSdk;
  final Directory _mojomRootDir;
  final Directory _dartRootDir;
  final Directory _canonical;
  final List<String> _skip;
  int _errors;

  TreeChecker(this._mojoSdk, this._mojomRootDir, this._dartRootDir,
      this._canonical, this._skip)
      : _errors = 0;

  check() async {
    // Generate missing .mojoms files if needed.
    var mojomFinder = new MojomFinder(_mojomRootDir, _dartRootDir, _skip);
    List<PackageInfo> packageInfos = await mojomFinder.find();

    for (PackageInfo info in packageInfos) {
      log.info("Checking package at ${info.packageDir}");
      await _checkAll(info.packageDir);
      await _checkSame(info.packageDir);
    }

    // If there were multiple mismatches, explain how to regenerate the bindings
    // for the whole tree.
    if (_errors > 1) {
      String dart = makeRelative(Platform.executable);
      String packRoot = (Platform.packageRoot == "")
          ? ""
          : "-p " + makeRelative(Platform.packageRoot);
      String scriptPath = makeRelative(path.fromUri(Platform.script));
      String mojoSdk = makeRelative(_mojoSdk.path);
      String root = makeRelative(_mojomRootDir.path);
      String dartRoot = makeRelative(_dartRootDir.path);
      String skips = _skip.map((s) => "-s " + makeRelative(s)).join(" ");
      stderr.writeln('It looks like there were multiple problems. '
          'You can run the following command to regenerate bindings for your '
          'whole tree:\n'
          '\t$dart $packRoot $scriptPath gen -m $mojoSdk -r $root -o $dartRoot '
          '$skips');
    }
  }

  int get errors => _errors;

  // Check that the files are the same.
  _checkSame(Directory package) async {
    Directory libDir = new Directory(path.join(package.path, 'lib'));
    await for (var entry in libDir.list(recursive: true, followLinks: false)) {
      if (entry is! File) continue;
      if (!isMojomDart(entry.path)) continue;

      String relPath = path.relative(entry.path, from: package.parent.path);
      File canonicalFile = new File(path.join(_canonical.path, relPath));
      if (!await canonicalFile.exists()) {
        log.info("No canonical file for $entry");
        continue;
      }

      log.info("Comparing $entry with $canonicalFile");
      int fileComparison = await compareFiles(entry, canonicalFile);
      if (fileComparison != 0) {
        String entryPath = makeRelative(entry.path);
        String canonicalPath = makeRelative(canonicalFile.path);
        if (fileComparison > 0) {
          stderr.writeln('The generated file:\n\t$entryPath\n'
              'is newer thanthe canonical file\n\t$canonicalPath\n,'
              'and they are different. Regenerate canonical files?');
        } else {
          String dart = makeRelative(Platform.executable);
          String packRoot = (Platform.packageRoot == "")
              ? ""
              : "-p " + makeRelative(Platform.packageRoot);
          String root = makeRelative(_mojomRootDir.path);
          String packagePath = makeRelative(package.path);
          String scriptPath = makeRelative(path.fromUri(Platform.script));
          String mojoSdk = makeRelative(_mojoSdk.path);
          String skips = _skip.map((s) => "-s " + makeRelative(s)).join(" ");
          stderr.writeln('For the package: $packagePath\n'
              'The generated file:\n\t$entryPath\n'
              'is older than the canonical file:\n\t$canonicalPath\n'
              'and they are different. Regenerate by running:\n'
              '\t$dart $packRoot $scriptPath single -m $mojoSdk -r $root '
              '-p $packagePath $skips');
        }
        _errors++;
        return;
      }
    }
  }

  // Check that every .mojom.dart in the canonical package is also in the
  // package we are checking.
  _checkAll(Directory package) async {
    String packageName = path.relative(package.path, from: package.parent.path);
    String canonicalPackagePath =
        path.join(_canonical.path, packageName, 'lib');
    Directory canonicalPackage = new Directory(canonicalPackagePath);
    if (!await canonicalPackage.exists()) return;

    var canonicalPackages =
        canonicalPackage.list(recursive: true, followLinks: false);
    await for (var entry in canonicalPackages) {
      if (entry is! File) continue;
      if (!isMojomDart(entry.path)) continue;

      String relPath = path.relative(entry.path, from: canonicalPackage.path);
      File genFile = new File(path.join(package.path, 'lib', relPath));
      log.info("Checking that $genFile exists");
      if (!await genFile.exists()) {
        String dart = makeRelative(Platform.executable);
        String packRoot = (Platform.packageRoot == "")
            ? ""
            : "-p " + makeRelative(Platform.packageRoot);
        String root = makeRelative(_mojomRootDir.path);
        String genFilePath = makeRelative(genFile.path);
        String packagePath = makeRelative(package.path);
        String scriptPath = makeRelative(path.fromUri(Platform.script));
        String mojoSdk = makeRelative(_mojoSdk.path);
        String skips = _skip.map((s) => "-s " + makeRelative(s)).join(" ");
        stderr.writeln('The generated file:\n\t$genFilePath\n'
            'is needed but does not exist. Run the command\n'
            '\t$dart $packRoot $scriptPath single -m $mojoSdk -r $root '
            '-p $packagePath $skips');
        _errors++;
        return;
      }
    }
  }

  bool _shouldSkip(File f) => containsPrefix(f.path, _skip);
}
