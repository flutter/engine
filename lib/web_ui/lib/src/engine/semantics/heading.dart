// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import '../dom.dart';
import 'semantics.dart';

/// Renders semantics objects as headings with the corresponding
/// level (h1 ... h6).
class Heading extends PrimaryRoleManager {
  Heading(SemanticsObject semanticsObject)
      : super.withBasics(PrimaryRole.heading, semanticsObject) {
        addHeadingRole();
      }

  @override
  void update() {
    super.update();

    if (semanticsObject.headingLevel != -1) {
      addHeadingLevel(semanticsObject.headingLevel);
    } else {
      addHeadingLevel(1);
    }
  }

  void addHeadingRole() {
    semanticsObject.setAriaRole('heading');
  }

  void addHeadingLevel(int headingLevel) {
    semanticsObject.element.setAttribute('aria-level', headingLevel);
  }
}
