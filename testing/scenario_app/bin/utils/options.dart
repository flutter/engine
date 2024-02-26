import 'dart:io' as io;

import 'package:args/args.dart';
import 'package:engine_repo_tools/engine_repo_tools.dart';
import 'package:path/path.dart' as p;

import 'environment.dart';

/// Command line options and parser for the Android `scenario_app` test runner.
extension type const Options._(ArgResults _args) {
  /// Parses the command line [args] into a set of options.
  factory Options.parse(
    List<String> args, {
    required Environment environment,
    required Engine? localEngine,
  }) {
    final ArgResults results = _parser(environment, localEngine).parse(args);
    final Options options = Options._(results);
    
    // The 'adb' tool must exist.
    if (results['adb'] == null) {
      throw ArgumentError('The --adb option must be set.');
    } else if (!io.File(options.adb).existsSync()) {
      throw ArgumentError('The adb tool does not exist at ${options.adb}.');
    }

    // The 'ndk-stack' tool must exist.
    if (results['ndk-stack'] == null) {
      throw ArgumentError('The --ndk-stack option must be set.');
    } else if (!io.File(options.ndkStack).existsSync()) {
      throw ArgumentError('The ndk-stack tool does not exist at ${options.ndkStack}.');
    }

    // The 'out-dir' must exist.
    if (results['out-dir'] == null) {
      throw ArgumentError('The --out-dir option must be set.');
    } else if (!io.Directory(options.outDir).existsSync()) {
      throw ArgumentError('The out directory does not exist at ${options.outDir}.');
    }

    return options;
  }

  /// A subset of command line options used by the `scenario_app` runner.
  ///
  /// These options are expected to be parsed before the full set of options.
  static final ArgParser _miniParser = ArgParser()
    ..addFlag(
      'verbose',
      abbr: 'v',
      help: 'Enable verbose logging',
    )
    ..addFlag(
      'help',
      abbr: 'h',
      help: 'Print usage information',
      negatable: false,
    );

  /// Whether usage information should be shown based on command line [args].
  ///
  /// This is a shortcut that can be used to determine if the usage information
  /// before parsing the remaining command line arguments. For example:
  ///
  /// ```
  /// void main(List<String> args) {
  ///   if (Options.showUsage(args)) {
  ///     stdout.writeln(Options.usage);
  ///     return;
  ///   }
  ///   final options = Options.parse(args);
  ///   // ...
  /// }
  /// ```
  static bool showUsage(List<String> args) {
    final ArgResults results = _miniParser.parse(args);
    return results['help'] as bool;
  }

  /// Whether verbose logging should be enabled based on command line [args].
  ///
  /// This is a shortcut that can be used to determine if verbose logging should
  /// be enabled before parsing the remaining command line arguments. For
  /// example:
  ///
  /// ```
  /// void main(List<String> args) {
  ///   final bool verbose = Options.showVerbose(args);
  ///   // ...
  /// }
  /// ```
  static bool showVerbose(List<String> args) {
    final ArgResults results = _miniParser.parse(args);
    return results['verbose'] as bool;
  }

  /// Returns usage information for the `scenario_app` test runner.
  ///
  /// If [verbose] is `true`, then additional options are shown.
  static String usage({
    required Environment environment,
    required Engine? localEngineDir,
  }) {
    return _parser(environment, localEngineDir).usage;
  }

