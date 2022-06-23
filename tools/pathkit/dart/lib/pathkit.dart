// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: camel_case_types
import 'dart:ffi' as ffi;
import 'dart:io';

/// Determines the winding rule that decides how the interior of a Path is
/// calculated.
///
/// This enum is used by the [Path] constructor
// must match ordering in //third_party/skia/include/core/SkPathTypes.h
enum FillType {
  /// The interior is defined by a non-zero sum of signed edge crossings.
  nonZero,

  /// The interior is defined by an odd number of edge crossings.
  evenOdd,
}

// Sync with //third_party/skia/include/pathops/SkPathOps.h
enum PathOp {
  difference,
  intersect,
  union,
  xor,
  reversedDifference,
}

/// Creates a path object to operate on.
///
/// First, build up the path contours with the [moveTo], [lineTo], [cubicTo],
/// and [close] methods. All methods expect absolute coordinates.
///
/// Finally, use the [dispose] method to clean up native resources. After
/// [dispose] has been called, this class must not be used again.
class Path {
  Path(FillType fillType) : _path = _createPathFn(fillType.index);

  ffi.Pointer<_SkPath>? _path;

  /// Adds a move verb to the absolute coordinates x,y.
  void moveTo(double x, double y) {
    assert(_path != null);
    _moveToFn(_path!, x, y);
  }

  /// Adds a line verb to the absolute coordinates x,y.
  void lineTo(double x, double y) {
    assert(_path != null);
    _lineToFn(_path!, x, y);
  }

  /// Adds a cubic Bezier curve with x1,y1 as the first control point, x2,y2 as
  /// the second control point, and end point x3,y3.
  void cubicTo(
    double x1,
    double y1,
    double x2,
    double y2,
    double x3,
    double y3,
  ) {
    assert(_path != null);
    _cubicToFn(_path!, x1, y1, x2, y2, x3, y3);
  }

  /// Adds a close command to the start of the current contour.
  void close() {
    assert(_path != null);
    _closeFn(_path!, true);
  }

  void reset() {
    assert(_path != null);
    _resetFn(_path!);
  }

  /// Releases native resources.
  ///
  /// After calling dispose, this class must not be used again.
  void dispose() {
    assert(_path != null);
    _destroyFn(_path!);
    _path = null;
  }

  void applyOp(Path other, PathOp op) {
    assert(_path != null);
    assert(other._path != null);
    _opFn(_path!, other._path!, op.index);
  }

  void dump() {
    assert(_path != null);
    _dumpFn(_path!);
  }
}

// TODO(dnfield): Figure out where to put this.
// https://github.com/flutter/flutter/issues/99563
final ffi.DynamicLibrary _dylib = () {
  if (Platform.isWindows) {
    return ffi.DynamicLibrary.open('pathkit.dll');
  } else if (Platform.isIOS || Platform.isMacOS) {
    return ffi.DynamicLibrary.open('libpathkit.dylib');
  } else if (Platform.isAndroid || Platform.isLinux) {
    return ffi.DynamicLibrary.open('libpathkit.so');
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}();

class _SkPath extends ffi.Opaque {}

typedef _CreatePathType = ffi.Pointer<_SkPath> Function(int);
typedef _create_path_type = ffi.Pointer<_SkPath> Function(ffi.Int);

final _CreatePathType _createPathFn =
    _dylib.lookupFunction<_create_path_type, _CreatePathType>(
  'CreatePath',
);

typedef _MoveToType = void Function(ffi.Pointer<_SkPath>, double, double);
typedef _move_to_type = ffi.Void Function(
  ffi.Pointer<_SkPath>,
  ffi.Float,
  ffi.Float,
);

final _MoveToType _moveToFn = _dylib.lookupFunction<_move_to_type, _MoveToType>(
  'MoveTo',
);

typedef _LineToType = void Function(ffi.Pointer<_SkPath>, double, double);
typedef _line_to_type = ffi.Void Function(
  ffi.Pointer<_SkPath>,
  ffi.Float,
  ffi.Float,
);

final _LineToType _lineToFn = _dylib.lookupFunction<_line_to_type, _LineToType>(
  'LineTo',
);

typedef _CubicToType = void Function(
  ffi.Pointer<_SkPath>,
  double,
  double,
  double,
  double,
  double,
  double,
);
typedef _cubic_to_type = ffi.Void Function(
  ffi.Pointer<_SkPath>,
  ffi.Float,
  ffi.Float,
  ffi.Float,
  ffi.Float,
  ffi.Float,
  ffi.Float,
);

final _CubicToType _cubicToFn =
    _dylib.lookupFunction<_cubic_to_type, _CubicToType>('CubicTo');

typedef _CloseType = void Function(ffi.Pointer<_SkPath>, bool);
typedef _close_type = ffi.Void Function(ffi.Pointer<_SkPath>, ffi.Bool);

final _CloseType _closeFn =
    _dylib.lookupFunction<_close_type, _CloseType>('Close');

typedef _ResetType = void Function(ffi.Pointer<_SkPath>);
typedef _reset_type = ffi.Void Function(ffi.Pointer<_SkPath>);

final _ResetType _resetFn =
    _dylib.lookupFunction<_reset_type, _ResetType>('Reset');

typedef _DestroyType = void Function(ffi.Pointer<_SkPath>);
typedef _destroy_type = ffi.Void Function(ffi.Pointer<_SkPath>);

final _DestroyType _destroyFn =
    _dylib.lookupFunction<_destroy_type, _DestroyType>(
  'DestroyPath',
);

typedef _OpType = void Function(
    ffi.Pointer<_SkPath>, ffi.Pointer<_SkPath>, int);
typedef _op_type = ffi.Void Function(
    ffi.Pointer<_SkPath>, ffi.Pointer<_SkPath>, ffi.Int);

final _OpType _opFn = _dylib.lookupFunction<_op_type, _OpType>('Op');

typedef _DumpType = void Function(ffi.Pointer<_SkPath>);
typedef _dump_type = ffi.Void Function(ffi.Pointer<_SkPath>);

final _DumpType _dumpFn = _dylib.lookupFunction<_dump_type, _DumpType>(
  'dump',
);
