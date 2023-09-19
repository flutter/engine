// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import '../browser_detection.dart';
import '../dom.dart';
import 'semantics.dart';

/// If true, labels will apply a JAWS-specific workaround to make sure it can
/// read labels on leaf text nodes.
///
/// Even though the workaround is only needed for JAWS, the workaround is
/// applied on all Windows systems because we can't know in advance what screen
/// reader will be used, and NVDA is also OK with the workaround.
///
/// The workaround is _not_ used on other systems because it breaks the
/// navigation on some screen readers. For example, VoiceOver on macOS creates
/// intermediate nodes that result in the label read multiple times as the user
/// invokes next/previous actions. The workaround also adds extra performance
/// cost because it requires an extra span element.
///
/// See also:
///
///   * https://github.com/flutter/flutter/issues/122607
///   * https://github.com/FreedomScientific/standards-support/issues/759
bool useJawsWorkaroundForLabels = operatingSystem == OperatingSystem.windows;

/// Renders [SemanticsObject.label] and/or [SemanticsObject.value] to the semantics DOM.
///
/// VoiceOver supports "aria-label" but only in conjunction with an ARIA role.
/// Setting "aria-label" on an empty element without a role causes VoiceOver to
/// treat element as if it does not exist. VoiceOver supports role "text", which
/// is a proprietary role not supported by other browsers. Flutter Web still
/// uses it because it provides the best user experience for plain text nodes.
///
/// TalkBack supports standalone "aria-label" attribute, but does not support
/// role "text". This leads to TalkBack reading "group" or "empty group" on
/// plain text elements, but that's still better than other alternatives
/// considered.
///
/// The value is not always rendered. Some semantics nodes correspond to
/// interactive controls, such as an `<input>` element. In such case the value
/// is reported via that element's `value` attribute rather than rendering it
/// separately.
///
/// This role manager does not manage images and text fields. See
/// [ImageRoleManager] and [TextField].
class LabelAndValue extends RoleManager {
  LabelAndValue(SemanticsObject semanticsObject)
      : super(Role.labelAndValue, semanticsObject);

  @override
  void update() {
    final bool hasValue = semanticsObject.hasValue;
    final bool hasLabel = semanticsObject.hasLabel;
    final bool hasTooltip = semanticsObject.hasTooltip;

    // If the node is incrementable the value is reported to the browser via
    // the respective role manager. We do not need to also render it again here.
    final bool shouldDisplayValue = hasValue && !semanticsObject.isIncrementable;

    if (!hasLabel && !shouldDisplayValue && !hasTooltip) {
      _setLabel(null);
      return;
    }

    final StringBuffer combinedValue = StringBuffer();
    if (hasTooltip) {
      combinedValue.write(semanticsObject.tooltip);
      if (hasLabel || shouldDisplayValue) {
        combinedValue.write('\n');
      }
    }
    if (hasLabel) {
      combinedValue.write(semanticsObject.label);
      if (shouldDisplayValue) {
        combinedValue.write(' ');
      }
    }

    if (shouldDisplayValue) {
      combinedValue.write(semanticsObject.value);
    }

    _setLabel(combinedValue.toString());
  }

  void _setLabel(String? label) {
    // TODO(yjbanov): revisit when/if this is fixed https://github.com/FreedomScientific/standards-support/issues/759
    if (useJawsWorkaroundForLabels) {
      _setLabelWithWindowsWorkaround(label);
    } else {
      if (label != null && label.isNotEmpty) {
        semanticsObject.element.setAttribute('aria-label', label);
      } else {
        semanticsObject.element.removeAttribute('aria-label');
      }
    }
  }

  DomElement? _windowsSpan;

  void _setLabelWithWindowsWorkaround(String? label) {
    DomElement? span = _windowsSpan;
    if (label == null || label.isEmpty) {
      // Empty label => clean up
      if (span != null && (span.isConnected ?? false)) {
        span.remove();
      } // nothing to clean up in the else case
    } else {
      // Non-empty label => set the ARIA labels

      // If the span does not exist, create one
      if (span == null) {
        _windowsSpan = span = createDomElement('span');
      }
      span.setAttribute('aria-label', label);

      // If the span did exist but was detached, attach it.
      if (!(span.isConnected ?? false)) {
        semanticsObject.element.appendChild(span);
      }
    }
  }

  @override
  void dispose() {
    super.dispose();
    _setLabel(null);
  }
}
