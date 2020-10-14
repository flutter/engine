// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.10

import 'package:quiver/testing/async.dart';
import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;
import 'package:meta/meta.dart' show isTest;

const int kPhysicalKeyA = 0x00070004;
const int kPhysicalKeyE = 0x00070008;
const int kPhysicalKeyU = 0x00070018;
const int kPhysicalShiftLeft = 0x000700e1;
const int kPhysicalShiftRight = 0x000700e5;
const int kPhysicalTab = 0x0007002b;
const int kPhysicalCapsLock = 0x00070039;

const int kLogicalLowerA = 0x00000000061;
const int kLogicalUpperA = 0x00000000041;
const int kLogicalLowerU = 0x00000000075;
const int kLogicalShift = 0x000000010d;
const int kLogicalTab = 0x0000000009;
const int kLogicalCapsLock = 0x00000000104;

typedef VoidCallback = void Function();

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  test('Single key press, repeat, and release', () {
    final List<ui.KeyData> keyDataList = <ui.KeyData>[];
    final KeyboardConverter converter = KeyboardConverter((ui.KeyData key) {
      keyDataList.add(key);
      return true;
    });
    bool preventedDefault = false;
    final onPreventDefault = () { preventedDefault = true; };

    converter.handleEvent(keyDownEvent('KeyA', 'a')
      ..timeStamp = 1
      ..onPreventDefault = onPreventDefault
    );
    expectKeyData(keyDataList.last,
      timeStamp: Duration(milliseconds: 1),
      change: ui.KeyChange.down,
      key: kPhysicalKeyA,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalLowerA, character: 'a'),
      ],
    );
    expect(preventedDefault, false);

    converter.handleEvent(keyRepeatedDownEvent('KeyA', 'a')
      ..timeStamp = 1.5
      ..onPreventDefault = onPreventDefault
    );
    expectKeyData(keyDataList.last,
      timeStamp: Duration(milliseconds: 1, microseconds: 500),
      change: ui.KeyChange.repeatedDown,
      key: kPhysicalKeyA,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.repeatedDown, key: kLogicalLowerA, character: 'a'),
      ],
    );
    expect(preventedDefault, false);

    converter.handleEvent(keyRepeatedDownEvent('KeyA', 'a')
      ..timeStamp = 1500
      ..onPreventDefault = onPreventDefault
    );
    expectKeyData(keyDataList.last,
      timeStamp: Duration(seconds: 1, milliseconds: 500),
      change: ui.KeyChange.repeatedDown,
      key: kPhysicalKeyA,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.repeatedDown, key: kLogicalLowerA, character: 'a'),
      ],
    );
    expect(preventedDefault, false);

    converter.handleEvent(keyUpEvent('KeyA', 'a')
      ..timeStamp = 2000.5
      ..onPreventDefault = onPreventDefault
    );
    expectKeyData(keyDataList.last,
      timeStamp: Duration(seconds: 2, microseconds: 500),
      change: ui.KeyChange.up,
      key: kPhysicalKeyA,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.up, key: kLogicalLowerA),
      ],
    );
    expect(preventedDefault, false);
  });

  test('Release modifier during a repeated sequence', () {
    final List<ui.KeyData> keyDataList = <ui.KeyData>[];
    final KeyboardConverter converter = KeyboardConverter((ui.KeyData key) {
      keyDataList.add(key);
      return true;
    });

    converter.handleEvent(keyDownEvent('ShiftLeft', 'Shift', kShift));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalShiftLeft,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalShift),
      ],
    );

    converter.handleEvent(keyDownEvent('KeyA', 'A', kShift));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalKeyA,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalUpperA, character: 'A'),
      ],
    );

    converter.handleEvent(keyRepeatedDownEvent('KeyA', 'A', kShift));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.repeatedDown,
      key: kPhysicalKeyA,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.repeatedDown, key: kLogicalUpperA, character: 'A'),
      ],
    );

    converter.handleEvent(keyUpEvent('ShiftLeft', 'Shift'));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.up,
      key: kPhysicalShiftLeft,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.up, key: kLogicalShift),
      ],
    );

    converter.handleEvent(keyRepeatedDownEvent('KeyA', 'a'));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.repeatedDown,
      key: kPhysicalKeyA,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.cancel, key: kLogicalUpperA),
        ui.LogicalKeyData(change: ui.KeyChange.synchronize, key: kLogicalLowerA, character: 'a'),
      ],
    );

    converter.handleEvent(keyRepeatedDownEvent('KeyA', 'a'));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.repeatedDown,
      key: kPhysicalKeyA,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.repeatedDown, key: kLogicalLowerA, character: 'a'),
      ],
    );

    converter.handleEvent(keyUpEvent('KeyA', 'a'));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.up,
      key: kPhysicalKeyA,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.up, key: kLogicalLowerA),
      ],
    );
  });

  test('Same logical key mapped to multiple physical keys', () {
    final List<ui.KeyData> keyDataList = <ui.KeyData>[];
    final KeyboardConverter converter = KeyboardConverter((ui.KeyData key) {
      keyDataList.add(key);
      return true;
    });

    converter.handleEvent(keyDownEvent('ShiftLeft', 'Shift', kShift));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalShiftLeft,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalShift),
      ],
    );

    converter.handleEvent(keyDownEvent('ShiftRight', 'Shift', kShift));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalShiftRight,
      logical: <ui.LogicalKeyData>[],
    );

    converter.handleEvent(keyUpEvent('ShiftLeft', 'Shift', kShift));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.up,
      key: kPhysicalShiftLeft,
      logical: <ui.LogicalKeyData>[],
    );

    converter.handleEvent(keyUpEvent('ShiftRight', 'Shift'));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.up,
      key: kPhysicalShiftRight,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.up, key: kLogicalShift),
      ],
    );
  });

  test('Prevents default when pressing Tab', () {
    final List<ui.KeyData> keyDataList = <ui.KeyData>[];
    final KeyboardConverter converter = KeyboardConverter((ui.KeyData key) {
      keyDataList.add(key);
      return true;
    });
    bool preventedDefault = false;
    final onPreventDefault = () { preventedDefault = true; };

    converter.handleEvent(keyDownEvent('Tab', 'Tab')..onPreventDefault = onPreventDefault);
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalTab,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalTab),
      ],
    );
    expect(preventedDefault, true);
    preventedDefault = false;

    converter.handleEvent(keyUpEvent('Tab', 'Tab')..onPreventDefault = onPreventDefault);
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.up,
      key: kPhysicalTab,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.up, key: kLogicalTab),
      ],
    );
    expect(preventedDefault, true);
    preventedDefault = false;
  });

  test('Dead keys are distinguishable', () {
    final List<ui.KeyData> keyDataList = <ui.KeyData>[];
    final KeyboardConverter converter = KeyboardConverter((ui.KeyData key) {
      keyDataList.add(key);
      return true;
    });

    // The absolute values of the following logical keys are not guaranteed.
    const int kLogicalAltE = 0x410800070008;
    const int kLogicalAltU = 0x410800070018;
    const int kLogicalAltShiftE = 0x610800070008;
    // The values must be distinguishable.
    expect(kLogicalAltE, isNot(equals(kLogicalAltU)));
    expect(kLogicalAltE, isNot(equals(kLogicalAltShiftE)));

    converter.handleEvent(keyDownEvent('AltLeft', 'Alt', kAlt));

    converter.handleEvent(keyDownEvent('KeyE', 'Dead', kAlt));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalKeyE,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalAltE),
      ],
    );

    converter.handleEvent(keyUpEvent('KeyE', 'Dead', kAlt));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.up,
      key: kPhysicalKeyE,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.up, key: kLogicalAltE),
      ],
    );

    converter.handleEvent(keyDownEvent('KeyU', 'Dead', kAlt));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalKeyU,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalAltU),
      ],
    );

    converter.handleEvent(keyUpEvent('KeyU', 'Dead', kAlt));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.up,
      key: kPhysicalKeyU,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.up, key: kLogicalAltU),
      ],
    );

    converter.handleEvent(keyDownEvent('ShiftLeft', 'Shift', kAlt | kShift));

    // This does not actually produce a Dead key on macOS (US layout); just for
    // testing.
    converter.handleEvent(keyDownEvent('KeyE', 'Dead', kAlt | kShift));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalKeyE,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalAltShiftE),
      ],
    );

    converter.handleEvent(keyUpEvent('AltLeft', 'Alt', kShift));

    converter.handleEvent(keyUpEvent('KeyE', 'e', kShift));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.up,
      key: kPhysicalKeyE,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.up, key: kLogicalAltShiftE),
      ],
    );

    converter.handleEvent(keyUpEvent('ShiftLeft', 'Shift'));
  });

  test('Duplicate down cancels the previous one physically', () {
    final List<ui.KeyData> keyDataList = <ui.KeyData>[];
    final KeyboardConverter converter = KeyboardConverter((ui.KeyData key) {
      keyDataList.add(key);
      return true;
    });

    converter.handleEvent(keyDownEvent('ShiftLeft', 'Shift', kShift));
    // A KeyUp of ShiftLeft is missed due to loss of focus.

    keyDataList.clear();
    converter.handleEvent(keyDownEvent('ShiftLeft', 'Shift', kShift));
    expect(keyDataList, hasLength(2));
    expectKeyData(keyDataList.first,
      change: ui.KeyChange.cancel,
      key: kPhysicalShiftLeft,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.cancel, key: kLogicalShift),
      ],
    );
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalShiftLeft,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalShift),
      ],
    );
    keyDataList.clear();

    converter.handleEvent(keyUpEvent('ShiftLeft', 'Shift'));
    expect(keyDataList, hasLength(1));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.up,
      key: kPhysicalShiftLeft,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.up, key: kLogicalShift),
      ],
    );
  });

  test('Duplicate ups are skipped', () {
    final List<ui.KeyData> keyDataList = <ui.KeyData>[];
    final KeyboardConverter converter = KeyboardConverter((ui.KeyData key) {
      keyDataList.add(key);
      return true;
    });

    // A KeyDown of ShiftRight is missed due to loss of focus.
    converter.handleEvent(keyUpEvent('ShiftRight', 'Shift'));
    expect(keyDataList, isEmpty);
  });

  test('Conflict from multiple keyboards do not crash', () {
    final List<ui.KeyData> keyDataList = <ui.KeyData>[];
    final KeyboardConverter converter = KeyboardConverter((ui.KeyData key) {
      keyDataList.add(key);
      return true;
    });

    // Same layout
    converter.handleEvent(keyDownEvent('KeyA', 'a'));
    converter.handleEvent(keyDownEvent('KeyA', 'a'));
    converter.handleEvent(keyUpEvent('KeyA', 'a'));
    converter.handleEvent(keyUpEvent('KeyA', 'a'));

    // Different layout
    converter.handleEvent(keyDownEvent('KeyA', 'a'));
    converter.handleEvent(keyDownEvent('KeyA', 'u'));
    converter.handleEvent(keyUpEvent('KeyA', 'u'));
    converter.handleEvent(keyUpEvent('KeyA', 'a'));

    // Passes if there's no crash, and states are reset after everything is released.
    keyDataList.clear();
    converter.handleEvent(keyDownEvent('KeyA', 'a'));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalKeyA,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalLowerA, character: 'a'),
      ],
    );

    converter.handleEvent(keyDownEvent('KeyU', 'u'));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalKeyU,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalLowerU, character: 'u'),
      ],
    );
  });

  testFakeAsync('CapsLock down synthesizes an immediate cancel on macOS', (FakeAsync async) {
    final List<ui.KeyData> keyDataList = <ui.KeyData>[];
    final KeyboardConverter converter = KeyboardConverter((ui.KeyData key) {
      keyDataList.add(key);
      return true;
    }, onMacOs: true);

    converter.handleEvent(keyDownEvent('CapsLock', 'CapsLock'));
    expect(keyDataList, hasLength(1));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalCapsLock,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalCapsLock),
      ],
    );
    keyDataList.clear();

    async.elapse(Duration(microseconds: 1));
    expect(keyDataList, hasLength(1));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.cancel,
      key: kPhysicalCapsLock,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.cancel, key: kLogicalCapsLock),
      ],
    );
    keyDataList.clear();

    converter.handleEvent(keyUpEvent('CapsLock', 'CapsLock'));
    expect(keyDataList, hasLength(1));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalCapsLock,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalCapsLock),
      ],
    );
    keyDataList.clear();

    async.elapse(Duration(microseconds: 1));
    expect(keyDataList, hasLength(1));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.cancel,
      key: kPhysicalCapsLock,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.cancel, key: kLogicalCapsLock),
      ],
    );
    keyDataList.clear();

    // Another key down works
    converter.handleEvent(keyDownEvent('CapsLock', 'CapsLock'));
    expect(keyDataList, hasLength(1));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalCapsLock,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalCapsLock),
      ],
    );
    keyDataList.clear();


    // Schedules are canceled after disposal
    converter.dispose();
    async.elapse(Duration(seconds: 10));
    expect(keyDataList, isEmpty);
  });

  testFakeAsync('CapsLock behaves normally on non-macOS', (FakeAsync async) {
    final List<ui.KeyData> keyDataList = <ui.KeyData>[];
    final KeyboardConverter converter = KeyboardConverter((ui.KeyData key) {
      keyDataList.add(key);
      return true;
    }, onMacOs: false);

    converter.handleEvent(keyDownEvent('CapsLock', 'CapsLock'));
    expect(keyDataList, hasLength(1));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.down,
      key: kPhysicalCapsLock,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.down, key: kLogicalCapsLock),
      ],
    );
    keyDataList.clear();

    async.elapse(Duration(microseconds: 1));
    expect(keyDataList, isEmpty);

    converter.handleEvent(keyUpEvent('CapsLock', 'CapsLock'));
    expect(keyDataList, hasLength(1));
    expectKeyData(keyDataList.last,
      change: ui.KeyChange.up,
      key: kPhysicalCapsLock,
      logical: <ui.LogicalKeyData>[
        ui.LogicalKeyData(change: ui.KeyChange.up, key: kLogicalCapsLock),
      ],
    );
    keyDataList.clear();

    async.elapse(Duration(microseconds: 1));
    expect(keyDataList, isEmpty);
  });
}


