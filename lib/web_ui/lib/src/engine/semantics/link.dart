// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import '../dom.dart';
import '../semantics.dart';

/// Provides accessibility for links.
class Link extends PrimaryRoleManager {
  Link(SemanticsObject semanticsObject) : super.withBasics(
    PrimaryRole.link,
    semanticsObject,
    preferredLabelRepresentation: LabelRepresentation.domText,
  ) {
    addTappable();
  }

  @override
  DomElement createElement() {
    final DomElement element = domDocument.createElement('a');
    // TODO: The <a> element has `aria-label={value}`. We should stop that!
    element.setAttribute('href', semanticsObject.hasValue ? semanticsObject.value! : '#');
    element.style.display = 'block';
    return element;
  }

  @override
  bool focusAsRouteDefault() => focusable?.focusAsRouteDefault() ?? false;
}
