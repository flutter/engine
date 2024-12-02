// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'label_and_value.dart';
import 'semantics.dart';

/// Represents semantic objects that deliver information in a visual manner.
///
/// Uses aria tab role to convey this semantic information to the element.
///
/// Screen-readers takes advantage of "aria-label" to describe the visual.
class SemanticTab extends SemanticRole {
  SemanticTab(SemanticsObject semanticsObject)
      : super.withBasics(
          SemanticRoleKind.tab,
          semanticsObject,
          preferredLabelRepresentation: LabelRepresentation.ariaLabel,
        );
  @override
  bool focusAsRouteDefault() => focusable?.focusAsRouteDefault() ?? false;

  @override
  void update() {
    super.update();
    setAriaRole('tab');
  }
}

/// Represents semantic objects that deliver information in a visual manner.
///
/// Uses aria tab role to convey this semantic information to the element.
///
/// Screen-readers takes advantage of "aria-label" to describe the visual.
class SemanticTabPanel extends SemanticRole {
  SemanticTabPanel(SemanticsObject semanticsObject)
      : super.withBasics(
          SemanticRoleKind.tabPanel,
          semanticsObject,
          preferredLabelRepresentation: LabelRepresentation.ariaLabel,
        );
  @override
  bool focusAsRouteDefault() => focusable?.focusAsRouteDefault() ?? false;

  @override
  void update() {
    super.update();
    setAriaRole('tabpanel');
  }
}
