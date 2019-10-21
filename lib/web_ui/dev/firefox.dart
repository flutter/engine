
// import 'dart:async';
// import 'dart:io';

// import 'package:pedantic/pedantic.dart';

// import 'package:test_core/src/util/io.dart'; // ignore: implementation_imports

// import 'browser.dart';
// import 'firefox_installer.dart';
// import 'common.dart';

// /// A class for running an instance of Firefox.
// ///
// /// Most of the communication with the browser is expected to happen via HTTP,
// /// so this exposes a bare-bones API. The browser starts as soon as the class is
// /// constructed, and is killed when [close] is called.
// ///
// /// Any errors starting or running the process are reported through [onExit].
// class Firefox extends Browser {
//   @override
//   final name = 'Firefox';

//   @override
//   final Future<Uri> remoteDebuggerUrl;

//   static String version;

//   /// Starts a new instance of Firefox open to the given [url], which may be a
//   /// [Uri] or a [String].
//   factory Firefox(Uri url, {bool debug = false}) {
//     assert(version != null);
//     var remoteDebuggerCompleter = Completer<Uri>.sync();
//     return Firefox._(() async {
//       final BrowserInstallation installation = await getOrInstallFirefox(
//         version,
//         infoLog: isCirrus ? stdout : DevNull(),
//       );

//       // A good source of various Firefox CLI options:
//       // https://peter.sh/experiments/chromium-command-line-switches/
//       //
//       // Things to try:
//       // --font-render-hinting
//       // --enable-font-antialiasing
//       // --gpu-rasterization-msaa-sample-count
//       // --disable-gpu
//       // --disallow-non-exact-resource-reuse
//       // --disable-font-subpixel-positioning
//       final bool isFirefoxNoSandbox = Platform.environment['CHROME_NO_SANDBOX'] == 'true';
//       var dir = createTempDir();
//       var args = [
//         '--user-data-dir=$dir',
//         url.toString(),
//         if (!debug) '--headless',
//         if (isFirefoxNoSandbox) '--no-sandbox',
//         '--window-size=$kMaxScreenshotWidth,$kMaxScreenshotHeight', // When headless, this is the actual size of the viewport
//         '--disable-extensions',
//         '--disable-popup-blocking',
//         '--bwsi',
//         '--no-first-run',
//         '--no-default-browser-check',
//         '--disable-default-apps',
//         '--disable-translate',
//         '--remote-debugging-port=$kDevtoolsPort',
//       ];

//       final Process process = await Process.start(installation.executable, args);

//       remoteDebuggerCompleter.complete(getRemoteDebuggerUrl(
//           Uri.parse('http://localhost:$_kFirefoxDevtoolsPort')));

//       unawaited(process.exitCode
//           .then((_) => Directory(dir).deleteSync(recursive: true)));

//       return process;
//     }, remoteDebuggerCompleter.future);
//   }

//   Firefox._(Future<Process> startBrowser(), this.remoteDebuggerUrl)
//       : super(startBrowser);
// }
