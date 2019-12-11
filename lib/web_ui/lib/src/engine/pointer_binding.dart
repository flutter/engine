// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// Set this flag to true to see all the fired events in the console.
const bool _debugLogPointerEvents = false;

/// The signature of a callback that handles pointer events.
typedef PointerDataCallback = void Function(List<ui.PointerData>);

// The mask for the bitfield of event buttons. Buttons not contained in this
// mask are cut off.
//
// In Flutter we used `kMaxUnsignedSMI`, but since that value is not available
// here, we use an already very large number (30 bits).
const int _kButtonsMask = 0x3FFFFFFF;

class PointerBinding {
  /// The singleton instance of this object.
  static PointerBinding get instance => _instance;
  static PointerBinding _instance;

  PointerBinding(this.domRenderer) {
    if (_instance == null) {
      _instance = this;
      _pointerDataConverter = PointerDataConverter();
      _detector = const PointerSupportDetector();
      _adapter = _createAdapter();
    }
    assert(() {
      registerHotRestartListener(() {
        _adapter?.clearListeners();
        _pointerDataConverter?.clearPointerState();
      });
      return true;
    }());
  }

  final DomRenderer domRenderer;
  PointerSupportDetector _detector;
  BaseAdapter _adapter;
  PointerDataConverter _pointerDataConverter;
  /// Should be used in tests to define custom detection of pointer support.
  ///
  /// ```dart
  /// // Forces PointerBinding to use mouse events.
  /// class MyTestDetector extends PointerSupportDetector {
  ///   @override
  ///   final bool hasPointerEvents = false;
  ///
  ///   @override
  ///   final bool hasTouchEvents = false;
  ///
  ///   @override
  ///   final bool hasMouseEvents = true;
  /// }
  ///
  /// PointerBinding.instance.debugOverrideDetector(MyTestDetector());
  /// ```
  void debugOverrideDetector(PointerSupportDetector newDetector) {
    newDetector ??= const PointerSupportDetector();
    // When changing the detector, we need to swap the adapter.
    if (newDetector != _detector) {
      _detector = newDetector;
      _adapter?.clearListeners();
      _adapter = _createAdapter();
      _pointerDataConverter?.clearPointerState();
    }
  }

  BaseAdapter _createAdapter() {
    if (_detector.hasPointerEvents) {
      return PointerAdapter(_onPointerData, domRenderer, _pointerDataConverter);
    }
    if (_detector.hasTouchEvents) {
      return TouchAdapter(_onPointerData, domRenderer, _pointerDataConverter);
    }
    if (_detector.hasMouseEvents) {
      return MouseAdapter(_onPointerData, domRenderer, _pointerDataConverter);
    }
    return null;
  }

  void _onPointerData(List<ui.PointerData> data) {
    final ui.PointerDataPacket packet = ui.PointerDataPacket(data: data);
    final ui.PointerDataPacketCallback callback = ui.window.onPointerDataPacket;
    if (callback != null) {
      callback(packet);
    }
  }
}

class PointerSupportDetector {
  const PointerSupportDetector();

  bool get hasPointerEvents => js_util.hasProperty(html.window, 'PointerEvent');
  bool get hasTouchEvents => js_util.hasProperty(html.window, 'TouchEvent');
  bool get hasMouseEvents => js_util.hasProperty(html.window, 'MouseEvent');

  @override
  String toString() =>
      'pointers:$hasPointerEvents, touch:$hasTouchEvents, mouse:$hasMouseEvents';
}

/// Common functionality that's shared among adapters.
abstract class BaseAdapter {
  BaseAdapter(this._callback, this.domRenderer, this._pointerDataConverter) {
    _setup();
  }

  /// Listeners that are registered through dart to js api.
  static final Map<String, html.EventListener> _listeners =
    <String, html.EventListener>{};
  /// Listeners that are registered through native javascript api.
  static final Map<String, html.EventListener> _nativeListeners =
    <String, html.EventListener>{};
  final DomRenderer domRenderer;
  PointerDataCallback _callback;
  PointerDataConverter _pointerDataConverter;

