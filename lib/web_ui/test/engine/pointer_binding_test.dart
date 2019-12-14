// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;
import 'dart:js_util' as js_util;

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

import 'package:test/test.dart';

void main() {
  html.Element glassPane = domRenderer.glassPaneElement;

  setUp(() {
    // Touching domRenderer creates PointerBinding.instance.
    domRenderer;

    ui.window.onPointerDataPacket = null;
  });

  // ALL ADAPTERS

  [_PointerEventContext(), _MouseEventContext(), _TouchEventContext()].forEach((_BasicEventContext context) {
    test('${context.name} can receive pointer events on the glass pane', () {
      PointerBinding.instance.debugOverrideDetector(context);
      ui.PointerDataPacket receivedPacket;
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        receivedPacket = packet;
      };

      glassPane.dispatchEvent((context
        ..down = true
      ).downEvent());

      expect(receivedPacket, isNotNull);
      expect(receivedPacket.data[0].buttons, equals(1));
    });
  });

  [_PointerEventContext(), _MouseEventContext(), _TouchEventContext()].forEach((_BasicEventContext context) {
    test('${context.name} does create an add event if got a pointerdown', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent((context
        ..down = true
      ).downEvent());

      expect(packets, hasLength(1));
      expect(packets.single.data, hasLength(2));

      expect(packets.single.data[0].change, equals(ui.PointerChange.add));
      expect(packets.single.data[1].change, equals(ui.PointerChange.down));
    });
  });

  // BUTTONED ADAPTERS

  [_MouseEventContext(), _PointerEventContext()].forEach((_ButtonedEventContext context) {
    test('${context.name} creates an add event if the first pointer activity is a hover', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent(context.moveEvent());

      expect(packets, hasLength(1));
      expect(packets.single.data, hasLength(2));

      expect(packets.single.data[0].change, equals(ui.PointerChange.add));
      expect(packets.single.data[0].synthesized, equals(true));
      expect(packets.single.data[1].change, equals(ui.PointerChange.hover));
    });
  });

  [_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventContext context) {
    test('${context.name} synthesizes a pointerup event on two pointerdowns in a row', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent((context
        ..down = true
      ).downEvent());

      glassPane.dispatchEvent((context
        ..down = true
      ).downEvent());

      expect(packets, hasLength(2));
      // An add will be synthesized.
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].synthesized, equals(true));
      expect(packets[0].data[1].change, equals(ui.PointerChange.down));
      expect(packets[1].data[0].change, equals(ui.PointerChange.up));
      expect(packets[1].data[1].change, equals(ui.PointerChange.down));
    });
  });

  [_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventContext context) {
    test('${context.name} does synthesize add or hover or more for scroll', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent(html.WheelEvent('wheel',
        button: 1,
        clientX: 10,
        clientY: 10,
        deltaX: 10,
        deltaY: 10,
      ));

      glassPane.dispatchEvent(html.WheelEvent('wheel',
        button: 1,
        clientX: 20,
        clientY: 50,
        deltaX: 10,
        deltaY: 10,
      ));

      glassPane.dispatchEvent((context
        ..button = 0
        ..buttons = 1
        ..clientX = 20.0
        ..clientY = 50.0
      ).downEvent());

      glassPane.dispatchEvent(html.WheelEvent('wheel',
        button: 1,
        clientX: 30,
        clientY: 60,
        deltaX: 10,
        deltaY: 10,
      ));

      expect(packets, hasLength(4));

      // An add will be synthesized.
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].pointerIdentifier, equals(0));
      expect(packets[0].data[0].synthesized, equals(true));
      expect(packets[0].data[0].physicalX, equals(10.0));
      expect(packets[0].data[0].physicalY, equals(10.0));
      expect(packets[0].data[0].physicalDeltaX, equals(0.0));
      expect(packets[0].data[0].physicalDeltaY, equals(0.0));

      expect(packets[0].data[1].change, equals(ui.PointerChange.hover));
      expect(packets[0].data[1].signalKind, equals(ui.PointerSignalKind.scroll));
      expect(packets[0].data[1].pointerIdentifier, equals(0));
      expect(packets[0].data[1].synthesized, equals(false));
      expect(packets[0].data[1].physicalX, equals(10.0));
      expect(packets[0].data[1].physicalY, equals(10.0));
      expect(packets[0].data[1].physicalDeltaX, equals(0.0));
      expect(packets[0].data[1].physicalDeltaY, equals(0.0));

      // A hover will be synthesized.
      expect(packets[1].data, hasLength(2));
      expect(packets[1].data[0].change, equals(ui.PointerChange.hover));
      expect(packets[1].data[0].pointerIdentifier, equals(0));
      expect(packets[1].data[0].synthesized, equals(true));
      expect(packets[1].data[0].physicalX, equals(20.0));
      expect(packets[1].data[0].physicalY, equals(50.0));
      expect(packets[1].data[0].physicalDeltaX, equals(10.0));
      expect(packets[1].data[0].physicalDeltaY, equals(40.0));

      expect(packets[1].data[1].change, equals(ui.PointerChange.hover));
      expect(packets[1].data[1].signalKind, equals(ui.PointerSignalKind.scroll));
      expect(packets[1].data[1].pointerIdentifier, equals(0));
      expect(packets[1].data[1].synthesized, equals(false));
      expect(packets[1].data[1].physicalX, equals(20.0));
      expect(packets[1].data[1].physicalY, equals(50.0));
      expect(packets[1].data[1].physicalDeltaX, equals(0.0));
      expect(packets[1].data[1].physicalDeltaY, equals(0.0));

      // No synthetic pointer data for down event.
      expect(packets[2].data, hasLength(1));
      expect(packets[2].data[0].change, equals(ui.PointerChange.down));
      expect(packets[2].data[0].signalKind, equals(ui.PointerSignalKind.none));
      expect(packets[2].data[0].pointerIdentifier, equals(1));
      expect(packets[2].data[0].synthesized, equals(false));
      expect(packets[2].data[0].physicalX, equals(20.0));
      expect(packets[2].data[0].physicalY, equals(50.0));
      expect(packets[2].data[0].physicalDeltaX, equals(0.0));
      expect(packets[2].data[0].physicalDeltaY, equals(0.0));

      // A move will be synthesized instead of hover because the button is currently down.
      expect(packets[3].data, hasLength(2));
      expect(packets[3].data[0].change, equals(ui.PointerChange.move));
      expect(packets[3].data[0].pointerIdentifier, equals(1));
      expect(packets[3].data[0].synthesized, equals(true));
      expect(packets[3].data[0].physicalX, equals(30.0));
      expect(packets[3].data[0].physicalY, equals(60.0));
      expect(packets[3].data[0].physicalDeltaX, equals(10.0));
      expect(packets[3].data[0].physicalDeltaY, equals(10.0));

      expect(packets[3].data[1].change, equals(ui.PointerChange.hover));
      expect(packets[3].data[1].signalKind, equals(ui.PointerSignalKind.scroll));
      expect(packets[3].data[1].pointerIdentifier, equals(1));
      expect(packets[3].data[1].synthesized, equals(false));
      expect(packets[3].data[1].physicalX, equals(30.0));
      expect(packets[3].data[1].physicalY, equals(60.0));
      expect(packets[3].data[1].physicalDeltaX, equals(0.0));
      expect(packets[3].data[1].physicalDeltaY, equals(0.0));
    });
  });

  [_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventContext context) {
    // Touch is in another test since this test involves hovering
    test('${context.name} does calculate delta and pointer identifier correctly', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent((context
        ..clientX = 10.0
        ..clientY = 10.0
      ).moveEvent());

      glassPane.dispatchEvent((context
        ..clientX = 20.0
        ..clientY = 20.0
      ).moveEvent());

      glassPane.dispatchEvent((context
        ..down = true
      ).downEvent());

      glassPane.dispatchEvent((context
        ..clientX = 40.0
        ..clientY = 30.0
      ).moveEvent());

      glassPane.dispatchEvent((context
        ..down = false
      ).upEvent());

      glassPane.dispatchEvent((context
        ..clientX = 20.0
        ..clientY = 10.0
      ).moveEvent());

      glassPane.dispatchEvent((context
        ..down = true
      ).downEvent());

      expect(packets, hasLength(7));

      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].pointerIdentifier, equals(0));
      expect(packets[0].data[0].synthesized, equals(true));
      expect(packets[0].data[0].physicalX, equals(10.0));
      expect(packets[0].data[0].physicalY, equals(10.0));
      expect(packets[0].data[0].physicalDeltaX, equals(0.0));
      expect(packets[0].data[0].physicalDeltaY, equals(0.0));

      expect(packets[0].data[1].change, equals(ui.PointerChange.hover));
      expect(packets[0].data[1].pointerIdentifier, equals(0));
      expect(packets[0].data[1].synthesized, equals(false));
      expect(packets[0].data[1].physicalX, equals(10.0));
      expect(packets[0].data[1].physicalY, equals(10.0));
      expect(packets[0].data[1].physicalDeltaX, equals(0.0));
      expect(packets[0].data[1].physicalDeltaY, equals(0.0));

      expect(packets[1].data, hasLength(1));
      expect(packets[1].data[0].change, equals(ui.PointerChange.hover));
      expect(packets[1].data[0].pointerIdentifier, equals(0));
      expect(packets[1].data[0].synthesized, equals(false));
      expect(packets[1].data[0].physicalX, equals(20.0));
      expect(packets[1].data[0].physicalY, equals(20.0));
      expect(packets[1].data[0].physicalDeltaX, equals(10.0));
      expect(packets[1].data[0].physicalDeltaY, equals(10.0));

      expect(packets[2].data, hasLength(1));
      expect(packets[2].data[0].change, equals(ui.PointerChange.down));
      expect(packets[2].data[0].pointerIdentifier, equals(1));
      expect(packets[2].data[0].synthesized, equals(false));
      expect(packets[2].data[0].physicalX, equals(20.0));
      expect(packets[2].data[0].physicalY, equals(20.0));
      expect(packets[2].data[0].physicalDeltaX, equals(0.0));
      expect(packets[2].data[0].physicalDeltaY, equals(0.0));

      expect(packets[3].data, hasLength(1));
      expect(packets[3].data[0].change, equals(ui.PointerChange.move));
      expect(packets[3].data[0].pointerIdentifier, equals(1));
      expect(packets[3].data[0].synthesized, equals(false));
      expect(packets[3].data[0].physicalX, equals(40.0));
      expect(packets[3].data[0].physicalY, equals(30.0));
      expect(packets[3].data[0].physicalDeltaX, equals(20.0));
      expect(packets[3].data[0].physicalDeltaY, equals(10.0));

      expect(packets[4].data, hasLength(1));
      expect(packets[4].data[0].change, equals(ui.PointerChange.up));
      expect(packets[4].data[0].pointerIdentifier, equals(1));
      expect(packets[4].data[0].synthesized, equals(false));
      expect(packets[4].data[0].physicalX, equals(40.0));
      expect(packets[4].data[0].physicalY, equals(30.0));
      expect(packets[4].data[0].physicalDeltaX, equals(0.0));
      expect(packets[4].data[0].physicalDeltaY, equals(0.0));

      expect(packets[5].data, hasLength(1));
      expect(packets[5].data[0].change, equals(ui.PointerChange.hover));
      expect(packets[5].data[0].pointerIdentifier, equals(1));
      expect(packets[5].data[0].synthesized, equals(false));
      expect(packets[5].data[0].physicalX, equals(20.0));
      expect(packets[5].data[0].physicalY, equals(10.0));
      expect(packets[5].data[0].physicalDeltaX, equals(-20.0));
      expect(packets[5].data[0].physicalDeltaY, equals(-20.0));

      expect(packets[6].data, hasLength(1));
      expect(packets[6].data[0].change, equals(ui.PointerChange.down));
      expect(packets[6].data[0].pointerIdentifier, equals(2));
      expect(packets[6].data[0].synthesized, equals(false));
      expect(packets[6].data[0].physicalX, equals(20.0));
      expect(packets[6].data[0].physicalY, equals(10.0));
      expect(packets[6].data[0].physicalDeltaX, equals(0.0));
      expect(packets[6].data[0].physicalDeltaY, equals(0.0));
    });
  });

  [_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventContext context) {
    test('correctly converts buttons of down, move and up events', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      // Add and hover

      glassPane.dispatchEvent((context
        ..clientX = 10
        ..clientY = 11
      ).moveEvent());

      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].synthesized, equals(true));
      expect(packets[0].data[0].physicalX, equals(10));
      expect(packets[0].data[0].physicalY, equals(11));

      expect(packets[0].data[1].change, equals(ui.PointerChange.hover));
      expect(packets[0].data[1].synthesized, equals(false));
      expect(packets[0].data[1].physicalX, equals(10));
      expect(packets[0].data[1].physicalY, equals(11));
      expect(packets[0].data[1].buttons, equals(0));
      packets.clear();

      glassPane.dispatchEvent((context
        ..button = 0
        ..buttons = 1
        ..clientX = 10.0
        ..clientY = 11.0
      ).downEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.down));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(10));
      expect(packets[0].data[0].physicalY, equals(11));
      expect(packets[0].data[0].buttons, equals(1));
      packets.clear();

      glassPane.dispatchEvent((context
        ..clientX = 20.0
        ..clientY = 21.0
      ).moveEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20));
      expect(packets[0].data[0].physicalY, equals(21));
      expect(packets[0].data[0].buttons, equals(1));
      packets.clear();

      glassPane.dispatchEvent((context
        ..button = 0
        ..buttons = 0
      ).upEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20));
      expect(packets[0].data[0].physicalY, equals(21));
      expect(packets[0].data[0].buttons, equals(0));
      packets.clear();

      // Drag with secondary button
      glassPane.dispatchEvent((context
        ..button = 2
        ..buttons = 2
        ..clientX = 20.0
        ..clientY = 21.0
      ).downEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.down));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20));
      expect(packets[0].data[0].physicalY, equals(21));
      expect(packets[0].data[0].buttons, equals(2));
      packets.clear();

      glassPane.dispatchEvent(html.PointerEvent('pointermove', {
        'pointerType': 'mouse',
      }));
      glassPane.dispatchEvent((context
        ..button = -1
        ..buttons = 2
        ..clientX = 30.0
        ..clientY = 31.0
      ).moveEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(30));
      expect(packets[0].data[0].physicalY, equals(31));
      expect(packets[0].data[0].buttons, equals(2));
      packets.clear();

      glassPane.dispatchEvent((context
        ..button = 2
        ..buttons = 0
      ).upEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(30));
      expect(packets[0].data[0].physicalY, equals(31));
      expect(packets[0].data[0].buttons, equals(0));
      packets.clear();

      // Drag with middle button
      glassPane.dispatchEvent((context
        ..button = 1
        ..buttons = 4
        ..clientX = 30.0
        ..clientY = 31.0
      ).downEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.down));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(30));
      expect(packets[0].data[0].physicalY, equals(31));
      expect(packets[0].data[0].buttons, equals(4));
      packets.clear();

      glassPane.dispatchEvent((context
        ..clientX = 40.0
        ..clientY = 41.0
      ).moveEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(40));
      expect(packets[0].data[0].physicalY, equals(41));
      expect(packets[0].data[0].buttons, equals(4));
      packets.clear();

      glassPane.dispatchEvent((context
        ..button = 1
        ..buttons = 0
      ).upEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(40));
      expect(packets[0].data[0].physicalY, equals(41));
      expect(packets[0].data[0].buttons, equals(0));
      packets.clear();
    });
  });

  [_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventContext context) {
    test('correctly handles button changes during a down sequence', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      // Press LMB.
      glassPane.dispatchEvent((context
        ..button = 0
        ..buttons = 1
      ).downEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].synthesized, equals(true));

      expect(packets[0].data[1].change, equals(ui.PointerChange.down));
      expect(packets[0].data[1].synthesized, equals(false));
      expect(packets[0].data[1].buttons, equals(1));
      packets.clear();

      // Press MMB.
      glassPane.dispatchEvent((context
        ..button = 1
        ..buttons = 5
      ).moveEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].buttons, equals(5));
      packets.clear();

      // Release LMB.
      glassPane.dispatchEvent((context
        ..button = 0
        ..buttons = 4
      ).moveEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].buttons, equals(4));
      packets.clear();

      // Release MMB.
      glassPane.dispatchEvent((context
        ..button = 1
        ..buttons = 0
      ).upEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].buttons, equals(0));
      packets.clear();
    });
  });

  [_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventContext context) {
    test('synthesizes a pointerup event when pointermove comes before the up', () {
      PointerBinding.instance.debugOverrideDetector(context);
      // This can happen when the user pops up the context menu by right
      // clicking, then dismisses it with a left click.

      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent((context
        ..button = 2
        ..buttons = 2
        ..clientX = 10
        ..clientY = 11
      ).downEvent());

      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].synthesized, equals(true));
      expect(packets[0].data[0].physicalX, equals(10));
      expect(packets[0].data[0].physicalY, equals(11));

      expect(packets[0].data[1].change, equals(ui.PointerChange.down));
      expect(packets[0].data[1].synthesized, equals(false));
      expect(packets[0].data[1].physicalX, equals(10));
      expect(packets[0].data[1].physicalY, equals(11));
      expect(packets[0].data[1].buttons, equals(2));
      packets.clear();

      glassPane.dispatchEvent((context
        ..button = -1
        ..buttons = 2
        ..clientX = 20.0
        ..clientY = 21.0
      ).moveEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20));
      expect(packets[0].data[0].physicalY, equals(21));
      expect(packets[0].data[0].buttons, equals(2));
      packets.clear();


      glassPane.dispatchEvent((context
        ..button = -1
        ..buttons = 2
        ..clientX = 20.0
        ..clientY = 21.0
      ).moveEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20));
      expect(packets[0].data[0].physicalY, equals(21));
      expect(packets[0].data[0].buttons, equals(2));
      packets.clear();


      glassPane.dispatchEvent((context
        ..button = 2
        ..buttons = 0
        ..clientX = 20.0
        ..clientY = 21.0
      ).upEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20));
      expect(packets[0].data[0].physicalY, equals(21));
      expect(packets[0].data[0].buttons, equals(0));
      packets.clear();
    });
  });

  [_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventContext context) {
    test('correctly handles uncontinuous button changes during a down sequence', () {
      PointerBinding.instance.debugOverrideDetector(context);
      // This can happen with the following gesture sequence:
      //
      //  - Pops up the context menu by right clicking, but holds RMB;
      //  - Clicks LMB;
      //  - Releases RMB.

      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      // Press RMB and hold, popping up the context menu.
      glassPane.dispatchEvent((context
        ..button = 2
        ..buttons = 2
      ).downEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].synthesized, equals(true));

      expect(packets[0].data[1].change, equals(ui.PointerChange.down));
      expect(packets[0].data[1].synthesized, equals(false));
      expect(packets[0].data[1].buttons, equals(2));
      packets.clear();

      // Press LMB. The event will have "button: -1" here, despite the change
      // in "buttons", probably because the "press" gesture was absorbed by
      // dismissing the context menu.
      glassPane.dispatchEvent((context
        ..button = -1
        ..buttons = 3
      ).moveEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].buttons, equals(3));
      packets.clear();

      // Release LMB.
      glassPane.dispatchEvent((context
        ..button = 0
        ..buttons = 2
      ).moveEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].buttons, equals(2));
      packets.clear();

      // Release RMB.
      glassPane.dispatchEvent((context
        ..button = 2
        ..buttons = 0
      ).upEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].buttons, equals(0));
      packets.clear();
    });
  });

  // POINTER ADAPTER

  [_PointerEventContext()].forEach((_PointerEventContext context) {
    test('${context.name} does not synthesize pointer up if from different device', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent((context
        ..pointerType = 'touch'
        ..pointerId = 1
        ..button = 0
        ..buttons = 1
      ).downEvent());

      glassPane.dispatchEvent((context
        ..pointerType = 'touch'
        ..pointerId = 2
        ..button = 0
        ..buttons = 1
      ).downEvent());

      expect(packets, hasLength(2));
      // An add will be synthesized.
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].synthesized, equals(true));
      expect(packets[0].data[0].device, equals(1));
      expect(packets[0].data[1].change, equals(ui.PointerChange.down));
      expect(packets[0].data[1].device, equals(1));
      // An add will be synthesized.
      expect(packets[1].data, hasLength(2));
      expect(packets[1].data[0].change, equals(ui.PointerChange.add));
      expect(packets[1].data[0].synthesized, equals(true));
      expect(packets[1].data[0].device, equals(2));
      expect(packets[1].data[1].change, equals(ui.PointerChange.down));
      expect(packets[1].data[1].device, equals(2));
    });
  });

  // TOUCH ADAPTER

  [_TouchEventContext()].forEach((_TouchEventContext context) {
    // Mouse and Pointer are in another test since these tests can involve hovering
    test('${context.name} does calculate delta and pointer identifier correctly', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent((context
        ..identifier = 1
        ..down = true
        ..clientX = 20.0
        ..clientY = 20.0
      ).downEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].pointerIdentifier, equals(1));
      expect(packets[0].data[0].synthesized, equals(true));
      expect(packets[0].data[0].physicalX, equals(20.0));
      expect(packets[0].data[0].physicalY, equals(20.0));
      expect(packets[0].data[0].physicalDeltaX, equals(0.0));
      expect(packets[0].data[0].physicalDeltaY, equals(0.0));

      expect(packets[0].data[1].change, equals(ui.PointerChange.down));
      expect(packets[0].data[1].pointerIdentifier, equals(1));
      expect(packets[0].data[1].synthesized, equals(false));
      expect(packets[0].data[1].physicalX, equals(20.0));
      expect(packets[0].data[1].physicalY, equals(20.0));
      expect(packets[0].data[1].physicalDeltaX, equals(0.0));
      expect(packets[0].data[1].physicalDeltaY, equals(0.0));
      packets.clear();

      glassPane.dispatchEvent((context
        ..clientX = 40.0
        ..clientY = 30.0
      ).moveEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].pointerIdentifier, equals(1));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(40.0));
      expect(packets[0].data[0].physicalY, equals(30.0));
      expect(packets[0].data[0].physicalDeltaX, equals(20.0));
      expect(packets[0].data[0].physicalDeltaY, equals(10.0));
      packets.clear();

      glassPane.dispatchEvent((context
        ..down = false
      ).upEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].pointerIdentifier, equals(1));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(40.0));
      expect(packets[0].data[0].physicalY, equals(30.0));
      expect(packets[0].data[0].physicalDeltaX, equals(0.0));
      expect(packets[0].data[0].physicalDeltaY, equals(0.0));
      packets.clear();

      glassPane.dispatchEvent((context
        ..identifier = 2
        ..clientX = 20.0
        ..clientY = 10.0
        ..down = true
      ).downEvent());
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].pointerIdentifier, equals(2));
      expect(packets[0].data[0].synthesized, equals(true));
      expect(packets[0].data[0].physicalX, equals(20.0));
      expect(packets[0].data[0].physicalY, equals(10.0));
      expect(packets[0].data[0].physicalDeltaX, equals(0.0));
      expect(packets[0].data[0].physicalDeltaY, equals(0.0));

      expect(packets[0].data[1].change, equals(ui.PointerChange.down));
      expect(packets[0].data[1].pointerIdentifier, equals(2));
      expect(packets[0].data[1].synthesized, equals(false));
      expect(packets[0].data[1].physicalX, equals(20.0));
      expect(packets[0].data[1].physicalY, equals(10.0));
      expect(packets[0].data[1].physicalDeltaX, equals(0.0));
      expect(packets[0].data[1].physicalDeltaY, equals(0.0));
      packets.clear();
    });
  });
}

