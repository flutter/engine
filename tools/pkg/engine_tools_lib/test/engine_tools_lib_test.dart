// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;
import 'package:engine_tools_lib/engine_tools_lib.dart';
import 'package:litetest/litetest.dart';
import 'package:path/path.dart' as p;

void main() {
  late io.Directory emptyDir;

  void setUp() {
    emptyDir = io.Directory.systemTemp.createTempSync('engine_tools_lib.test');
  }

  void tearDown() {
    emptyDir.deleteSync(recursive: true);
  }

  group('Engine.fromSrcPath', () {
    group('should fail when', () {
      test('the path does not end in `${p.separator}src`', () {
        setUp();
        try {
          expect(
          () => Engine.fromSrcPath(emptyDir.path),
          throwsArgumentError,
        );
        } finally {
          tearDown();
        }
      });

      test('the path does not exist', () {
        setUp();
        try {
          expect(
            () => Engine.fromSrcPath(p.join(emptyDir.path, 'src')),
            throwsArgumentError,
          );
        } finally {
          tearDown();
        }
      });

      test('the path does not contain a "flutter" directory', () {
        setUp();
        try {
          final io.Directory srcDir = io.Directory(p.join(emptyDir.path, 'src'))..createSync();
          expect(
            () => Engine.fromSrcPath(srcDir.path),
            throwsArgumentError,
          );
        } finally {
          tearDown();
        }
      });

      test('the path does not contain an "out" directory', () {
        setUp();
        try {
          final io.Directory srcDir = io.Directory(p.join(emptyDir.path, 'src'))..createSync();
          io.Directory(p.join(srcDir.path, 'flutter')).createSync();
          expect(
            () => Engine.fromSrcPath(srcDir.path),
            throwsArgumentError,
          );
        } finally {
          tearDown();
        }
      });

      test('returns an Engine', () {
        setUp();
        try {
          final io.Directory srcDir = io.Directory(p.join(emptyDir.path, 'src'))..createSync();
          io.Directory(p.join(srcDir.path, 'flutter')).createSync();
          io.Directory(p.join(srcDir.path, 'out')).createSync();

          final Engine engine = Engine.fromSrcPath(srcDir.path);

          expect(engine.srcDir.path, srcDir.path);
          expect(engine.flutterDir.path, p.join(srcDir.path, 'flutter'));
          expect(engine.outDir.path, p.join(srcDir.path, 'out'));
        } finally {
          tearDown();
        }
      });
    });
  });

  group('Engine.findWithin', () {
    late io.Directory emptyDir;

    void setUp() {
      emptyDir = io.Directory.systemTemp.createTempSync('engine_tools_lib.test');
    }

    void tearDown() {
      emptyDir.deleteSync(recursive: true);
    }

    group('should fail when', () {
      test('the path does not contain a "src" directory', () {
        setUp();
        try {
          expect(
            () => Engine.findWithin(emptyDir.path),
            throwsArgumentError,
          );
        } finally {
          tearDown();
        }
      });

      test('returns an Engine', () {
        setUp();
        try {
          final io.Directory srcDir = io.Directory(p.join(emptyDir.path, 'src'))..createSync();
          io.Directory(p.join(srcDir.path, 'flutter')).createSync();
          io.Directory(p.join(srcDir.path, 'out')).createSync();

          final Engine engine = Engine.findWithin(srcDir.path);

          expect(engine.srcDir.path, srcDir.path);
          expect(engine.flutterDir.path, p.join(srcDir.path, 'flutter'));
          expect(engine.outDir.path, p.join(srcDir.path, 'out'));
        } finally {
          tearDown();
        }
      });
    });
  });

  test('outputs a list of targets', () {
    setUp();

    try {
      // Create a valid engine.
      io.Directory(p.join(emptyDir.path, 'src', 'flutter')).createSync(recursive: true);
      io.Directory(p.join(emptyDir.path, 'src', 'out')).createSync(recursive: true);

      // Create two targets in out: host_debug and host_debug_unopt_arm64.
      io.Directory(p.join(emptyDir.path, 'src', 'out', 'host_debug')).createSync(recursive: true);
      io.Directory(p.join(emptyDir.path, 'src', 'out', 'host_debug_unopt_arm64')).createSync(recursive: true);

      final Engine engine = Engine.fromSrcPath(p.join(emptyDir.path, 'src'));
      final List<String> outputs = engine.outputs().map((Output o) => p.basename(o.dir.path)).toList()..sort();
      expect(outputs, <String>[
        'host_debug',
        'host_debug_unopt_arm64',
      ]);
    } finally {
      tearDown();
    }
  });

  test('outputs the latest target and compile_commands.json', () {
    setUp();

    try {
      // Create a valid engine.
      io.Directory(p.join(emptyDir.path, 'src', 'flutter')).createSync(recursive: true);
      io.Directory(p.join(emptyDir.path, 'src', 'out')).createSync(recursive: true);

      // Create two targets in out: host_debug and host_debug_unopt_arm64.
      io.Directory(p.join(emptyDir.path, 'src', 'out', 'host_debug')).createSync(recursive: true);
      io.Directory(p.join(emptyDir.path, 'src', 'out', 'host_debug_unopt_arm64')).createSync(recursive: true);

      // Intentionnally make host_debug a day old to ensure it is not picked.
      final io.File oldJson = io.File(p.join(emptyDir.path, 'src', 'out', 'host_debug', 'compile_commands.json'))..createSync();
      oldJson.setLastModifiedSync(oldJson.lastModifiedSync().subtract(const Duration(days: 1)));

      io.File(p.join(emptyDir.path, 'src', 'out', 'host_debug_unopt_arm64', 'compile_commands.json')).createSync();

      final Engine engine = Engine.fromSrcPath(p.join(emptyDir.path, 'src'));
      final Output? latestOutput = engine.latestOutput();
      expect(latestOutput, isNotNull);
      expect(p.basename(latestOutput!.dir.path), 'host_debug_unopt_arm64');
      expect(latestOutput.compileCommandsJson, isNotNull);
    } finally {
      tearDown();
    }
  });
}