  // A map from devices to buttons that each device has pressed.
  Map<int, int> _pressedButtons = <int, int>{};

  int _deviceDownButtons(int device) {
    return _pressedButtons[device] ?? 0;
  }

  bool _isButtonDown(int device, int button) {
    return _deviceDownButtons(device) & button != 0;
  }

  // Update the down state of button bitfield `button` of device `device`.
  //
  // You can choose to set the new value by specifying `pressed`, or by
  // toggling the value by `togglePressed: true`, but you must choose one and
  // only one of them.
  void _updateButtonDownState(int device, int button, {
    bool pressed,
    bool togglePressed = false,
  }) {
    final int oldButtons = _deviceDownButtons(device);
    int newButtons;
    if (!togglePressed)  {
      assert(pressed != null);
      if (pressed) {
        newButtons = oldButtons | button;
      } else {
        newButtons = oldButtons & ~button;
      }
    } else {
      assert(togglePressed);
      newButtons = oldButtons ^ button;
    }
    _pressedButtons[device] = newButtons & _kButtonsMask;
  }

  void _setButtonDownState(int device, int buttons) {
    _pressedButtons[device] = buttons & _kButtonsMask;
  }

  /// Each subclass is expected to override this method to attach its own event
  /// listeners and convert events into pointer events.
  void _setup();

  /// Remove all active event listeners.
  void clearListeners() {
    final html.Element glassPane = domRenderer.glassPaneElement;
    _listeners.forEach((String eventName, html.EventListener listener) {
        glassPane.removeEventListener(eventName, listener, true);
    });
    // For native listener, we will need to remove it through native javascript
    // api.
    _nativeListeners.forEach((String eventName, html.EventListener listener) {
      js_util.callMethod(
        domRenderer.glassPaneElement,
        'removeEventListener', <dynamic>[
          'wheel',
          listener,
        ]
      );
    });
    _listeners.clear();
    _nativeListeners.clear();
  }

  void _addEventListener(String eventName, html.EventListener handler) {
    final html.EventListener loggedHandler = (html.Event event) {
      if (_debugLogPointerEvents) {
        print(event.type);
      }
      // Report the event to semantics. This information is used to debounce
      // browser gestures. Semantics tells us whether it is safe to forward
      // the event to the framework.
      if (EngineSemanticsOwner.instance.receiveGlobalEvent(event)) {
        handler(event);
      }
    };
    _listeners[eventName] = loggedHandler;
    domRenderer.glassPaneElement
        .addEventListener(eventName, loggedHandler, true);
  }

  /// Converts a floating number timestamp (in milliseconds) to a [Duration] by
  /// splitting it into two integer components: milliseconds + microseconds.
  Duration _eventTimeStampToDuration(num milliseconds) {
    final int ms = milliseconds.toInt();
    final int micro =
    ((milliseconds - ms) * Duration.microsecondsPerMillisecond).toInt();
    return Duration(milliseconds: ms, microseconds: micro);
  }

  List<ui.PointerData> _convertWheelEventToPointerData(
    html.WheelEvent event,
   ) {
    const int domDeltaPixel = 0x00;
    const int domDeltaLine = 0x01;
    const int domDeltaPage = 0x02;

    // Flutter only supports pixel scroll delta. Convert deltaMode values
    // to pixels.
    double deltaX = event.deltaX;
    double deltaY = event.deltaY;
    switch (event.deltaMode) {
      case domDeltaLine:
        deltaX *= 32.0;
        deltaY *= 32.0;
        break;
      case domDeltaPage:
        deltaX *= ui.window.physicalSize.width;
        deltaY *= ui.window.physicalSize.height;
        break;
      case domDeltaPixel:
      default:
        break;
    }
    final List<ui.PointerData> data = <ui.PointerData>[];
    _pointerDataConverter.convert(
      data,
      change: ui.PointerChange.hover,
      timeStamp: _eventTimeStampToDuration(event.timeStamp),
      kind: ui.PointerDeviceKind.mouse,
      signalKind: ui.PointerSignalKind.scroll,
      device: _mouseDeviceId,
      physicalX: event.client.x * ui.window.devicePixelRatio,
      physicalY: event.client.y * ui.window.devicePixelRatio,
      buttons: event.buttons,
      pressure: 1.0,
      pressureMin: 0.0,
      pressureMax: 1.0,
      scrollDeltaX: deltaX,
      scrollDeltaY: deltaY,
    );
    return data;
  }