abstract class _BasicEventContext implements PointerSupportDetector {
  String get name;

  double clientX = 0;
  double clientY = 0;
  bool get down;
  set down(bool value);

  html.Event downEvent();
  html.Event moveEvent();
  html.Event upEvent();
}

class _TouchEventContext extends _BasicEventContext implements PointerSupportDetector {
  _TouchEventContext() {
    _target = html.document.createElement('div');
  }

  @override
  String get name => 'TouchAdapter';

  @override
  bool get hasPointerEvents => false;

  @override
  bool get hasTouchEvents => true;

  @override
  bool get hasMouseEvents => false;

  @override
  bool get down => _down;
  set down(bool value) {
    _down = value;
  }
  bool _down;

  html.EventTarget _target;

  int identifier = 1;

  html.Touch _createTouch({
    int identifier,
    double clientX,
    double clientY,
  }) {
    return html.Touch({
      'identifier': identifier,
      'clientX': clientX,
      'clientY': clientY,
      'target': _target,
    });
  }

  html.TouchEvent _createTouchEvent(String eventType, {
    int identifier,
    double clientX,
    double clientY,
  }) {
    return html.TouchEvent(eventType, {
      'changedTouches': [
        _createTouch(
          identifier: identifier,
          clientX: clientX,
          clientY: clientY,
        )
      ],
    });
  }