class MockKeyboardEvent implements FlutterHtmlKeyboardEvent {
  MockKeyboardEvent({
    required this.type,
    required this.code,
    required this.key,
    this.timeStamp = 0,
    this.repeat = false,
    this.altKey = false,
    this.ctrlKey = false,
    this.shiftKey = false,
    this.metaKey = false,
    this.onPreventDefault,
  });

  String type;
  String? code;
  String? key;
  bool? repeat;
  num? timeStamp;
  bool altKey;
  bool ctrlKey;
  bool shiftKey;
  bool metaKey;

  bool getModifierState(String key) => modifierState.contains(key);
  final Set<String> modifierState = <String>{};

  void preventDefault() { onPreventDefault?.call(); }
  VoidCallback? onPreventDefault;
}

// Flags used for the `modifiers` argument of `key___Event` functions.
const kAlt = 0x1;
const kCtrl = 0x2;
const kShift = 0x4;
const kMeta = 0x8;

// Utility functions to make code more concise.
//
// To add timeStamp or onPreventDefault, use syntax like `..timeStamp = `.
MockKeyboardEvent keyDownEvent(String code, String key, [int modifiers = 0]) {
  return MockKeyboardEvent(
    type: 'keydown',
    code: code,
    key: key,
    altKey: modifiers & kAlt != 0,
    ctrlKey: modifiers & kCtrl != 0,
    shiftKey: modifiers & kShift != 0,
    metaKey: modifiers & kMeta != 0,
  );
}