  void _addWheelEventListener(html.EventListener handler) {
    final dynamic eventOptions = js_util.newObject();
    final html.EventListener jsHandler = js.allowInterop((html.Event event) => handler(event));
    _nativeListeners['wheel'] = jsHandler;
    js_util.setProperty(eventOptions, 'passive', false);
    js_util.callMethod(
      domRenderer.glassPaneElement,
      'addEventListener', <dynamic>[
        'wheel',
        jsHandler,
        eventOptions
      ]
    );

  }
}

const int _kPrimaryMouseButton = 0x1;
const int _kSecondaryMouseButton = 0x2;

typedef _GetDeviceButton = int Function();

int _pointerButtonFromHtmlEvent(html.Event event, {_GetDeviceButton getDefaultButton}) {
  if (event is html.PointerEvent) {
    final html.PointerEvent pointerEvent = event;
    switch (pointerEvent.button) {
      case 1:
        return _kPrimaryMouseButton;
      case 2:
        return _kSecondaryMouseButton;
      default:
        break;
    }
  } else if (event is html.MouseEvent) {
    final html.MouseEvent mouseEvent = event;
    switch (mouseEvent.button) {
      case 1:
        return _kPrimaryMouseButton;
      case 2:
        return _kSecondaryMouseButton;
      default:
        break;
    }
  }
  return getDefaultButton != null ? getDefaultButton() : _kPrimaryMouseButton;
}

int _deviceFromHtmlEvent(event) {
  if (event is html.PointerEvent) {
    final html.PointerEvent pointerEvent = event;
    return pointerEvent.pointerId;
  }
  return _mouseDeviceId;
}

/// Adapter class to be used with browsers that support native pointer events.
class PointerAdapter extends BaseAdapter {
  PointerAdapter(
    PointerDataCallback callback,
    DomRenderer domRenderer,
    PointerDataConverter _pointerDataConverter
  ) : super(callback, domRenderer, _pointerDataConverter);

  @override
  void _setup() {
    _addEventListener('pointerdown', (html.Event event) {
      final int device = _deviceFromHtmlEvent(event);
      final html.PointerEvent pointerEvent = event;
      // TODO(flutter_web): Remove this temporary fix for right click
      // on web platform once context guesture is implemented.
      final int currentButtons = _deviceDownButtons(device);
      if (currentButtons != 0) {
        _setButtonDownState(device, 0);
        _callback(_convertEventToPointerData(ui.PointerChange.up, event));
      }
      final int newButtons = _htmlButtonsToFlutterButtons(pointerEvent.buttons);
      _setButtonDownState(device, newButtons);
      _callback(_convertEventToPointerData(ui.PointerChange.down, event));
    });

    _addEventListener('pointermove', (html.Event event) {
      // During a drag operation pointermove will set button to the changed
      // button if there is a button change, or -1 otherwise (dragging).
      final html.PointerEvent pointerEvent = event;
      final int device = _deviceFromHtmlEvent(event);
      final int newButtons = _htmlButtonsToFlutterButtons(pointerEvent.buttons);
      _setButtonDownState(device, newButtons);
      _callback(_convertEventToPointerData(
        _deviceDownButtons(device) == 0
            ? ui.PointerChange.hover
            : ui.PointerChange.move,
        pointerEvent,
      ));
    });

    _addEventListener('pointerup', (html.Event event) {
      final int device = _deviceFromHtmlEvent(event);
      final int currentButtons = _deviceDownButtons(device);
      _setButtonDownState(device, 0);
      // The pointer could have been released by a `pointerout` event, in which
      // case `pointerup` should have no effect.
      if (currentButtons == 0) {
        return;
      }
      _callback(_convertEventToPointerData(ui.PointerChange.up, event));
    });

    // A browser fires cancel event if it concludes the pointer will no longer
    // be able to generate events (example: device is deactivated)
    _addEventListener('pointercancel', (html.Event event) {
      final int device = _deviceFromHtmlEvent(event);
      _setButtonDownState(device, 0);
      _callback(_convertEventToPointerData(ui.PointerChange.cancel, event));
    });

    _addWheelEventListener((html.Event event) {
      assert(event is html.WheelEvent);
      if (_debugLogPointerEvents) {
        print(event.type);
      }
      _callback(_convertWheelEventToPointerData(event));
      // Prevent default so mouse wheel event doesn't get converted to
      // a scroll event that semantic nodes would process.
      event.preventDefault();
    });
  }