  html.Event downEvent() {
    return _createTouchEvent(
      'touchstart',
      identifier: identifier,
      clientX: clientX,
      clientY: clientY,
    );
  }

  html.Event moveEvent() {
    return _createTouchEvent(
      'touchmove',
      identifier: identifier,
      clientX: clientX,
      clientY: clientY,
    );
  }

  html.Event upEvent() {
    return _createTouchEvent(
      'touchend',
      identifier: identifier,
      clientX: clientX,
      clientY: clientY,
    );
  }
}

abstract class _ButtonedEventContext extends _BasicEventContext {
  int button = 0;
  int buttons = 0;

  @override
  bool get down => buttons != 0;

  @override
  set down(bool value) {
    if (value != down) {
      button = 0;
      buttons = value ? 1 : 0;
    }
  }

  int getButtonAndReset() {
    int result = button;
    button = -1;
    return result;
  }
}

class _MouseEventContext extends _ButtonedEventContext implements PointerSupportDetector {
  @override
  String get name => 'MouseAdapter';

  @override
  bool get hasPointerEvents => false;

  @override
  bool get hasTouchEvents => false;

  @override
  bool get hasMouseEvents => true;

  html.Event downEvent() {
    return _createMouseEvent(
      'mousedown',
      buttons: buttons,
      button: getButtonAndReset(),
      clientX: clientX,
      clientY: clientY,
    );
  }

