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

  @override
  final String type = 'custom-element';

  void applyViewportMeta() {}
  void setHostStyles({
    required String font,
  }) {}
  void setHostAttribute(String name, String value) {}
  void attachGlassPane(DomElement glassPaneElement) {}
  void attachResourcesHost(DomElement resourceHost, {DomElement? nextTo }) {}

  void setMetricsChangeHandler(void Function(DomEvent? event) handler) {}
  void setLanguageChangeHandler(void Function(DomEvent event) handler) {}
  void registerPostHotRestartCleanup(List<DomElement> elements) {}
}
