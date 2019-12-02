// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

class _PointerState {
  _PointerState(this.x, this.y);

  int get pointer => _pointer; // The identifier used in PointerEvent objects.
  int _pointer;
  static int _pointerCount = 0;
  void startNewPointer() {
    _pointerCount += 1;
    _pointer = _pointerCount;
  }

  bool down = false;

  double x;
  double y;
}

/// Converter to convert web pointer data to a framework-compatible form.
///
/// This converter calculates pointer location delta and pointer identifier for
/// each pointer. Both are required by framework to correctly trigger gesture
/// activity. It also attempts to sanitize pointer data input sequence by always
/// synthesizing an add pointer data prior to hover or down if it the pointer is
/// not previously added.
///
/// For example:
///   before:
///     hover -> down -> move -> up
///   after:
///     add(synthesize) -> hover -> down -> move -> up
///
///   before:
///     down -> move -> up
///   after:
///     add(synthesize) -> down -> move -> up
class PointerDataConverter {
  PointerDataConverter._();

  // Map from platform pointer identifiers to PointerEvent pointer identifiers.
  // Static to guarantee that pointers are unique.
  static final Map<int, _PointerState> _pointers = <int, _PointerState>{};

  static _PointerState _ensureStateForPointer(ui.PointerData datum, double x, double y) {
    return _pointers.putIfAbsent(
      datum.device,
        () => _PointerState(x, y),
    );
  }

  static ui.PointerData _generateCompletePointerData(ui.PointerData datum) {
    assert(_pointers.containsKey(datum.device));
    final _PointerState state = _pointers[datum.device];
    final double deltaX = datum.physicalX - state.x;
    final double deltaY = datum.physicalY - state.y;
    state.x = datum.physicalX;
    state.y = datum.physicalY;
    return ui.PointerData(
      timeStamp: datum.timeStamp,
      change: datum.change,
      kind: datum.kind,
      signalKind: datum.signalKind,
      device: datum.device,
      pointerIdentifier: state.pointer ?? 0,
      physicalX: datum.physicalX,
      physicalY: datum.physicalY,
      physicalDeltaX: deltaX,
      physicalDeltaY: deltaY,
      buttons: datum.buttons,
      obscured: datum.obscured,
      synthesized: datum.synthesized,
      pressure: datum.pressure,
      pressureMin: datum.pressureMin,
      pressureMax: datum.pressureMax,
      distance: datum.distance,
      distanceMax: datum.distanceMax,
      size: datum.size,
      radiusMajor: datum.radiusMajor,
      radiusMinor: datum.radiusMinor,
      radiusMin: datum.radiusMin,
      radiusMax: datum.radiusMax,
      orientation: datum.orientation,
      tilt: datum.tilt,
      platformData: datum.platformData,
      scrollDeltaX: datum.scrollDeltaX,
      scrollDeltaY: datum.scrollDeltaY,
    );
  }

  static bool _locationHasChanged(ui.PointerData datum) {
    assert(_pointers.containsKey(datum.device));
    final _PointerState state = _pointers[datum.device];
    return state.x != datum.physicalX || state.y != datum.physicalY;
  }

  static ui.PointerData _synthesizeFrom(ui.PointerData datum, ui.PointerChange change) {
    assert(_pointers.containsKey(datum.device));
    final _PointerState state = _pointers[datum.device];
    final double deltaX = datum.physicalX - state.x;
    final double deltaY = datum.physicalY - state.y;
    state.x = datum.physicalX;
    state.y = datum.physicalY;
    return ui.PointerData(
      timeStamp: datum.timeStamp,
      change: change,
      kind: datum.kind,
      // All the pointer data except scroll should not have a signal kind, and
      // there is no use case for synthetic scroll event. We should be
      // safe to default it to ui.PointerSignalKind.none.
      signalKind: ui.PointerSignalKind.none,
      device: datum.device,
      pointerIdentifier: state.pointer ?? 0,
      physicalX: datum.physicalX,
      physicalY: datum.physicalY,
      physicalDeltaX: deltaX,
      physicalDeltaY: deltaY,
      buttons: datum.buttons,
      obscured: datum.obscured,
      synthesized: true,
      pressure: datum.pressure,
      pressureMin: datum.pressureMin,
      pressureMax: datum.pressureMax,
      distance: datum.distance,
      distanceMax: datum.distanceMax,
      size: datum.size,
      radiusMajor: datum.radiusMajor,
      radiusMinor: datum.radiusMinor,
      radiusMin: datum.radiusMin,
      radiusMax: datum.radiusMax,
      orientation: datum.orientation,
      tilt: datum.tilt,
      platformData: datum.platformData,
      scrollDeltaX: datum.scrollDeltaX,
      scrollDeltaY: datum.scrollDeltaY,
    );
  }