  html.Event moveEvent() {
    return _createMouseEvent(
      'mousemove',
      buttons: buttons,
      button: getButtonAndReset(),
      clientX: clientX,
      clientY: clientY,
    );
  }

  html.Event upEvent() {
    return _createMouseEvent(
      'mouseup',
      buttons: buttons,
      button: getButtonAndReset(),
      clientX: clientX,
      clientY: clientY,
    );
  }

  html.MouseEvent _createMouseEvent(
    String type, {
    int buttons,
    int button,
    double clientX,
    double clientY,
  }) {
    final Function jsMouseEvent = js_util.getProperty(html.window, 'MouseEvent');
    final List<dynamic> eventArgs = <dynamic>[
      type,
      <String, dynamic>{
        'buttons': buttons,
        'button': button,
        'clientX': clientX,
        'clientY': clientY,
      }
    ];
    return js_util.callConstructor(jsMouseEvent, js_util.jsify(eventArgs));
  }
}


class _PointerEventContext extends _ButtonedEventContext implements PointerSupportDetector {
  @override
  String get name => 'PointerAdapter';

  @override
  bool get hasPointerEvents => true;

  @override
  bool get hasTouchEvents => false;

  @override
  bool get hasMouseEvents => false;

  int pointerId = 0;
  String pointerType = 'mouse';

  html.Event downEvent() {
    return html.PointerEvent('pointerdown', {
      'pointerId': pointerId,
      'button': getButtonAndReset(),
      'buttons': buttons,
      'clientX': clientX,
      'clientY': clientY,
      'pointerType': pointerType,
    });
  }

  html.Event moveEvent() {
    return html.PointerEvent('pointermove', {
      'pointerId': pointerId,
      'button': getButtonAndReset(),
      'buttons': buttons,
      'clientX': clientX,
      'clientY': clientY,
      'pointerType': pointerType,
    });
  }

  html.Event upEvent() {
    return html.PointerEvent('pointerup', {
      'pointerId': pointerId,
      'button': getButtonAndReset(),
      'buttons': buttons,
      'clientX': clientX,
      'clientY': clientY,
      'pointerType': pointerType,
    });
  }
}
