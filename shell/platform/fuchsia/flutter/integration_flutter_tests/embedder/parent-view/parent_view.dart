// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';
import 'package:args/args.dart';
import 'package:fidl_fuchsia_sys/fidl_async.dart';
import 'package:fidl_fuchsia_ui_app/fidl_async.dart';
import 'package:fidl_fuchsia_ui_views/fidl_async.dart';
import 'package:fuchsia_services/services.dart';
import 'package:zircon/zircon.dart';

const _kChildAppUrl =
    'fuchsia-pkg://engine/child-view#meta/child-view.cmx';

TestApp app;

Future<void> main(List<String> args) async {
  final parser = ArgParser()
    ..addFlag('showOverlay', defaultsTo: false)
    ..addFlag('hitTestable', defaultsTo: true)
    ..addFlag('focusable', defaultsTo: true);
  final arguments = parser.parse(args);
  for (final option in arguments.options) {
    print('parent-view: $option: ${arguments[option]}');
  }

  // TODO(richkadel): uncomment childViewToken when ready to launch child-view
  // final childViewToken = _launchApp(_kChildAppUrl);
  app = TestApp(
    // FuchsiaViewConnection(childViewToken),
    showOverlay: arguments['showOverlay'],
    hitTestable: arguments['hitTestable'],
    focusable: arguments['focusable'],
  );

  app.run();
}

class TestApp {
  static const _black = Color.fromARGB(255, 0, 0, 0);
  static const _blue = Color.fromARGB(255, 0, 0, 255);
  static const _red = Color.fromARGB(255, 255, 0, 0);

  // final FuchsiaViewConnection connection;
  final bool showOverlay;
  final bool hitTestable;
  final bool focusable;

  Color _backgroundColor = _blue;

  TestApp(
    // this.connection,
    {this.showOverlay = false,
    this.hitTestable = true,
    this.focusable = true});

  void run() {
    window.onBeginFrame = (Duration duration) {
      app.beginFrame(duration);
    };
    window.scheduleFrame();
  }

  void beginFrame(Duration duration) {
  //     FuchsiaViewConnection(childViewToken),
  // call and await "createView" behavior from fuchsia_views_cservice.dart (casts handle to a token)
  // don't drop token
  // put view in parent scene (size and position it)
    final pixelRatio = window.devicePixelRatio;
    final size = window.physicalSize / pixelRatio;
    final physicalBounds = Offset.zero & size * pixelRatio;
    final recorder = PictureRecorder();
    final canvas = Canvas(recorder, physicalBounds);
    canvas.scale(pixelRatio, pixelRatio);
    final paint = Paint()..color = this._backgroundColor;
    final width = size.width * .33;
    final height = size.height * .33;
    final center = size.center(Offset.zero);
    final left = center.dx - (width / 2);
    final top = center.dy - (height / 2);
    //canvas.drawCircle(center, size.shortestSide / 4, paint);
    canvas.drawRect(Rect.fromLTWH(left, top, width, height), paint);
    final picture = recorder.endRecording();
    final sceneBuilder = SceneBuilder()
      ..pushClipRect(physicalBounds)
      ..addPicture(Offset.zero, picture)
      // ..addPlatformView(...) for child view using same ID used in createView (a zircon handle, internally)
      ..pop();
      // may need to await on createView
    window.render(sceneBuilder.build());
  }

//   @override
//   Widget build(BuildContext context) {
//     return Listener(
//       onPointerDown: (_) => _backgroundColor.value = _black,
//       child: AnimatedBuilder(
//           animation: _backgroundColor,
//           builder: (context, snapshot) {
//             return Container(
//               color: _backgroundColor.value,
//               child: Stack(
//                 alignment: Alignment.center,
//                 children: [
//                   FractionallySizedBox(
//                     widthFactor: 0.33,
//                     heightFactor: 0.33,
//                     child: FuchsiaView(
//                       controller: connection,
//                       hitTestable: hitTestable,
//                       focusable: focusable,
//                     ),
//                   ),
//                   if (showOverlay)
//                     FractionallySizedBox(
//                       widthFactor: 0.66,
//                       heightFactor: 0.66,
//                       child: Container(
//                         alignment: Alignment.topRight,
//                         child: FractionallySizedBox(
//                           widthFactor: 0.5,
//                           heightFactor: 0.5,
//                           child: Container(
//                             color: Color.fromARGB(255, 0, 255, 0),
//                           ),
//                         ),
//                       ),
//                     ),
//                 ],
//               ),
//             );
//           }),
//     );
//   }
}

ViewHolderToken _launchApp(String componentUrl) {
  final incoming = Incoming();
  final componentController = ComponentControllerProxy();

  final launcher = LauncherProxy();
  Incoming.fromSvcPath()
    ..connectToService(launcher)
    ..close();
  launcher.createComponent(
    LaunchInfo(
      url: componentUrl,
      directoryRequest: incoming.request().passChannel(),
    ),
    componentController.ctrl.request(),
  );
  launcher.ctrl.close();

  ViewProviderProxy viewProvider = ViewProviderProxy();
  incoming
    ..connectToService(viewProvider)
    ..close();

  final viewTokens = EventPairPair();
  assert(viewTokens.status == ZX.OK);
  final viewHolderToken = ViewHolderToken(value: viewTokens.first);
  final viewToken = ViewToken(value: viewTokens.second);

  viewProvider.createView(viewToken.value, null, null);
  viewProvider.ctrl.close();

  return viewHolderToken;
}
