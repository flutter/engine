// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import '../dom.dart';
import 'label_and_value.dart';
import 'semantics.dart';

/// Renders semantics objects as headings with the corresponding
/// level (h1 ... h6).
class Heading extends PrimaryRoleManager {
  Heading(SemanticsObject semanticsObject)
      : super.blank(PrimaryRole.heading, semanticsObject) {
    addFocusManagement();
    addLiveRegion();
    addRouteName();
    addLabelAndValue(preferredRepresentation: LabelRepresentation.domText);
  }

  @override
  DomElement createElement() => createDomElement('h${semanticsObject.headingLevel}');

  /// Focuses on this heading element if it turns out to be the default element
  /// of a route.
  ///
  /// Normally, heading elements are not focusable as they do not receive
  /// keyboard input. However, when a route is pushed (e.g. a dialog pops up),
  /// then it may be desirable to move the screen reader focus to the heading
  /// that explains the contents of the route to the user. This method makes the
  /// element artificially focusable and moves the screen reader focus to it.
  ///
  /// That said, if the node is formally focusable, then the focus is
  /// transferred using [Focusable].
  @override
  bool focusAsRouteDefault() {
    if (semanticsObject.isFocusable) {
      final focusable = this.focusable;
      if (focusable != null) {
        return focusable.focusAsRouteDefault();
      }
    }

    labelAndValue!.focusAsRouteDefault();
    return true;
  }
}
