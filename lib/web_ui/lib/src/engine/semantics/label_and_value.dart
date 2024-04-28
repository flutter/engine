// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../dom.dart';
import 'semantics.dart';

/// The method used to represend a label of a leaf node in the DOM.
///
/// This is required by some screen readers and web crawlers.
///
/// Container nodes only use `aria-label`, even if [domText] is chosen. This is
/// because screen readers treat container nodes as "groups" of other nodes, and
/// respect the `aria-label` without a [DomText] node. Crawlers typically do not
/// need this information, as they primarily scan visible text, which is
/// communicated in semantics as leaf text and heading nodes.
enum LabelRepresentation {
  /// Represents the label as an `aria-label` attribute.
  ///
  /// This representation is the most efficient as all it does is pass a string
  /// to the browser that does not incur any DOM costs.
  ///
  /// The drawback of this representation is that it is not compatible with most
  /// web crawlers, and for some ARIA roles (including the implicit "generic"
  /// role) JAWS on Windows. However, this role is still the most common, as it
  /// applies to all container nodes, and many ARIA roles (e.g. checkboxes,
  /// radios, scrollables, sliders).
  ariaLabel(AriaLabelRepresentation),

  /// Represents the label as a [DomText] node.
  ///
  /// This is the second fastest way to represent a label in the DOM. It has a
  /// small cost because the browser lays out the text (in addition to Flutter
  /// having already done it).
  ///
  /// This representation is compatible with most web crawlers, and it is the
  /// best option for certain ARIA roles, such as buttons, links, and headings.
  domText(DomTextRepresentation),

  /// Represents the label as a sized span.
  ///
  /// This representation is the costliest as it uses an extra element that
  /// need to be laid out to compute the right size. It is compatible with most
  /// web crawlers, and it is the best options for certain ARIA roles, such as
  /// the implicit "generic" role used for plain text (not headings).
  sizedSpan(SizedSpanRepresentation);

  const LabelRepresentation(this.implementation);

  /// The type used to implement this representation.
  final Type implementation;
}

/// Provides a DOM behavior for a [LabelRepresentation].
abstract final class LabelRepresentationBehavior {
  LabelRepresentationBehavior(this.owner);

  final PrimaryRoleManager owner;

  SemanticsObject get semanticsObject => owner.semanticsObject;

  void update(String label);

  void cleanUp();
}

/// Sets the label as `aria-label`.
///
/// Example:
///
///     <flt-semantics aria-label="Hello, World!"></flt-semantics>
final class AriaLabelRepresentation extends LabelRepresentationBehavior {
  AriaLabelRepresentation._(super.owner);

  String? _previousLabel;

  @override
  void update(String label) {
    if (label == _previousLabel) {
      return;
    }
    owner.setAttribute('aria-label', label);
  }

  @override
  void cleanUp() {
    owner.removeAttribute('aria-label');
  }
}

/// Sets the label as text inside the DOM element.
///
/// Example:
///
///     <flt-semantics>Hello, World!</flt-semantics>
///
/// This representation is used when the ARIA role of the element already sizes
/// the element and therefore no extra sizing assistance is needed. If there is
/// no ARIA role set, or the role does not size the element, then the
/// [SizedSpanRepresentation] representation can be used.
final class DomTextRepresentation extends LabelRepresentationBehavior {
  DomTextRepresentation._(super.owner);

  DomText? _domText;
  String? _previousLabel;

  @override
  void update(String label) {
    if (label == _previousLabel) {
      return;
    }

    _domText?.remove();
    final DomText domText = domDocument.createTextNode(label);
    _domText = domText;
    semanticsObject.element.appendChild(domText);
  }

  @override
  void cleanUp() {
    _domText?.remove();
  }
}