  // Transform html.PointerEvent.buttons to Flutter's PointerEvent buttons.
  int _htmlButtonsToFlutterButtons(int buttons) {
    // Flutter's button definition conveniently matches that of JavaScript
    // from primary button (0x1) to forward button (0x10), which allows us to
    // avoid transforming it bit by bit.
    return buttons;
  }

  List<ui.PointerData> _convertEventToPointerData(
    ui.PointerChange change,
    html.PointerEvent evt,
  ) {
    final List<ui.PointerData> data = <ui.PointerData>[];
    for (html.PointerEvent event in _expandEvents(evt)) {
      _pointerDataConverter.convert(
        data,
        change: change,
        timeStamp: _eventTimeStampToDuration(event.timeStamp),
        kind: _pointerTypeToDeviceKind(event.pointerType),
        device: event.pointerId,
        physicalX: event.client.x * ui.window.devicePixelRatio,
        physicalY: event.client.y * ui.window.devicePixelRatio,
        buttons: _deviceDownButtons(event.pointerId),
        pressure: event.pressure,
        pressureMin: 0.0,
        pressureMax: 1.0,
        tilt: _computeHighestTilt(event),
      );
    }
    return data;
  }

  List<html.PointerEvent> _expandEvents(html.PointerEvent event) {
    // For browsers that don't support `getCoalescedEvents`, we fallback to
    // using the original event.
    if (js_util.hasProperty(event, 'getCoalescedEvents')) {
      final List<html.PointerEvent> coalescedEvents =
          event.getCoalescedEvents();
      // Some events don't perform coalescing, so they return an empty list. In
      // that case, we also fallback to using the original event.
      if (coalescedEvents.isNotEmpty) {
        return coalescedEvents;
      }
    }
    return <html.PointerEvent>[event];
  }

  ui.PointerDeviceKind _pointerTypeToDeviceKind(String pointerType) {
    switch (pointerType) {
      case 'mouse':
        return ui.PointerDeviceKind.mouse;
      case 'pen':
        return ui.PointerDeviceKind.stylus;
      case 'touch':
        return ui.PointerDeviceKind.touch;
      default:
        return ui.PointerDeviceKind.unknown;
    }
  }

  /// Tilt angle is -90 to + 90. Take maximum deflection and convert to radians.
  double _computeHighestTilt(html.PointerEvent e) =>
      (e.tiltX.abs() > e.tiltY.abs() ? e.tiltX : e.tiltY).toDouble() /
      180.0 *
      math.pi;
}

/// Adapter to be used with browsers that support touch events.
class TouchAdapter extends BaseAdapter {
  TouchAdapter(
    PointerDataCallback callback,
    DomRenderer domRenderer,
    PointerDataConverter _pointerDataConverter
  ) : super(callback, domRenderer, _pointerDataConverter);

  @override
  void _setup() {
    _addEventListener('touchstart', (html.Event event) {
      _updateButtonDownState(
          _deviceFromHtmlEvent(event), _kPrimaryMouseButton, pressed: true);
      _callback(_convertEventToPointerData(ui.PointerChange.down, event));
    });

    _addEventListener('touchmove', (html.Event event) {
      event.preventDefault(); // Prevents standard overscroll on iOS/Webkit.
      if (!_isButtonDown(_deviceFromHtmlEvent(event), _kPrimaryMouseButton)) {
        return;
      }
      _callback(_convertEventToPointerData(ui.PointerChange.move, event));
    });

    _addEventListener('touchend', (html.Event event) {
      // On Safari Mobile, the keyboard does not show unless this line is
      // added.
      event.preventDefault();
      _updateButtonDownState(
          _deviceFromHtmlEvent(event), _kPrimaryMouseButton, pressed: false);
      _callback(_convertEventToPointerData(ui.PointerChange.up, event));
    });

    _addEventListener('touchcancel', (html.Event event) {
      _callback(_convertEventToPointerData(ui.PointerChange.cancel, event));
    });
  }