  /// Parses the command line [args] into a set of options.
  ///
  /// Unlike [_miniParser], this parser includes all options.
  static ArgParser _parser(Environment environment, Engine? localEngine) {
    final bool hideUnusualOptions = !environment.showVerbose;
    return ArgParser()
      ..addFlag(
        'verbose',
        abbr: 'v',
        help: 'Enable verbose logging',
      )
      ..addFlag(
        'help',
        abbr: 'h',
        help: 'Print usage information',
      )
      ..addFlag(
        'use-skia-gold',
        help:
            'Whether to use Skia Gold to compare screenshots. Defaults to true '
            'on CI and false otherwise.',
        defaultsTo: environment.isCi,
        hide: hideUnusualOptions,
      )
      ..addFlag(
        'enable-impeller',
        help:
            'Whether to enable Impeller as the graphics backend. If true, the '
            'test runner will use --impeller-backend if set, otherwise the '
            'default backend will be used. To explicitly run with the Skia '
            'backend, set this to false (--no-enable-impeller).',
      )
      ..addOption(
        'impeller-backend',
        help: 'The graphics backend to use when --enable-impeller is true. '
            'Unlike the similar option when launching an app, there is no '
            'fallback; that is, either Vulkan or OpenGLES must be specified. ',
        allowed: <String>['vulkan', 'opengles'],
        defaultsTo: 'vulkan',
      )
      ..addOption(
        'logs-dir',
        help: 'Path to a directory where logs and screenshots are stored.',
        defaultsTo: environment.logsDir,
      )
      ..addOption(
        'adb',
        help: 'Path to the Android Debug Bridge (adb) executable. '
            'If the current working directory is within the engine repository, '
            'defaults to ./third_party/android_tools/sdk/platform-tools/adb.',
        defaultsTo: localEngine != null
            ? p.join(
                localEngine.srcDir.path,
                'third_party',
                'android_tools',
                'sdk',
                'platform-tools',
                'adb',
              )
            : null,
        valueHelp: 'path/to/adb',
        hide: hideUnusualOptions,
      )
      ..addOption(
        'ndk-stack',
        help:
            'Path to the NDK stack tool. Defaults to the checked-in version in '
            'third_party/android_tools if the current working directory is '
            'within the engine repository on a supported platform.',
        defaultsTo: localEngine != null &&
                (io.Platform.isLinux ||
                    io.Platform.isMacOS ||
                    io.Platform.isWindows)
            ? p.join(
                localEngine.srcDir.path,
                'third_party',
                'android_tools',
                'ndk',
                'prebuilt',
                () {
                  if (io.Platform.isLinux) {
                    return 'linux-x86_64';
                  } else if (io.Platform.isMacOS) {
                    return 'darwin-x86_64';
                  } else if (io.Platform.isWindows) {
                    return 'windows-x86_64';
                  } else {
                    // Unreachable.
                    throw UnsupportedError(
                      'Unsupported platform: ${io.Platform.operatingSystem}',
                    );
                  }
                }(),
                'bin',
                'ndk-stack',
              )
            : null,
        valueHelp: 'path/to/ndk-stack',
        hide: hideUnusualOptions,
      )
      ..addOption(
        'out-dir',
        help: 'Path to a out/{variant} directory where the APKs are built. '
            'Defaults to the latest updated out/ directory that starts with '
            '"android_" if the current working directory is within the engine '
            'repository.',
        defaultsTo: localEngine
            ?.outputs()
            .where((Output o) => p.basename(o.path.path).startsWith('android_'))
            .firstOrNull
            ?.path
            .path,
        mandatory: environment.isCi,
        valueHelp: 'path/to/out/android_variant',
      )
      ..addOption(
        'smoke-test',
        help: 'Fully qualified class name of a single test to run. For example '
            'try "dev.flutter.scenarios.EngineLaunchE2ETest" or '
            '"dev.flutter.scenariosui.ExternalTextureTests".',
        valueHelp: 'package.ClassName',
      )
      ..addOption(
        'output-contents-golden',
        help: 'Path to a file that contains the expected filenames of golden '
            'files. If the current working directory is within the engine '
            'repository, defaults to ./testing/scenario_app/android/'
            'expected_golden_output.txt.',
        defaultsTo: localEngine != null
            ? p.join(
                localEngine.srcDir.path,
                'testing',
                'scenario_app',
                'android',
                'expected_golden_output.txt',
              )
            : null,
        valueHelp: 'path/to/golden.txt',
      );
  }

  /// Whether verbose logging should be enabled.
  bool get verbose => _args['verbose'] as bool;

  /// Whether usage information should be shown.
  bool get help => _args['help'] as bool;

  /// Whether to use Skia Gold to compare screenshots.
  bool get useSkiaGold => _args['use-skia-gold'] as bool;

  /// Whether to enable Impeller as the graphics backend.
  bool get enableImpeller => _args['enable-impeller'] as bool;

  /// The graphics backend to use when --enable-impeller is true.
  String get impellerBackend => _args['impeller-backend'] as String;

  /// Path to a directory where logs and screenshots are stored.
  String get logsDir => _args['logs-dir'] as String;

  /// Path to the Android Debug Bridge (adb) executable.
  String get adb => _args['adb'] as String;

  /// Path to the NDK stack tool.
  String get ndkStack => _args['ndk-stack'] as String;

  /// Path to a out/{variant} directory where the APKs are built.
  String get outDir => _args['out-dir'] as String;

  /// Fully qualified class name of a single test to run.
  String? get smokeTest => _args['smoke-test'] as String?;

  /// Path to a file that contains the expected filenames of golden files.
  String? get outputContentsGolden => _args['output-contents-golden'] as String;
}
