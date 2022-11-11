// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:ui/ui.dart' as ui;

import '../dom.dart';
import 'application_dom.dart';

class CustomElementApplicationDom extends ApplicationDom {

  CustomElementApplicationDom(this._hostElement);

  final DomElement _hostElement;

  void applyViewportMeta() {}
  void createGlassPane() {}
  void createSceneHost() {}
  void createSemanticsHost() {}
  void prepareAccessibilityPlaceholder() {}
  void assembleGlassPane() {}

  void addScene(DomElement? sceneElement) {}

  void setMetricsChangeHandler(void Function(DomEvent? event) handler) {}
  void setLanguageChangeHandler(void Function(DomEvent event) handler) {}
  void registerPostHotRestartCleanup(List<DomElement> elements) {}

}
