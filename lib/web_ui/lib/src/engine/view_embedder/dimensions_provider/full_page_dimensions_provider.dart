// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine/window.dart';
import 'package:ui/ui.dart' as ui show Size;

import '../../browser_detection.dart';
import '../../dom.dart';
import 'dimensions_provider.dart';

/// This class provides the real-time dimensions of a "full page" viewport.
///
/// Note: all the measurements returned from this class are potentially
/// *expensive*, and should be cached as needed. Every call to every method on
/// this class WILL perform actual DOM measurements.
class FullPageDimensionsProvider extends DimensionsProvider {

  @override
  ui.Size getPhysicalSize() {
    late double windowInnerWidth;
    late double windowInnerHeight;
    final DomVisualViewport? viewport = domWindow.visualViewport;
    final double devicePixelRatio = getDevicePixelRatio();

    if (viewport != null) {
      if (operatingSystem == OperatingSystem.iOs) {
        /// Chrome on iOS reports incorrect viewport.height when app
        /// starts in portrait orientation and the phone is rotated to
        /// landscape.
        ///
        /// We instead use documentElement clientWidth/Height to read
        /// accurate physical size. VisualViewport api is only used during
        /// text editing to make sure inset is correctly reported to
        /// framework.
        final double docWidth = domDocument.documentElement!.clientWidth;
        final double docHeight = domDocument.documentElement!.clientHeight;
        windowInnerWidth = docWidth * devicePixelRatio;
        windowInnerHeight = docHeight * devicePixelRatio;
      } else {
        windowInnerWidth = viewport.width! * devicePixelRatio;
        windowInnerHeight = viewport.height! * devicePixelRatio;
      }
    } else {
      windowInnerWidth = domWindow.innerWidth! * devicePixelRatio;
      windowInnerHeight = domWindow.innerHeight! * devicePixelRatio;
    }
    return ui.Size(
      windowInnerWidth,
      windowInnerHeight,
    );
  }

  @override
  WindowPadding getKeyboardInsets(double physicalHeight, bool isEditingOnMobile) {
    final double devicePixelRatio = getDevicePixelRatio();
    final DomVisualViewport? viewport = domWindow.visualViewport;
    late double windowInnerHeight;

    if (viewport != null) {
      if (operatingSystem == OperatingSystem.iOs && !isEditingOnMobile) {
        windowInnerHeight =
            domDocument.documentElement!.clientHeight * devicePixelRatio;
      } else {
        windowInnerHeight = viewport.height! * devicePixelRatio;
      }
    } else {
      windowInnerHeight = domWindow.innerHeight! * devicePixelRatio;
    }
    final double bottomPadding = physicalHeight - windowInnerHeight;

    return WindowPadding(bottom: bottomPadding, left: 0, right: 0, top: 0);
  }
}
