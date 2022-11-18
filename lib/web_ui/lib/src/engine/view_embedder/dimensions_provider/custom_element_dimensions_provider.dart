// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine/window.dart';
import 'package:ui/ui.dart' as ui show Size;

import '../../dom.dart';
import 'dimensions_provider.dart';

// import 'custom_element_application_dom.dart';
// import 'full_page_application_dom.dart';
// import 'hot_restart_cache_handler.dart';

/// This class provides the real-time dimensions of a "hostElement".
///
/// Note: all the measurements returned from this class are potentially
/// *expensive*, and should be cached as needed. Every call to every method on
/// this class WILL perform actual DOM measurements.
class CustomElementDimensionsProvider extends DimensionsProvider {

  CustomElementDimensionsProvider(this._hostElement);

  final DomElement _hostElement;

  @override
  ui.Size getPhysicalSize() {
    final double devicePixelRatio = getDevicePixelRatio();

    return ui.Size(
      _hostElement.clientWidth * devicePixelRatio,
      _hostElement.clientHeight * devicePixelRatio,
    );
  }

  @override
  WindowPadding getKeyboardInsets(double physicalHeight, bool isEditingOnMobile) {
    return const WindowPadding(
      top: 0,
      right: 0,
      bottom: 0,
      left: 0,
    );
  }
}