  List<ui.PointerData> _convertEventToPointerData(
    ui.PointerChange change,
    html.TouchEvent event,
  ) {
    final html.TouchList touches = event.changedTouches;
    final List<ui.PointerData> data = List<ui.PointerData>();
    final int len = touches.length;
    for (int i = 0; i < len; i++) {
      final html.Touch touch = touches[i];
      _pointerDataConverter.convert(
        data,
        change: change,
        timeStamp: _eventTimeStampToDuration(event.timeStamp),
        kind: ui.PointerDeviceKind.touch,
        signalKind: ui.PointerSignalKind.none,
        device: touch.identifier,
        physicalX: touch.client.x * ui.window.devicePixelRatio,
        physicalY: touch.client.y * ui.window.devicePixelRatio,
        pressure: 1.0,
        pressureMin: 0.0,
        pressureMax: 1.0,
      );
    }

    return data;
  }
}

/// Intentionally set to -1 so it doesn't conflict with other device IDs.
const int _mouseDeviceId = -1;

/// Adapter to be used with browsers that support mouse events.
class MouseAdapter extends BaseAdapter {
  MouseAdapter(
    PointerDataCallback callback,
    DomRenderer domRenderer,
    PointerDataConverter _pointerDataConverter
  ) : super(callback, domRenderer, _pointerDataConverter);

  @override
  void _setup() {
    _addEventListener('mousedown', (html.Event event) {
      final int pointerButton = _pointerButtonFromHtmlEvent(event);
      final int device = _deviceFromHtmlEvent(event);
      if (_isButtonDown(device, pointerButton)) {
        // TODO(flutter_web): Remove this temporary fix for right click
        // on web platform once context guesture is implemented.
        _callback(_convertEventToPointerData(ui.PointerChange.up, event));
      }
      _updateButtonDownState(device, pointerButton, pressed: true);
      _callback(_convertEventToPointerData(ui.PointerChange.down, event));
    });

    _addEventListener('mousemove', (html.Event event) {
      final int pointerButton = _pointerButtonFromHtmlEvent(event);
      final int device = _deviceFromHtmlEvent(event);
      final List<ui.PointerData> data = _convertEventToPointerData(
          _isButtonDown(device, pointerButton)
              ? ui.PointerChange.move
              : ui.PointerChange.hover,
          event);
      _callback(data);
    });

    _addEventListener('mouseup', (html.Event event) {
      final int device = _deviceFromHtmlEvent(event);
      _updateButtonDownState(device, _pointerButtonFromHtmlEvent(event), pressed: false);
      _callback(_convertEventToPointerData(ui.PointerChange.up, event));
    });

    _addWheelEventListener((html.Event event) {
      assert(event is html.WheelEvent);
      if (_debugLogPointerEvents) {
        print(event.type);
      }
      _callback(_convertWheelEventToPointerData(event));
      event.preventDefault();
    });
  }

  List<ui.PointerData> _convertEventToPointerData(
    ui.PointerChange change,
    html.MouseEvent event,
  ) {
    List<ui.PointerData> data = <ui.PointerData>[];
    _pointerDataConverter.convert(
      data,
      change: change,
      timeStamp: _eventTimeStampToDuration(event.timeStamp),
      kind: ui.PointerDeviceKind.mouse,
      signalKind: ui.PointerSignalKind.none,
      device: _mouseDeviceId,
      physicalX: event.client.x * ui.window.devicePixelRatio,
      physicalY: event.client.y * ui.window.devicePixelRatio,
      buttons: event.buttons,
      pressure: 1.0,
      pressureMin: 0.0,
      pressureMax: 1.0,
    );
    return data;
  }
}
