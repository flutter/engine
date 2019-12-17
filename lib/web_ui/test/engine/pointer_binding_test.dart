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

      glassPane.dispatchEvent(context.down());

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

      glassPane.dispatchEvent(context.down());

      expect(packets, hasLength(1));
      expect(packets.single.data, hasLength(2));

      expect(packets.single.data[0].change, equals(ui.PointerChange.add));
      expect(packets.single.data[1].change, equals(ui.PointerChange.down));
    });
  });

  // BUTTONED ADAPTERS

  <_ButtonedEventMixin>[_MouseEventContext(), _PointerEventContext()].forEach((_ButtonedEventMixin context) {
    test('${context.name} creates an add event if the first pointer activity is a hover', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent(context.hoverEvent());

      expect(packets, hasLength(1));
      expect(packets.single.data, hasLength(2));

      expect(packets.single.data[0].change, equals(ui.PointerChange.add));
      expect(packets.single.data[0].synthesized, equals(true));
      expect(packets.single.data[1].change, equals(ui.PointerChange.hover));
    });
  });

  <_ButtonedEventMixin>[_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventMixin context) {
    test('${context.name} synthesizes a pointerup event on two pointerdowns in a row', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent(context.down());

      glassPane.dispatchEvent(context.down());

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

  <_ButtonedEventMixin>[_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventMixin context) {
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

      glassPane.dispatchEvent(context.downWithButton(
        button: 0,
        buttons: 1,
        clientX: 20.0,
        clientY: 50.0,
      ));

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

  <_ButtonedEventMixin>[_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventMixin context) {
    // Touch is in another test since this test involves hovering
    test('${context.name} does calculate delta and pointer identifier correctly', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent(context.hoverEvent(
        clientX: 10.0,
        clientY: 10.0,
      ));

      glassPane.dispatchEvent(context.hoverEvent(
        clientX: 20.0,
        clientY: 20.0,
      ));

      glassPane.dispatchEvent(context.down(
        clientX: 20.0,
        clientY: 20.0,
      ));

      glassPane.dispatchEvent(context.move(
        clientX: 40.0,
        clientY: 30.0,
      ));

      glassPane.dispatchEvent(context.up(
        clientX: 40.0,
        clientY: 30.0,
      ));

      glassPane.dispatchEvent(context.hoverEvent(
        clientX: 20.0,
        clientY: 10.0,
      ));

      glassPane.dispatchEvent(context.down(
        clientX: 20.0,
        clientY: 10.0,
      ));

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

  <_ButtonedEventMixin>[_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventMixin context) {
    test('correctly converts buttons of down, move and up events', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      // Add and hover

      glassPane.dispatchEvent(context.hoverEvent(
        clientX: 10,
        clientY: 11,
      ));

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

      glassPane.dispatchEvent(context.downWithButton(
        button: 0,
        buttons: 1,
        clientX: 10.0,
        clientY: 11.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.down));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(10));
      expect(packets[0].data[0].physicalY, equals(11));
      expect(packets[0].data[0].buttons, equals(1));
      packets.clear();

      glassPane.dispatchEvent(context.moveWithButton(
        button: -1,
        buttons: 1,
        clientX: 20.0,
        clientY: 21.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20));
      expect(packets[0].data[0].physicalY, equals(21));
      expect(packets[0].data[0].buttons, equals(1));
      packets.clear();

      glassPane.dispatchEvent(context.upWithButton(
        button: 0,
        clientX: 20.0,
        clientY: 21.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20));
      expect(packets[0].data[0].physicalY, equals(21));
      expect(packets[0].data[0].buttons, equals(0));
      packets.clear();

      // Drag with secondary button
      glassPane.dispatchEvent(context.downWithButton(
        button: 2,
        buttons: 2,
        clientX: 20.0,
        clientY: 21.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.down));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20));
      expect(packets[0].data[0].physicalY, equals(21));
      expect(packets[0].data[0].buttons, equals(2));
      packets.clear();

      glassPane.dispatchEvent(context.moveWithButton(
        button: -1,
        buttons: 2,
        clientX: 30.0,
        clientY: 31.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(30));
      expect(packets[0].data[0].physicalY, equals(31));
      expect(packets[0].data[0].buttons, equals(2));
      packets.clear();

      glassPane.dispatchEvent(context.upWithButton(
        button: 2,
        clientX: 30.0,
        clientY: 31.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(30));
      expect(packets[0].data[0].physicalY, equals(31));
      expect(packets[0].data[0].buttons, equals(0));
      packets.clear();

      // Drag with middle button
      glassPane.dispatchEvent(context.downWithButton(
        button: 1,
        buttons: 4,
        clientX: 30.0,
        clientY: 31.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.down));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(30));
      expect(packets[0].data[0].physicalY, equals(31));
      expect(packets[0].data[0].buttons, equals(4));
      packets.clear();

      glassPane.dispatchEvent(context.moveWithButton(
        button: -1,
        buttons: 4,
        clientX: 40.0,
        clientY: 41.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(40));
      expect(packets[0].data[0].physicalY, equals(41));
      expect(packets[0].data[0].buttons, equals(4));
      packets.clear();

      glassPane.dispatchEvent(context.upWithButton(
        button: 1,
        clientX: 40.0,
        clientY: 41.0,
      ));
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

  <_ButtonedEventMixin>[_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventMixin context) {
    test('correctly handles button changes during a down sequence', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      // Press LMB.
      glassPane.dispatchEvent(context.downWithButton(
        button: 0,
        buttons: 1,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].synthesized, equals(true));

      expect(packets[0].data[1].change, equals(ui.PointerChange.down));
      expect(packets[0].data[1].synthesized, equals(false));
      expect(packets[0].data[1].buttons, equals(1));
      packets.clear();

      // Press MMB.
      glassPane.dispatchEvent(context.moveWithButton(
        button: 1,
        buttons: 5,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].buttons, equals(5));
      packets.clear();

      // Release LMB.
      glassPane.dispatchEvent(context.moveWithButton(
        button: 0,
        buttons: 4,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].buttons, equals(4));
      packets.clear();

      // Release MMB.
      glassPane.dispatchEvent(context.upWithButton(
        button: 1,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].buttons, equals(0));
      packets.clear();
    });
  });

  <_ButtonedEventMixin>[_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventMixin context) {
    test('synthesizes a pointerup event when pointermove comes before the up', () {
      PointerBinding.instance.debugOverrideDetector(context);
      // This can happen when the user pops up the context menu by right
      // clicking, then dismisses it with a left click.

      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent(context.downWithButton(
        button: 2,
        buttons: 2,
        clientX: 10,
        clientY: 11,
      ));

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

      glassPane.dispatchEvent(context.moveWithButton(
        button: -1,
        buttons: 2,
        clientX: 20.0,
        clientY: 21.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20));
      expect(packets[0].data[0].physicalY, equals(21));
      expect(packets[0].data[0].buttons, equals(2));
      packets.clear();


      glassPane.dispatchEvent(context.moveWithButton(
        button: -1,
        buttons: 2,
        clientX: 20.0,
        clientY: 21.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20));
      expect(packets[0].data[0].physicalY, equals(21));
      expect(packets[0].data[0].buttons, equals(2));
      packets.clear();


      glassPane.dispatchEvent(context.upWithButton(
        button: 2,
        clientX: 20.0,
        clientY: 21.0,
      ));
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

  <_ButtonedEventMixin>[_PointerEventContext(), _MouseEventContext()].forEach((_ButtonedEventMixin context) {
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
      glassPane.dispatchEvent(context.downWithButton(
        button: 2,
        buttons: 2,
      ));
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
      glassPane.dispatchEvent(context.moveWithButton(
        button: -1,
        buttons: 3,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].buttons, equals(3));
      packets.clear();

      // Release LMB.
      glassPane.dispatchEvent(context.moveWithButton(
        button: 0,
        buttons: 2,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].buttons, equals(2));
      packets.clear();

      // Release RMB.
      glassPane.dispatchEvent(context.upWithButton(
        button: 2,
      ));
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
      ).downWithButtonPointer(
        pointer: 1,
        button: 0,
        buttons: 1,
      ));

      glassPane.dispatchEvent((context
        ..pointerType = 'touch'
      ).downWithButtonPointer(
        pointer: 2,
        button: 0,
        buttons: 1,
      ));

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

      glassPane.dispatchEvent(context.downWithPointer(
        pointer: 1,
        clientX: 20.0,
        clientY: 20.0,
      ));
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

      glassPane.dispatchEvent(context.moveWithPointer(
        pointer: 1,
        clientX: 40.0,
        clientY: 30.0,
      ));
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

      glassPane.dispatchEvent(context.upWithPointer(
        pointer: 1,
        clientX: 40.0,
        clientY: 30.0,
      ));
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

      glassPane.dispatchEvent(context.downWithPointer(
        pointer: 2,
        clientX: 20.0,
        clientY: 10.0,
      ));
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

  html.Event down({double clientX, double clientY});
  html.Event move({double clientX, double clientY});
  html.Event up({double clientX, double clientY});
}

mixin _ButtonedEventMixin on _BasicEventContext {
  html.Event downWithButton({double clientX, double clientY, int button, int buttons});
  html.Event moveWithButton({double clientX, double clientY, int button, int buttons});
  html.Event upWithButton({double clientX, double clientY, int button});

  html.Event hoverEvent({double clientX, double clientY}) {
    return moveWithButton(
      buttons: 0,
      button: -1,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event down({double clientX, double clientY}) {
    return downWithButton(
      buttons: 1,
      button: 0,
      clientX: clientX,
      clientY: clientY,
    );
  }


  @override
  html.Event move({double clientX, double clientY}) {
    return moveWithButton(
      buttons: 1,
      button: -1,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event up({double clientX, double clientY}) {
    return upWithButton(
      button: 1,
      clientX: clientX,
      clientY: clientY,
    );
  }
}

mixin _MultiPointerEventMixin on _BasicEventContext {
  html.Event downWithPointer({double clientX, double clientY, int pointer});
  html.Event moveWithPointer({double clientX, double clientY, int pointer});
  html.Event upWithPointer({double clientX, double clientY, int pointer});

  @override
  html.Event down({double clientX, double clientY}) {
    return downWithPointer(
      pointer: 1,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event move({double clientX, double clientY}) {
    return moveWithPointer(
      pointer: 1,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event up({double clientX, double clientY}) {
    return upWithPointer(
      pointer: 1,
      clientX: clientX,
      clientY: clientY,
    );
  }
}

class _TouchEventContext extends _BasicEventContext with _MultiPointerEventMixin implements PointerSupportDetector {
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

  html.EventTarget _target;

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

  html.Event downWithPointer({double clientX, double clientY, int pointer}) {
    return _createTouchEvent(
      'touchstart',
      identifier: pointer,
      clientX: clientX,
      clientY: clientY,
    );
  }

  html.Event moveWithPointer({double clientX, double clientY, int pointer}) {
    return _createTouchEvent(
      'touchmove',
      identifier: pointer,
      clientX: clientX,
      clientY: clientY,
    );
  }

  html.Event upWithPointer({double clientX, double clientY, int pointer}) {
    return _createTouchEvent(
      'touchend',
      identifier: pointer,
      clientX: clientX,
      clientY: clientY,
    );
  }
}

class _MouseEventContext extends _BasicEventContext with _ButtonedEventMixin implements PointerSupportDetector {
  @override
  String get name => 'MouseAdapter';

  @override
  bool get hasPointerEvents => false;

  @override
  bool get hasTouchEvents => false;

  @override
  bool get hasMouseEvents => true;

  @override
  html.Event downWithButton({double clientX, double clientY, int button, int buttons}) {
    return _createMouseEvent(
      'mousedown',
      buttons: buttons,
      button: button,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event moveWithButton({double clientX, double clientY, int button, int buttons}) {
    return _createMouseEvent(
      'mousemove',
      buttons: buttons,
      button: button,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event upWithButton({double clientX, double clientY, int button}) {
    return _createMouseEvent(
      'mouseup',
      buttons: 0,
      button: button,
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

class _PointerEventContext extends _BasicEventContext with _ButtonedEventMixin, _MultiPointerEventMixin implements PointerSupportDetector {
  @override
  String get name => 'PointerAdapter';

  @override
  bool get hasPointerEvents => true;

  @override
  bool get hasTouchEvents => false;

  @override
  bool get hasMouseEvents => false;

  String pointerType = 'mouse';

  @override
  html.Event downWithPointer({double clientX, double clientY, int pointer}) {
    return downWithButtonPointer(
      pointer: 1,
      buttons: 1,
      button: 0,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event downWithButton({double clientX, double clientY, int button, int buttons}) {
    return downWithButtonPointer(
      pointer: 1,
      buttons: buttons,
      button: button,
      clientX: clientX,
      clientY: clientY,
    );
  }

  html.Event downWithButtonPointer({double clientX, double clientY, int button, int buttons, int pointer}) {
    return html.PointerEvent('pointerdown', {
      'pointerId': pointer,
      'button': button,
      'buttons': buttons,
      'clientX': clientX,
      'clientY': clientY,
      'pointerType': pointerType,
    });
  }

  @override
  html.Event moveWithPointer({double clientX, double clientY, int pointer}) {
    return moveWithButtonPointer(
      pointer: 1,
      buttons: 1,
      button: -1,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event moveWithButton({double clientX, double clientY, int button, int buttons}) {
    return moveWithButtonPointer(
      pointer: 1,
      buttons: buttons,
      button: button,
      clientX: clientX,
      clientY: clientY,
    );
  }

  html.Event moveWithButtonPointer({double clientX, double clientY, int button, int buttons, int pointer}) {
    return html.PointerEvent('pointermove', {
      'pointerId': pointer,
      'button': button,
      'buttons': buttons,
      'clientX': clientX,
      'clientY': clientY,
      'pointerType': pointerType,
    });
  }

  @override
  html.Event upWithPointer({double clientX, double clientY, int pointer}) {
    return upWithButtonPointer(
      pointer: 1,
      button: 0,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event upWithButton({double clientX, double clientY, int button}) {
    return upWithButtonPointer(
      pointer: 1,
      button: button,
      clientX: clientX,
      clientY: clientY,
    );
  }

  html.Event upWithButtonPointer({double clientX, double clientY, int button, int pointer}) {
    return html.PointerEvent('pointerup', {
      'pointerId': pointer,
      'button': button,
      'buttons': 0,
      'clientX': clientX,
      'clientY': clientY,
      'pointerType': pointerType,
    });
  }
}
