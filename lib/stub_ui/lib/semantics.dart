// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Stub implementation. See docs in `../ui/`.
class SemanticsAction {
  const SemanticsAction._(this.index);

  static const int _kTapIndex = 1 << 0;
  static const int _kLongPressIndex = 1 << 1;
  static const int _kScrollLeftIndex = 1 << 2;
  static const int _kScrollRightIndex = 1 << 3;
  static const int _kScrollUpIndex = 1 << 4;
  static const int _kScrollDownIndex = 1 << 5;
  static const int _kIncreaseIndex = 1 << 6;
  static const int _kDecreaseIndex = 1 << 7;
  static const int _kShowOnScreenIndex = 1 << 8;
  static const int _kMoveCursorForwardByCharacterIndex = 1 << 9;
  static const int _kMoveCursorBackwardByCharacterIndex = 1 << 10;
  static const int _kSetSelectionIndex = 1 << 11;
  static const int _kCopyIndex = 1 << 12;
  static const int _kCutIndex = 1 << 13;
  static const int _kPasteIndex = 1 << 14;
  static const int _kDidGainAccessibilityFocusIndex = 1 << 15;
  static const int _kDidLoseAccessibilityFocusIndex = 1 << 16;
  static const int _kCustomAction = 1 << 17;
  static const int _kDismissIndex = 1 << 18;
  static const int _kMoveCursorForwardByWordIndex = 1 << 19;
  static const int _kMoveCursorBackwardByWordIndex = 1 << 20;