MockKeyboardEvent keyUpEvent(String code, String key, [int modifiers = 0]) {
  return MockKeyboardEvent(
    type: 'keyup',
    code: code,
    key: key,
    altKey: modifiers & kAlt != 0,
    ctrlKey: modifiers & kCtrl != 0,
    shiftKey: modifiers & kShift != 0,
    metaKey: modifiers & kMeta != 0,
  );
}

MockKeyboardEvent keyRepeatedDownEvent(String code, String key, [int modifiers = 0]) {
  return MockKeyboardEvent(
    type: 'keydown',
    code: code,
    key: key,
    altKey: modifiers & kAlt != 0,
    ctrlKey: modifiers & kCtrl != 0,
    shiftKey: modifiers & kShift != 0,
    metaKey: modifiers & kMeta != 0,
    repeat: true,
  );
}

void expectKeyData(
  ui.KeyData target, {
  Duration? timeStamp,
  required ui.KeyChange change,
  required int key,
  required List<ui.LogicalKeyData> logical,
}) {
  expect(target.change, change);
  expect(target.key, key);
  if (timeStamp != null)
    expect(target.timeStamp, equals(timeStamp));
  expect(target.logicalEvents.length, logical.length);
  for (int i = 0; i < logical.length; i++) {
    expect(target.logicalEvents[i].change, logical[i].change);
    expect(target.logicalEvents[i].key, logical[i].key);
    expect(target.logicalEvents[i].character, logical[i].character);
  }
}

typedef FakeAsyncTest = void Function(FakeAsync);

@isTest
void testFakeAsync(String description, FakeAsyncTest fn) {
  test(description, () {
    FakeAsync().run(fn);
  });
}