/// Sets the label as the text of a `<span>` child element.
///
/// The span element is scaled to match the size of the semantic node.
///
/// Example:
///
///     <flt-semantics>
///       <span style="transform: scale(2, 2)">Hello, World!</span>
///     </flt-semantics>
///
/// Text scaling is used to control the size of the screen reader focus ring.
/// This is used for plain text nodes (e.g. paragraphs of text).
final class SizedSpanRepresentation extends LabelRepresentationBehavior {
  SizedSpanRepresentation._(super.owner) {
    _domText.style
      // `inline-block` is needed for two reasons:
      // - It supports measuring the true size of the text. Pure `block` would
      //   disassosiate the size of the text from the size of the element.
      // - It supports the `transform` and `transform-origin` properties. Pure
      //   `inline` does not support them.
      ..display = 'inline-block'

      // Stretch the text full width because if the text is broken up into a
      // multi-line paragraph the width of the paragraph can become smaller than
      // the offsetWidth of the element, and therefore not fully cover the rect
      // of the parent. "justify" will ensure the text is stretched edge to edge
      // covering full offsetWidth.
      ..textAlign = 'justify'

      // The origin of the coordinate system is the top-left corner of the
      // parent element.
      ..transformOrigin = '0 0 0';
    semanticsObject.element.appendChild(_domText);
  }

  final DomElement _domText = domDocument.createElement('span');
  String? _previousLabel;
  ui.Size? _previousSize;

  @override
  void update(String label) {
    final ui.Size? size = semanticsObject.rect?.size;
    final bool labelChanged = label != _previousLabel;
    final bool sizeChanged = size != _previousSize;

    // Label must be updated before sizing because the size depend on text
    // content.
    if (labelChanged) {
      _domText.innerText = label;
    }

    // This code makes the assumption that the DOM size of the element depends
    // solely on the text of the label. This is because text in the semantics
    // tree is unstyled. If this ever changes, this assumption will no longer
    // hold, and this code will need to be updated.
    if (labelChanged || sizeChanged) {
      _updateSize(size);
    }

    // Remember the last used data to shut off unnecessary updates.
    _previousLabel = label;
    _previousSize = size;
  }

  // Scales the text span (if any), such that the text matches the size of the
  // node. This is important because screen reader focus sizes itself tightly
  // around the text. Frequently, Flutter wants the focus to be different from
  // the text itself. For example, when you focus on a card widget containing a
  // piece of text, it is desirable that the focus covers the whole card, and
  // not just the text inside.
  //
  // The scaling may cause the text to become distorted, but that doesn't matter
  // because the semantic DOM is invisible.
  //
  // See: https://github.com/flutter/flutter/issues/146774
  void _updateSize(ui.Size? size) {
    if (size == null) {
      // There's no size to match => remove whatever stale sizing information was there.
      _domText.style.transform = '';
      return;
    }

    // Perform the adjustment in a post-update callback because the DOM layout
    // can only be performed when the elements are attached to the document.
    semanticsObject.owner.addOneTimePostUpdateCallback(() {
      // Both clientWidth/Height and offsetWidth/Height provide a good
      // approximation for the purposes of sizing the focus ring of the text,
      // since there's no borders or scrollbars. The `offset` variant was chosen
      // mostly because it rounds the value to `int`, so the value is less
      // volatile and therefore would need fewer updates.
      //
      // getBoundingClientRect() was considered and rejected, because it provides
      // the rect in screen coordinates but this scale adjustment needs to be
      // local.
      final double domWidth = _domText.offsetWidth;
      final double domHeight = _domText.offsetHeight;

      if (domWidth < 1 && domHeight < 1) {
        // Don't bother dealing with divisions by tiny numbers. This probably means
        // the label is empty or doesn't contain anything that would be visible to
        // the user.
        _domText.style.transform = '';
      } else {
        final double scaleX = size.width / domWidth;
        final double scaleY = size.height / domHeight;
        _domText.style.transform = 'scale($scaleX, $scaleY)';
      }
    });
  }

  @override
  void cleanUp() {
    _domText.remove();
  }
}

