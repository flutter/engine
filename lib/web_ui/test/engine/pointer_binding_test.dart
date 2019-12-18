// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;
import 'dart:js_util' as js_util;

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

import 'package:test/test.dart';

const int _kPrimaryMouseButton = 0x1;
const int _kSecondaryMouseButton = 0x2;
const int _kMiddleMouseButton =0x4;
const int _kBackwardMouseButton = 0x8;
const int _kForwardMouseButton = 0x10;

const int _kNoButtonChange = -1;

void main() {
  html.Element glassPane = domRenderer.glassPaneElement;

  setUp(() {
    // Touching domRenderer creates PointerBinding.instance.
    domRenderer;

    ui.window.onPointerDataPacket = null;
  });

  test('_PointerEventContext generates expected events', () {
    html.PointerEvent expectCorrectType(html.Event e) {
      expect(e.runtimeType, equals(html.PointerEvent));
      return e;
    }

    final _PointerEventContext context = _PointerEventContext();
    html.PointerEvent event;

    event = expectCorrectType(context.primaryDown(clientX: 100, clientY: 101));
    expect(event.type, equals('pointerdown'));
    expect(event.pointerId, equals(1));
    expect(event.button, equals(0));
    expect(event.buttons, equals(1));
    expect(event.client.x, equals(100));
    expect(event.client.y, equals(101));

    event = expectCorrectType(context.mouseDown(clientX: 110, clientY: 111, button: 2, buttons: 2));
    expect(event.type, equals('pointerdown'));
    expect(event.pointerId, equals(1));
    expect(event.button, equals(2));
    expect(event.buttons, equals(2));
    expect(event.client.x, equals(110));
    expect(event.client.y, equals(111));

    event = expectCorrectType(context.downWithPointer(clientX: 120, clientY: 121, pointer: 100));
    expect(event.type, equals('pointerdown'));
    expect(event.pointerId, equals(100));
    expect(event.button, equals(0));
    expect(event.buttons, equals(1));
    expect(event.client.x, equals(120));
    expect(event.client.y, equals(121));

    event = expectCorrectType(context.primaryMove(clientX: 200, clientY: 201));
    expect(event.type, equals('pointermove'));
    expect(event.pointerId, equals(1));
    expect(event.button, equals(-1));
    expect(event.buttons, equals(1));
    expect(event.client.x, equals(200));
    expect(event.client.y, equals(201));

    event = expectCorrectType(context.mouseMove(clientX: 210, clientY: 211, button: _kNoButtonChange, buttons: 6));
    expect(event.type, equals('pointermove'));
    expect(event.pointerId, equals(1));
    expect(event.button, equals(-1));
    expect(event.buttons, equals(6));
    expect(event.client.x, equals(210));
    expect(event.client.y, equals(211));

    event = expectCorrectType(context.mouseMove(clientX: 212, clientY: 213, button: 2, buttons: 6));
    expect(event.type, equals('pointermove'));
    expect(event.pointerId, equals(1));
    expect(event.button, equals(2));
    expect(event.buttons, equals(6));
    expect(event.client.x, equals(212));
    expect(event.client.y, equals(213));

    event = expectCorrectType(context.mouseMove(clientX: 214, clientY: 215, button: 2, buttons: 1));
    expect(event.type, equals('pointermove'));
    expect(event.pointerId, equals(1));
    expect(event.button, equals(2));
    expect(event.buttons, equals(1));
    expect(event.client.x, equals(214));
    expect(event.client.y, equals(215));

    event = expectCorrectType(context.moveWithPointer(clientX: 220, clientY: 221, pointer: 101));
    expect(event.type, equals('pointermove'));
    expect(event.pointerId, equals(101));
    expect(event.button, equals(-1));
    expect(event.buttons, equals(1));
    expect(event.client.x, equals(220));
    expect(event.client.y, equals(221));

    event = expectCorrectType(context.primaryUp(clientX: 300, clientY: 301));
    expect(event.type, equals('pointerup'));
    expect(event.pointerId, equals(1));
    expect(event.button, equals(0));
    expect(event.buttons, equals(0));
    expect(event.client.x, equals(300));
    expect(event.client.y, equals(301));

    event = expectCorrectType(context.mouseUp(clientX: 310, clientY: 311, button: 2));
    expect(event.type, equals('pointerup'));
    expect(event.pointerId, equals(1));
    expect(event.button, equals(2));
    expect(event.buttons, equals(0));
    expect(event.client.x, equals(310));
    expect(event.client.y, equals(311));

    event = expectCorrectType(context.upWithPointer(clientX: 320, clientY: 321, pointer: 102));
    expect(event.type, equals('pointerup'));
    expect(event.pointerId, equals(102));
    expect(event.button, equals(0));
    expect(event.buttons, equals(0));
    expect(event.client.x, equals(320));
    expect(event.client.y, equals(321));

    event = expectCorrectType(context.hover(clientX: 400, clientY: 401));
    expect(event.type, equals('pointermove'));
    expect(event.pointerId, equals(1));
    expect(event.button, equals(-1));
    expect(event.buttons, equals(0));
    expect(event.client.x, equals(400));
    expect(event.client.y, equals(401));
  });

  test('_TouchEventContext generates expected events', () {
    html.TouchEvent expectCorrectType(html.Event e) {
      expect(e.runtimeType, equals(html.TouchEvent));
      return e;
    }

    final _TouchEventContext context = _TouchEventContext();
    html.TouchEvent event;

    event = expectCorrectType(context.primaryDown(clientX: 100, clientY: 101));
    expect(event.type, equals('touchstart'));
    expect(event.changedTouches.length, equals(1));
    expect(event.changedTouches[0].identifier, equals(1));
    expect(event.changedTouches[0].client.x, equals(100));
    expect(event.changedTouches[0].client.y, equals(101));

    event = expectCorrectType(context.downWithPointer(clientX: 110, clientY: 111, pointer: 100));
    expect(event.type, equals('touchstart'));
    expect(event.changedTouches.length, equals(1));
    expect(event.changedTouches[0].identifier, equals(100));
    expect(event.changedTouches[0].client.x, equals(110));
    expect(event.changedTouches[0].client.y, equals(111));

    event = expectCorrectType(context.primaryMove(clientX: 200, clientY: 201));
    expect(event.type, equals('touchmove'));
    expect(event.changedTouches.length, equals(1));
    expect(event.changedTouches[0].identifier, equals(1));
    expect(event.changedTouches[0].client.x, equals(200));
    expect(event.changedTouches[0].client.y, equals(201));

    event = expectCorrectType(context.moveWithPointer(clientX: 210, clientY: 211, pointer: 101));
    expect(event.type, equals('touchmove'));
    expect(event.changedTouches.length, equals(1));
    expect(event.changedTouches[0].identifier, equals(101));
    expect(event.changedTouches[0].client.x, equals(210));
    expect(event.changedTouches[0].client.y, equals(211));

    event = expectCorrectType(context.primaryUp(clientX: 300, clientY: 301));
    expect(event.type, equals('touchend'));
    expect(event.changedTouches.length, equals(1));
    expect(event.changedTouches[0].identifier, equals(1));
    expect(event.changedTouches[0].client.x, equals(300));
    expect(event.changedTouches[0].client.y, equals(301));

    event = expectCorrectType(context.upWithPointer(clientX: 310, clientY: 311, pointer: 102));
    expect(event.type, equals('touchend'));
    expect(event.changedTouches.length, equals(1));
    expect(event.changedTouches[0].identifier, equals(102));
    expect(event.changedTouches[0].client.x, equals(310));
    expect(event.changedTouches[0].client.y, equals(311));
  });

  test('_MouseEventContext generates expected events', () {
    html.MouseEvent expectCorrectType(html.Event e) {
      expect(e.runtimeType, equals(html.MouseEvent));
      return e;
    }

    final _MouseEventContext context = _MouseEventContext();
    html.MouseEvent event;

    event = expectCorrectType(context.primaryDown(clientX: 100, clientY: 101));
    expect(event.type, equals('mousedown'));
    expect(event.button, equals(0));
    expect(event.buttons, equals(1));
    expect(event.client.x, equals(100));
    expect(event.client.y, equals(101));

    event = expectCorrectType(context.mouseDown(clientX: 110, clientY: 111, button: 2, buttons: 2));
    expect(event.type, equals('mousedown'));
    expect(event.button, equals(2));
    expect(event.buttons, equals(2));
    expect(event.client.x, equals(110));
    expect(event.client.y, equals(111));

    event = expectCorrectType(context.primaryMove(clientX: 200, clientY: 201));
    expect(event.type, equals('mousemove'));
    expect(event.button, equals(0));
    expect(event.buttons, equals(1));
    expect(event.client.x, equals(200));
    expect(event.client.y, equals(201));

    event = expectCorrectType(context.mouseMove(clientX: 210, clientY: 211, button: _kNoButtonChange, buttons: 6));
    expect(event.type, equals('mousemove'));
    expect(event.button, equals(0));
    expect(event.buttons, equals(6));
    expect(event.client.x, equals(210));
    expect(event.client.y, equals(211));

    event = expectCorrectType(context.mouseMove(clientX: 212, clientY: 213, button: 2, buttons: 6));
    expect(event.type, equals('mousedown'));
    expect(event.button, equals(2));
    expect(event.buttons, equals(6));
    expect(event.client.x, equals(212));
    expect(event.client.y, equals(213));

    event = expectCorrectType(context.mouseMove(clientX: 214, clientY: 215, button: 2, buttons: 1));
    expect(event.type, equals('mouseup'));
    expect(event.button, equals(2));
    expect(event.buttons, equals(1));
    expect(event.client.x, equals(214));
    expect(event.client.y, equals(215));

    event = expectCorrectType(context.primaryUp(clientX: 300, clientY: 301));
    expect(event.type, equals('mouseup'));
    expect(event.button, equals(0));
    expect(event.buttons, equals(0));
    expect(event.client.x, equals(300));
    expect(event.client.y, equals(301));

    event = expectCorrectType(context.mouseUp(clientX: 310, clientY: 311, button: 2));
    expect(event.type, equals('mouseup'));
    expect(event.button, equals(2));
    expect(event.buttons, equals(0));
    expect(event.client.x, equals(310));
    expect(event.client.y, equals(311));

    event = expectCorrectType(context.hover(clientX: 400, clientY: 401));
    expect(event.type, equals('mousemove'));
    expect(event.button, equals(0));
    expect(event.buttons, equals(0));
    expect(event.client.x, equals(400));
    expect(event.client.y, equals(401));
  });

  // ALL ADAPTERS

  [_PointerEventContext(), _MouseEventContext(), _TouchEventContext()].forEach((_BasicEventContext context) {
    test('${context.name} can receive pointer events on the glass pane', () {
      PointerBinding.instance.debugOverrideDetector(context);
      ui.PointerDataPacket receivedPacket;
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        receivedPacket = packet;
      };

      glassPane.dispatchEvent(context.primaryDown());

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

      glassPane.dispatchEvent(context.primaryDown());

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

      glassPane.dispatchEvent(context.hover());

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

      glassPane.dispatchEvent(context.primaryDown());

      glassPane.dispatchEvent(context.primaryDown());

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

      glassPane.dispatchEvent(context.mouseDown(
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

      glassPane.dispatchEvent(context.hover(
        clientX: 10.0,
        clientY: 10.0,
      ));
      expect(packets, hasLength(1));
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
      packets.clear();

      glassPane.dispatchEvent(context.hover(
        clientX: 20.0,
        clientY: 20.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.hover));
      expect(packets[0].data[0].pointerIdentifier, equals(0));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20.0));
      expect(packets[0].data[0].physicalY, equals(20.0));
      expect(packets[0].data[0].physicalDeltaX, equals(10.0));
      expect(packets[0].data[0].physicalDeltaY, equals(10.0));
      packets.clear();

      glassPane.dispatchEvent(context.primaryDown(
        clientX: 20.0,
        clientY: 20.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.down));
      expect(packets[0].data[0].pointerIdentifier, equals(1));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20.0));
      expect(packets[0].data[0].physicalY, equals(20.0));
      expect(packets[0].data[0].physicalDeltaX, equals(0.0));
      expect(packets[0].data[0].physicalDeltaY, equals(0.0));
      packets.clear();

      glassPane.dispatchEvent(context.primaryMove(
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

      glassPane.dispatchEvent(context.primaryUp(
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

      glassPane.dispatchEvent(context.hover(
        clientX: 20.0,
        clientY: 10.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.hover));
      expect(packets[0].data[0].pointerIdentifier, equals(1));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20.0));
      expect(packets[0].data[0].physicalY, equals(10.0));
      expect(packets[0].data[0].physicalDeltaX, equals(-20.0));
      expect(packets[0].data[0].physicalDeltaY, equals(-20.0));
      packets.clear();

      glassPane.dispatchEvent(context.primaryDown(
        clientX: 20.0,
        clientY: 10.0,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.down));
      expect(packets[0].data[0].pointerIdentifier, equals(2));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(20.0));
      expect(packets[0].data[0].physicalY, equals(10.0));
      expect(packets[0].data[0].physicalDeltaX, equals(0.0));
      expect(packets[0].data[0].physicalDeltaY, equals(0.0));
      packets.clear();
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

      glassPane.dispatchEvent(context.hover(
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

      glassPane.dispatchEvent(context.mouseDown(
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

      glassPane.dispatchEvent(context.mouseMove(
        button: _kNoButtonChange,
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

      glassPane.dispatchEvent(context.mouseUp(
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
      glassPane.dispatchEvent(context.mouseDown(
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

      glassPane.dispatchEvent(context.mouseMove(
        button: _kNoButtonChange,
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

      glassPane.dispatchEvent(context.mouseUp(
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
      glassPane.dispatchEvent(context.mouseDown(
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

      glassPane.dispatchEvent(context.mouseMove(
        button: _kNoButtonChange,
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

      glassPane.dispatchEvent(context.mouseUp(
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
      glassPane.dispatchEvent(context.mouseDown(
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
      glassPane.dispatchEvent(context.mouseMove(
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
      glassPane.dispatchEvent(context.mouseMove(
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
      glassPane.dispatchEvent(context.mouseUp(
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

      glassPane.dispatchEvent(context.mouseDown(
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

      glassPane.dispatchEvent(context.mouseMove(
        button: _kNoButtonChange,
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


      glassPane.dispatchEvent(context.mouseMove(
        button: _kNoButtonChange,
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


      glassPane.dispatchEvent(context.mouseUp(
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
      glassPane.dispatchEvent(context.mouseDown(
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
      glassPane.dispatchEvent(context.mouseMove(
        button: _kNoButtonChange,
        buttons: 3,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].buttons, equals(3));
      packets.clear();

      // Release LMB.
      glassPane.dispatchEvent(context.mouseMove(
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
      glassPane.dispatchEvent(context.mouseUp(
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

  // MULTIPOINTER ADAPTERS

  <_MultiPointerEventMixin>[_PointerEventContext(), _TouchEventContext()].forEach((_MultiPointerEventMixin context) {
    test('${context.name} treats each pointer separately', () {
      PointerBinding.instance.debugOverrideDetector(context);
      List<ui.PointerDataPacket> packets = <ui.PointerDataPacket>[];
      ui.window.onPointerDataPacket = (ui.PointerDataPacket packet) {
        packets.add(packet);
      };

      glassPane.dispatchEvent(context.downWithPointer(
        pointer: 2,
        clientX: 100,
        clientY: 101,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].synthesized, equals(true));
      expect(packets[0].data[0].device, equals(2));
      expect(packets[0].data[0].physicalX, equals(100));
      expect(packets[0].data[0].physicalY, equals(101));

      expect(packets[0].data[1].device, equals(2));
      expect(packets[0].data[1].buttons, equals(1));
      expect(packets[0].data[1].physicalX, equals(100));
      expect(packets[0].data[1].physicalY, equals(101));
      expect(packets[0].data[1].physicalDeltaX, equals(0));
      expect(packets[0].data[1].physicalDeltaY, equals(0));
      packets.clear();

      glassPane.dispatchEvent(context.downWithPointer(
        pointer: 3,
        clientX: 200,
        clientY: 201,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].synthesized, equals(true));
      expect(packets[0].data[0].device, equals(3));
      expect(packets[0].data[0].physicalX, equals(200));
      expect(packets[0].data[0].physicalY, equals(201));

      expect(packets[0].data[1].change, equals(ui.PointerChange.down));
      expect(packets[0].data[1].device, equals(3));
      expect(packets[0].data[1].buttons, equals(1));
      expect(packets[0].data[1].physicalX, equals(200));
      expect(packets[0].data[1].physicalY, equals(201));
      expect(packets[0].data[1].physicalDeltaX, equals(0));
      expect(packets[0].data[1].physicalDeltaY, equals(0));
      packets.clear();

      glassPane.dispatchEvent(context.moveWithPointer(
        pointer: 3,
        clientX: 300,
        clientY: 302,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].device, equals(3));
      expect(packets[0].data[0].buttons, equals(1));
      expect(packets[0].data[0].physicalX, equals(300));
      expect(packets[0].data[0].physicalY, equals(302));
      expect(packets[0].data[0].physicalDeltaX, equals(100));
      expect(packets[0].data[0].physicalDeltaY, equals(101));
      packets.clear();

      glassPane.dispatchEvent(context.moveWithPointer(
        pointer: 2,
        clientX: 400,
        clientY: 402,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(1));
      expect(packets[0].data[0].change, equals(ui.PointerChange.move));
      expect(packets[0].data[0].device, equals(2));
      expect(packets[0].data[0].buttons, equals(1));
      expect(packets[0].data[0].physicalX, equals(400));
      expect(packets[0].data[0].physicalY, equals(402));
      expect(packets[0].data[0].physicalDeltaX, equals(300));
      expect(packets[0].data[0].physicalDeltaY, equals(301));
      packets.clear();

      glassPane.dispatchEvent(context.upWithPointer(
        pointer: 3,
        clientX: 300,
        clientY: 302,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].device, equals(3));
      expect(packets[0].data[0].buttons, equals(0));
      expect(packets[0].data[0].physicalX, equals(300));
      expect(packets[0].data[0].physicalY, equals(302));
      expect(packets[0].data[0].physicalDeltaX, equals(0));
      expect(packets[0].data[0].physicalDeltaY, equals(0));

      expect(packets[0].data[1].change, equals(ui.PointerChange.remove));
      expect(packets[0].data[1].device, equals(3));
      expect(packets[0].data[1].buttons, equals(0));
      expect(packets[0].data[1].physicalX, equals(300));
      expect(packets[0].data[1].physicalY, equals(302));
      expect(packets[0].data[1].physicalDeltaX, equals(0));
      expect(packets[0].data[1].physicalDeltaY, equals(0));
      packets.clear();

      glassPane.dispatchEvent(context.upWithPointer(
        pointer: 2,
        clientX: 400,
        clientY: 402,
      ));
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].device, equals(2));
      expect(packets[0].data[0].buttons, equals(0));
      expect(packets[0].data[0].physicalX, equals(400));
      expect(packets[0].data[0].physicalY, equals(402));
      expect(packets[0].data[0].physicalDeltaX, equals(0));
      expect(packets[0].data[0].physicalDeltaY, equals(0));

      expect(packets[0].data[1].change, equals(ui.PointerChange.remove));
      expect(packets[0].data[1].device, equals(2));
      expect(packets[0].data[1].buttons, equals(0));
      expect(packets[0].data[1].physicalX, equals(400));
      expect(packets[0].data[1].physicalY, equals(402));
      expect(packets[0].data[1].physicalDeltaX, equals(0));
      expect(packets[0].data[1].physicalDeltaY, equals(0));
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

      glassPane.dispatchEvent(context.downWithPointer(
        pointer: 1,
      ));
      expect(packets, hasLength(1));
      // An add will be synthesized.
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].synthesized, equals(true));
      expect(packets[0].data[0].device, equals(1));
      expect(packets[0].data[1].change, equals(ui.PointerChange.down));
      expect(packets[0].data[1].device, equals(1));
      packets.clear();

      glassPane.dispatchEvent(context.downWithPointer(
        pointer: 2,
      ));
      // An add will be synthesized.
      expect(packets, hasLength(1));
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.add));
      expect(packets[0].data[0].synthesized, equals(true));
      expect(packets[0].data[0].device, equals(2));
      expect(packets[0].data[1].change, equals(ui.PointerChange.down));
      expect(packets[0].data[1].device, equals(2));
      packets.clear();
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
      expect(packets[0].data, hasLength(2));
      expect(packets[0].data[0].change, equals(ui.PointerChange.up));
      expect(packets[0].data[0].pointerIdentifier, equals(1));
      expect(packets[0].data[0].synthesized, equals(false));
      expect(packets[0].data[0].physicalX, equals(40.0));
      expect(packets[0].data[0].physicalY, equals(30.0));
      expect(packets[0].data[0].physicalDeltaX, equals(0.0));
      expect(packets[0].data[0].physicalDeltaY, equals(0.0));

      expect(packets[0].data[1].change, equals(ui.PointerChange.remove));
      expect(packets[0].data[1].pointerIdentifier, equals(1));
      expect(packets[0].data[1].synthesized, equals(false));
      expect(packets[0].data[1].physicalX, equals(40.0));
      expect(packets[0].data[1].physicalY, equals(30.0));
      expect(packets[0].data[1].physicalDeltaX, equals(0.0));
      expect(packets[0].data[1].physicalDeltaY, equals(0.0));
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

  // Generate an event that is:
  //
  //  * For mouse, a left click
  //  * For touch, a touch down
  html.Event primaryDown({double clientX, double clientY});


  // Generate an event that is:
  //
  //  * For mouse, a drag with LMB down
  //  * For touch, a touch drag
  html.Event primaryMove({double clientX, double clientY});


  // Generate an event that is:
  //
  //  * For mouse, release LMB
  //  * For touch, a touch up
  html.Event primaryUp({double clientX, double clientY});
}

mixin _ButtonedEventMixin on _BasicEventContext {
  // Generate an event that is a mouse down with the specific buttons.
  html.Event mouseDown({double clientX, double clientY, int button, int buttons});

  // Generate an event that is a mouse drag with the specific buttons, or button
  // changes during the drag.
  //
  // If there is no button change, assign `button` with _kNoButtonChange.
  html.Event mouseMove({double clientX, double clientY, int button, int buttons});

  // Generate an event that releases all mouse buttons.
  html.Event mouseUp({double clientX, double clientY, int button});

  html.Event hover({double clientX, double clientY}) {
    return mouseMove(
      buttons: 0,
      button: _kNoButtonChange,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event primaryDown({double clientX, double clientY}) {
    return mouseDown(
      buttons: 1,
      button: 0,
      clientX: clientX,
      clientY: clientY,
    );
  }


  @override
  html.Event primaryMove({double clientX, double clientY}) {
    return mouseMove(
      buttons: 1,
      button: _kNoButtonChange,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event primaryUp({double clientX, double clientY}) {
    return mouseUp(
      button: 0,
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
  html.Event primaryDown({double clientX, double clientY}) {
    return downWithPointer(
      pointer: 1,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event primaryMove({double clientX, double clientY}) {
    return moveWithPointer(
      pointer: 1,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event primaryUp({double clientX, double clientY}) {
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

  static int _convertButtonToButtons(int button) {
    switch(button) {
      case 0:
        return _kPrimaryMouseButton;
      case 1:
        return _kSecondaryMouseButton;
      case 2:
        return _kMiddleMouseButton;
      case 3:
        return _kBackwardMouseButton;
      case 4:
        return _kForwardMouseButton;
      default:
        assert(false, 'Unexpected button $button.');
        return -1;
    }
  }

  @override
  html.Event mouseDown({double clientX, double clientY, int button, int buttons}) {
    return _createMouseEvent(
      'mousedown',
      buttons: buttons,
      button: button,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event mouseMove({double clientX, double clientY, int button, int buttons}) {
    final bool hasButtonChange = button != _kNoButtonChange;
    final bool changeIsButtonDown = hasButtonChange && (buttons & _convertButtonToButtons(button)) != 0;
    final String adjustedType = !hasButtonChange ?   'mousemove' :
                                changeIsButtonDown ? 'mousedown' :
                                                     'mouseup';
    final int adjustedButton = hasButtonChange ? button : 0;
    return _createMouseEvent(
      adjustedType,
      buttons: buttons,
      button: adjustedButton,
      clientX: clientX,
      clientY: clientY,
    );
  }

  @override
  html.Event mouseUp({double clientX, double clientY, int button}) {
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

class _PointerEventContext extends _BasicEventContext with _ButtonedEventMixin implements PointerSupportDetector, _MultiPointerEventMixin {
  @override
  String get name => 'PointerAdapter';

  @override
  bool get hasPointerEvents => true;

  @override
  bool get hasTouchEvents => false;

  @override
  bool get hasMouseEvents => false;

  @override
  html.Event downWithPointer({double clientX, double clientY, int pointer}) {
    return _downWithFullDetails(
      pointer: pointer,
      buttons: 1,
      button: 0,
      clientX: clientX,
      clientY: clientY,
      pointerType: 'touch',
    );
  }

  @override
  html.Event mouseDown({double clientX, double clientY, int button, int buttons}) {
    return _downWithFullDetails(
      pointer: 1,
      buttons: buttons,
      button: button,
      clientX: clientX,
      clientY: clientY,
      pointerType: 'mouse',
    );
  }

  html.Event _downWithFullDetails({double clientX, double clientY, int button, int buttons, int pointer, String pointerType}) {
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
    return _moveWithFullDetails(
      pointer: pointer,
      buttons: 1,
      button: _kNoButtonChange,
      clientX: clientX,
      clientY: clientY,
      pointerType: 'touch',
    );
  }

  @override
  html.Event mouseMove({double clientX, double clientY, int button, int buttons}) {
    return _moveWithFullDetails(
      pointer: 1,
      buttons: buttons,
      button: button,
      clientX: clientX,
      clientY: clientY,
      pointerType: 'mouse',
    );
  }

  html.Event _moveWithFullDetails({double clientX, double clientY, int button, int buttons, int pointer, String pointerType}) {
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
    return _upWithFullDetails(
      pointer: pointer,
      button: 0,
      clientX: clientX,
      clientY: clientY,
      pointerType: 'touch',
    );
  }

  @override
  html.Event mouseUp({double clientX, double clientY, int button}) {
    return _upWithFullDetails(
      pointer: 1,
      button: button,
      clientX: clientX,
      clientY: clientY,
      pointerType: 'mouse',
    );
  }

  html.Event _upWithFullDetails({double clientX, double clientY, int button, int pointer, String pointerType}) {
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