  /// Convert the given list pointer data into a sequence of framework-compatible
  /// pointer data.
  static Iterable<ui.PointerData> convert(List<ui.PointerData> data) sync* {
    for (ui.PointerData datum in data) {
      assert(datum.change != null);
      if (datum.signalKind == null ||
        datum.signalKind == ui.PointerSignalKind.none) {
        switch (datum.change) {
          case ui.PointerChange.add:
            assert(!_pointers.containsKey(datum.device));
            _ensureStateForPointer(datum, datum.physicalX, datum.physicalY);
            assert(!_locationHasChanged(datum));
            yield _generateCompletePointerData(datum);
            break;
          case ui.PointerChange.hover:
            final bool alreadyAdded = _pointers.containsKey(datum.device);
            final _PointerState state = _ensureStateForPointer(
              datum, datum.physicalX, datum.physicalY);
            assert(!state.down);
            if (!alreadyAdded) {
              // Synthesizes an add pointer data.
              yield _synthesizeFrom(datum, ui.PointerChange.add);
            }
            yield _generateCompletePointerData(datum);
            break;
          case ui.PointerChange.down:
            final bool alreadyAdded = _pointers.containsKey(datum.device);
            final _PointerState state = _ensureStateForPointer(
              datum, datum.physicalX, datum.physicalY);
            assert(!state.down);
            if (!alreadyAdded) {
              // Synthesizes an add pointer data.
              yield _synthesizeFrom(datum, ui.PointerChange.add);
            }
            assert(!_locationHasChanged(datum));
            state.startNewPointer();
            state.down = true;
            yield _generateCompletePointerData(datum);
            break;
          case ui.PointerChange.move:
            final bool alreadyAdded = _pointers.containsKey(datum.device);
            final _PointerState state = _ensureStateForPointer(
              datum, datum.physicalX, datum.physicalY);
            if (!alreadyAdded) {
              // Synthesizes an add pointer data and down pointer data.
              yield _synthesizeFrom(datum, ui.PointerChange.add);
              yield _synthesizeFrom(datum, ui.PointerChange.down);
            }
            assert(state.down);
            yield _generateCompletePointerData(datum);
            break;
          case ui.PointerChange.up:
          case ui.PointerChange.cancel:
            assert(_pointers.containsKey(datum.device));
            final _PointerState state = _pointers[datum.device];
            assert(state.down);
            assert(!_locationHasChanged(datum));
            state.down = false;
            yield _generateCompletePointerData(datum);
            break;
          case ui.PointerChange.remove:
            assert(_pointers.containsKey(datum.device));
            final _PointerState state = _pointers[datum.device];
            assert(!state.down);
            assert(!_locationHasChanged(datum));
            _pointers.remove(datum.device);
            yield _generateCompletePointerData(datum);
            break;
        }
      } else {
        switch (datum.signalKind) {
          case ui.PointerSignalKind.scroll:
            final bool alreadyAdded = _pointers.containsKey(datum.device);
            final _PointerState state = _ensureStateForPointer(
              datum, datum.physicalX, datum.physicalY);
            if (!alreadyAdded) {
              // Synthesizes an add pointer data.
              yield _synthesizeFrom(datum, ui.PointerChange.add);
            }
            if (_locationHasChanged(datum)) {
              // Synthesize a hover/move of the pointer to the scroll location
              // before sending the scroll event, if necessary, so that clients
              // don't have to worry about native ordering of hover and scroll
              // events.
              if (state.down) {
                yield _synthesizeFrom(datum, ui.PointerChange.move);
              } else {
                yield _synthesizeFrom(datum, ui.PointerChange.hover);
              }
            }
            yield _generateCompletePointerData(datum);
            break;
          case ui.PointerSignalKind.none:
            assert(false); // This branch should already have 'none' filtered out.
            break;
          case ui.PointerSignalKind.unknown:
          // Ignore unknown signals.
            break;
        }
      }
    }
  }

}