  /// Stub implementation. See docs in `../ui/`.
  final int index;

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction tap = const SemanticsAction._(_kTapIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction longPress = const SemanticsAction._(_kLongPressIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction scrollLeft = const SemanticsAction._(_kScrollLeftIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction scrollRight = const SemanticsAction._(_kScrollRightIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction scrollUp = const SemanticsAction._(_kScrollUpIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction scrollDown = const SemanticsAction._(_kScrollDownIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction increase = const SemanticsAction._(_kIncreaseIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction decrease = const SemanticsAction._(_kDecreaseIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction showOnScreen = const SemanticsAction._(_kShowOnScreenIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction moveCursorForwardByCharacter = const SemanticsAction._(_kMoveCursorForwardByCharacterIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction moveCursorBackwardByCharacter = const SemanticsAction._(_kMoveCursorBackwardByCharacterIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction setSelection = const SemanticsAction._(_kSetSelectionIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction copy = const SemanticsAction._(_kCopyIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction cut = const SemanticsAction._(_kCutIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction paste = const SemanticsAction._(_kPasteIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction didGainAccessibilityFocus = const SemanticsAction._(_kDidGainAccessibilityFocusIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction didLoseAccessibilityFocus = const SemanticsAction._(_kDidLoseAccessibilityFocusIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction customAction = const SemanticsAction._(_kCustomAction);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction dismiss = const SemanticsAction._(_kDismissIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction moveCursorForwardByWord = const SemanticsAction._(_kMoveCursorForwardByWordIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsAction moveCursorBackwardByWord = const SemanticsAction._(_kMoveCursorBackwardByWordIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const Map<int, SemanticsAction> values = const <int, SemanticsAction>{
    _kTapIndex: tap,
    _kLongPressIndex: longPress,
    _kScrollLeftIndex: scrollLeft,
    _kScrollRightIndex: scrollRight,
    _kScrollUpIndex: scrollUp,
    _kScrollDownIndex: scrollDown,
    _kIncreaseIndex: increase,
    _kDecreaseIndex: decrease,
    _kShowOnScreenIndex: showOnScreen,
    _kMoveCursorForwardByCharacterIndex: moveCursorForwardByCharacter,
    _kMoveCursorBackwardByCharacterIndex: moveCursorBackwardByCharacter,
    _kSetSelectionIndex: setSelection,
    _kCopyIndex: copy,
    _kCutIndex: cut,
    _kPasteIndex: paste,
    _kDidGainAccessibilityFocusIndex: didGainAccessibilityFocus,
    _kDidLoseAccessibilityFocusIndex: didLoseAccessibilityFocus,
    _kCustomAction: customAction,
    _kDismissIndex: dismiss,
    _kMoveCursorForwardByWordIndex: moveCursorForwardByWord,
    _kMoveCursorBackwardByWordIndex: moveCursorBackwardByWord,
  };

  @override
  String toString() {
    switch (index) {
      case _kTapIndex:
        return 'SemanticsAction.tap';
      case _kLongPressIndex:
        return 'SemanticsAction.longPress';
      case _kScrollLeftIndex:
        return 'SemanticsAction.scrollLeft';
      case _kScrollRightIndex:
        return 'SemanticsAction.scrollRight';
      case _kScrollUpIndex:
        return 'SemanticsAction.scrollUp';
      case _kScrollDownIndex:
        return 'SemanticsAction.scrollDown';
      case _kIncreaseIndex:
        return 'SemanticsAction.increase';
      case _kDecreaseIndex:
        return 'SemanticsAction.decrease';
      case _kShowOnScreenIndex:
        return 'SemanticsAction.showOnScreen';
      case _kMoveCursorForwardByCharacterIndex:
        return 'SemanticsAction.moveCursorForwardByCharacter';
      case _kMoveCursorBackwardByCharacterIndex:
        return 'SemanticsAction.moveCursorBackwardByCharacter';
      case _kSetSelectionIndex:
        return 'SemanticsAction.setSelection';
      case _kCopyIndex:
        return 'SemanticsAction.copy';
      case _kCutIndex:
        return 'SemanticsAction.cut';
      case _kPasteIndex:
        return 'SemanticsAction.paste';
      case _kDidGainAccessibilityFocusIndex:
        return 'SemanticsAction.didGainAccessibilityFocus';
      case _kDidLoseAccessibilityFocusIndex:
        return 'SemanticsAction.didLoseAccessibilityFocus';
      case _kCustomAction:
        return 'SemanticsAction.customAction';
      case _kDismissIndex:
        return 'SemanticsAction.dismiss';
      case _kMoveCursorForwardByWordIndex:
        return 'SemanticsAction.moveCursorForwardByWord';
      case _kMoveCursorBackwardByWordIndex:
        return 'SemanticsAction.moveCursorBackwardByWord';
    }
    return null;
  }
}

/// Stub implementation. See docs in `../ui/`.
class SemanticsFlag {
  static const int _kHasCheckedStateIndex = 1 << 0;
  static const int _kIsCheckedIndex = 1 << 1;
  static const int _kIsSelectedIndex = 1 << 2;
  static const int _kIsButtonIndex = 1 << 3;
  static const int _kIsTextFieldIndex = 1 << 4;
  static const int _kIsFocusedIndex = 1 << 5;
  static const int _kHasEnabledStateIndex = 1 << 6;
  static const int _kIsEnabledIndex = 1 << 7;
  static const int _kIsInMutuallyExclusiveGroupIndex = 1 << 8;
  static const int _kIsHeaderIndex = 1 << 9;
  static const int _kIsObscuredIndex = 1 << 10;
  static const int _kScopesRouteIndex= 1 << 11;
  static const int _kNamesRouteIndex = 1 << 12;
  static const int _kIsHiddenIndex = 1 << 13;
  static const int _kIsImageIndex = 1 << 14;
  static const int _kIsLiveRegionIndex = 1 << 15;
  static const int _kHasToggledStateIndex = 1 << 16;
  static const int _kIsToggledIndex = 1 << 17;
  static const int _kHasImplicitScrollingIndex = 1 << 18;

  const SemanticsFlag._(this.index);

  /// Stub implementation. See docs in `../ui/`.
  final int index;

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag hasCheckedState = const SemanticsFlag._(_kHasCheckedStateIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isChecked = const SemanticsFlag._(_kIsCheckedIndex);


  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isSelected = const SemanticsFlag._(_kIsSelectedIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isButton = const SemanticsFlag._(_kIsButtonIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isTextField = const SemanticsFlag._(_kIsTextFieldIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isFocused = const SemanticsFlag._(_kIsFocusedIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag hasEnabledState = const SemanticsFlag._(_kHasEnabledStateIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isEnabled = const SemanticsFlag._(_kIsEnabledIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isInMutuallyExclusiveGroup = const SemanticsFlag._(_kIsInMutuallyExclusiveGroupIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isHeader = const SemanticsFlag._(_kIsHeaderIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isObscured = const SemanticsFlag._(_kIsObscuredIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag scopesRoute = const SemanticsFlag._(_kScopesRouteIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag namesRoute = const SemanticsFlag._(_kNamesRouteIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isHidden = const SemanticsFlag._(_kIsHiddenIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isImage = const SemanticsFlag._(_kIsImageIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isLiveRegion = const SemanticsFlag._(_kIsLiveRegionIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag hasToggledState = const SemanticsFlag._(_kHasToggledStateIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag isToggled = const SemanticsFlag._(_kIsToggledIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const SemanticsFlag hasImplicitScrolling = const SemanticsFlag._(_kHasImplicitScrollingIndex);

  /// Stub implementation. See docs in `../ui/`.
  static const Map<int, SemanticsFlag> values = const <int, SemanticsFlag>{
    _kHasCheckedStateIndex: hasCheckedState,
    _kIsCheckedIndex: isChecked,
    _kIsSelectedIndex: isSelected,
    _kIsButtonIndex: isButton,
    _kIsTextFieldIndex: isTextField,
    _kIsFocusedIndex: isFocused,
    _kHasEnabledStateIndex: hasEnabledState,
    _kIsEnabledIndex: isEnabled,
    _kIsInMutuallyExclusiveGroupIndex: isInMutuallyExclusiveGroup,
    _kIsHeaderIndex: isHeader,
    _kIsObscuredIndex: isObscured,
    _kScopesRouteIndex: scopesRoute,
    _kNamesRouteIndex: namesRoute,
    _kIsHiddenIndex: isHidden,
    _kIsImageIndex: isImage,
    _kIsLiveRegionIndex: isLiveRegion,
    _kHasToggledStateIndex: hasToggledState,
    _kIsToggledIndex: isToggled,
    _kHasImplicitScrollingIndex: hasImplicitScrolling,
  };

  @override
  String toString() {
    switch (index) {
      case _kHasCheckedStateIndex:
        return 'SemanticsFlag.hasCheckedState';
      case _kIsCheckedIndex:
        return 'SemanticsFlag.isChecked';
      case _kIsSelectedIndex:
        return 'SemanticsFlag.isSelected';
      case _kIsButtonIndex:
        return 'SemanticsFlag.isButton';
      case _kIsTextFieldIndex:
        return 'SemanticsFlag.isTextField';
      case _kIsFocusedIndex:
        return 'SemanticsFlag.isFocused';
      case _kHasEnabledStateIndex:
        return 'SemanticsFlag.hasEnabledState';
      case _kIsEnabledIndex:
        return 'SemanticsFlag.isEnabled';
      case _kIsInMutuallyExclusiveGroupIndex:
        return 'SemanticsFlag.isInMutuallyExclusiveGroup';
      case _kIsHeaderIndex:
        return 'SemanticsFlag.isHeader';
      case _kIsObscuredIndex:
        return 'SemanticsFlag.isObscured';
      case _kScopesRouteIndex:
        return 'SemanticsFlag.scopesRoute';
      case _kNamesRouteIndex:
        return 'SemanticsFlag.namesRoute';
      case _kIsHiddenIndex:
        return 'SemanticsFlag.isHidden';
      case _kIsImageIndex:
        return 'SemanticsFlag.isImage';
      case _kIsLiveRegionIndex:
        return 'SemanticsFlag.isLiveRegion';
      case _kHasToggledStateIndex:
        return 'SemanticsFlag.hasToggledState';
      case _kIsToggledIndex:
        return 'SemanticsFlag.isToggled';
      case _kHasImplicitScrollingIndex:
        return 'SemanticsFlag.hasImplicitScrolling';
    }
    return null;
  }
}

/// Stub implementation. See docs in `../ui/`.
class SemanticsUpdateBuilder {
  /// Stub implementation. See docs in `../ui/`.
  SemanticsUpdateBuilder();

  /// Stub implementation. See docs in `../ui/`.
  void updateNode({
    int id,
    int flags,
    int actions,
    int textSelectionBase,
    int textSelectionExtent,
    int platformViewId,
    int scrollChildren,
    int scrollIndex,
    double scrollPosition,
    double scrollExtentMax,
    double scrollExtentMin,
    double elevation,
    double thickness,
    Rect rect,
    String label,
    String hint,
    String value,
    String increasedValue,
    String decreasedValue,
    TextDirection textDirection,
    Float64List transform,
    Int32List childrenInTraversalOrder,
    Int32List childrenInHitTestOrder,
    Int32List additionalActions,
  }) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void updateCustomAction({int id, String label, String hint, int overrideId = -1}) {
    assert(id != null);
    assert(overrideId != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  SemanticsUpdate build() {
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
class SemanticsUpdate {
  /// Stub implementation. See docs in `../ui/`.
  SemanticsUpdate._();

  /// Stub implementation. See docs in `../ui/`.
  void dispose() {
    throw UnimplementedError();
  }
}