/// Renders [SemanticsObject.label] and/or [SemanticsObject.value] to the semantics DOM.
///
/// The value is not always rendered. Some semantics nodes correspond to
/// interactive controls. In such case the value is reported via that element's
/// `value` attribute rather than rendering it separately.
class LabelAndValue extends RoleManager {
  LabelAndValue(SemanticsObject semanticsObject, PrimaryRoleManager owner, { required this.labelRepresentation })
      : super(Role.labelAndValue, semanticsObject, owner);

  // TODO(yjbanov): rename to `preferredRepresentation` because it's not guaranteed.
  /// The preferred representation of the label in the DOM.
  ///
  /// This value may be changed. Calling [update] after changing it will apply
  /// the new preference.
  ///
  /// If the node contains children, [LabelRepresentation.ariaLabel] is used
  /// instead.
  LabelRepresentation labelRepresentation;

  @override
  void update() {
    final String? computedLabel = _computeLabel();

    if (computedLabel == null) {
      _cleanUpDom();
      return;
    }

    _getEffectiveRepresentation().update(computedLabel);
  }

  LabelRepresentationBehavior? _representation;

  /// Return the representation that should be used based on the current
  /// parameters of the semantic node.
  ///
  /// If the node has children always use an `aria-label`. Using extra child
  /// nodes to represent the label will cause layout shifts and confuse the
  /// screen reader. If the are no children, use the representation preferred
  /// by the primary role manager.
  LabelRepresentationBehavior _getEffectiveRepresentation() {
    final LabelRepresentation effectiveRepresentation = semanticsObject.hasChildren
      ? LabelRepresentation.ariaLabel
      : labelRepresentation;

    LabelRepresentationBehavior? representation = _representation;
    if (representation == null || representation.runtimeType != effectiveRepresentation.runtimeType) {
      representation?.cleanUp();
      _representation = representation = switch (effectiveRepresentation) {
        LabelRepresentation.ariaLabel => AriaLabelRepresentation._(owner),
        LabelRepresentation.domText => DomTextRepresentation._(owner),
        LabelRepresentation.sizedSpan => SizedSpanRepresentation._(owner),
      };
    }
    return representation;
  }

  /// Computes the final label to be assigned to the node.
  ///
  /// The label is a concatenation of tooltip, label, hint, and value, whichever
  /// combination is present.
  String? _computeLabel() {
    // If the node is incrementable the value is reported to the browser via
    // the respective role manager. We do not need to also render it again here.
    final bool shouldDisplayValue = !semanticsObject.isIncrementable && semanticsObject.hasValue;

    return computeDomSemanticsLabel(
      tooltip: semanticsObject.hasTooltip ? semanticsObject.tooltip : null,
      label: semanticsObject.hasLabel ? semanticsObject.label : null,
      hint: semanticsObject.hint,
      value: shouldDisplayValue ? semanticsObject.value : null,
    );
  }

  void _cleanUpDom() {
    _representation?.cleanUp();
  }

  @override
  void dispose() {
    super.dispose();
    _cleanUpDom();
  }
}

String? computeDomSemanticsLabel({
  String? tooltip,
  String? label,
  String? hint,
  String? value,
}) {
  final String? labelHintValue = _computeLabelHintValue(label: label, hint: hint, value: value);

  if (tooltip == null && labelHintValue == null) {
    return null;
  }

  final StringBuffer combinedValue = StringBuffer();
  if (tooltip != null) {
    combinedValue.write(tooltip);

    // Separate the tooltip from the rest via a line-break (if the rest exists).
    if (labelHintValue != null) {
      combinedValue.writeln();
    }
  }

  if (labelHintValue != null) {
    combinedValue.write(labelHintValue);
  }

  return combinedValue.isNotEmpty ? combinedValue.toString() : null;
}

String? _computeLabelHintValue({
  String? label,
  String? hint,
  String? value,
}) {
  final String combinedValue = <String?>[label, hint, value]
    .whereType<String>() // poor man's null filter
    .where((String element) => element.trim().isNotEmpty)
    .join(' ');
  return combinedValue.isNotEmpty ? combinedValue : null;
}
