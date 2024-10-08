// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import '../dom.dart';
import 'label_and_value.dart';
import 'semantics.dart';

/// Renders a semantic header.
///
/// A header is a group of nodes that together introduce the content of the
/// current screen or page.
///
/// Uses the `<header>` element, which implies ARIA role "banner".
///
/// See also:
///   * https://developer.mozilla.org/en-US/docs/Web/HTML/Element/header
///   * https://developer.mozilla.org/en-US/docs/Web/Accessibility/ARIA/Roles/banner_role
class SemanticHeader extends SemanticRole {
  SemanticHeader(SemanticsObject semanticsObject) : super.withBasics(
    SemanticRoleKind.header,
    semanticsObject,
    preferredLabelRepresentation: LabelRepresentation.ariaLabel,
  );

  @override
  DomElement createElement() => createDomElement('header');

  @override
  bool focusAsRouteDefault() => focusable?.focusAsRouteDefault() ?? false;
}
