import 'dart:ffi';
import 'dart:wasm';

import 'raw_geometry.dart';
import 'raw_memory.dart';

class RawPath extends Opaque {}

typedef PathHandle = Pointer<RawPath>;

typedef RawPathFillType = WasmI32;
typedef RawPathDirection = WasmI32;
typedef RawArcSize = WasmI32;
typedef RawPathOperation = WasmI32;

@pragma('wasm:import', 'skwasm.path_create')
external PathHandle path_create();

@pragma('wasm:import', 'skwasm.path_destroy')
external void path_destroy(PathHandle path);

@pragma('wasm:import', 'skwasm.path_copy')
external PathHandle path_copy(PathHandle path);

@pragma('wasm:import', 'skwasm.path_setFillType')
external void path_setFillType(PathHandle path, RawPathFillType fillType);

@pragma('wasm:import', 'skwasm.path_getFillType')
external RawPathFillType path_getFillType(PathHandle path);

@pragma('wasm:import', 'skwasm.path_moveTo')
external void path_moveTo(PathHandle path, RawScalar x, RawScalar y);

@pragma('wasm:import', 'skwasm.path_relativeMoveTo')
external void path_relativeMoveTo(PathHandle path, RawScalar x, RawScalar y);

@pragma('wasm:import', 'skwasm.path_lineTo')
external void path_lineTo(PathHandle path, RawScalar x, RawScalar y);

@pragma('wasm:import', 'skwasm.path_lineTo')
external void path_relativeLineTo(PathHandle path, RawScalar x, RawScalar y);

@pragma('wasm:import', 'skwasm.path_quadraticBezierTo')
external void path_quadraticBezierTo(
    PathHandle path, RawScalar x1, RawScalar y1, RawScalar x2, RawScalar y2);

@pragma('wasm:import', 'skwasm.path_relativeQuadraticBezierTo')
external void path_relativeQuadraticBezierTo(
    PathHandle path, RawScalar x1, RawScalar y1, RawScalar x2, RawScalar y2);

@pragma('wasm:import', 'skwasm.path_cubicTo')
external void path_cubicTo(PathHandle path, RawScalar x1, RawScalar y1,
    RawScalar x2, RawScalar y2, RawScalar x3, RawScalar y3);

@pragma('wasm:import', 'skwasm.path_relativeCubicTo')
external void path_relativeCubicTo(PathHandle path, RawScalar x1, RawScalar y1,
    RawScalar x2, RawScalar y2, RawScalar x3, RawScalar y3);

@pragma('wasm:import', 'skwasm.path_conicTo')
external void path_conicTo(PathHandle path, RawScalar x1, RawScalar y1,
    RawScalar x2, RawScalar y2, RawScalar w);

@pragma('wasm:import', 'skwasm.path_relativeConicTo')
external void path_relativeConicTo(PathHandle path, RawScalar x1, RawScalar y1,
    RawScalar x2, RawScalar y2, RawScalar w);

@pragma('wasm:import', 'skwasm.path_arcToOval')
external void path_arcToOval(PathHandle path, Pointer<Float> rect,
    RawScalar startAngle, RawScalar sweepAngle, RawBool forceMoveto);

@pragma('wasm:import', 'skwasm.path_arcToRotated')
external void path_arcToRotated(
    PathHandle path,
    RawScalar rx,
    RawScalar ry,
    RawScalar xAxisRotate,
    RawArcSize arcSize,
    RawPathDirection pathDirection,
    RawScalar x,
    RawScalar y);

@pragma('wasm:import', 'skwasm.path_relativeArcToRotated')
external void path_relativeArcToRotated(
    PathHandle path,
    RawScalar rx,
    RawScalar ry,
    RawScalar xAxisRotate,
    RawArcSize arcSize,
    RawPathDirection pathDirection,
    RawScalar x,
    RawScalar y);

@pragma('wasm:import', 'skwasm.path_addRect')
external void path_addRect(PathHandle path, RawRect oval);

@pragma('wasm:import', 'skwasm.path_addOval')
external void path_addOval(PathHandle path, RawRect oval);

@pragma('wasm:import', 'skwasm.path_addArc')
external void path_addArc(PathHandle path, RawRect ovalRect,
    RawScalar startAngleDegrees, RawScalar sweepAngleDegrees);

@pragma('wasm:import', 'skwasm.path_addPolygon')
external void path_addPolygon(
    PathHandle path, RawPointArray points, RawSize pointCount, RawBool close);

@pragma('wasm:import', 'skwasm.path_addRRect')
external void path_addRRect(
    PathHandle path, RawRRect rrectValues);

@pragma('wasm:import', 'skwasm.path_addPath')
external void path_addPath(PathHandle path, PathHandle other,
    RawMatrix33 matrix33, RawBool extendPath);

@pragma('wasm:import', 'skwasm.path_close')
external void path_close(PathHandle path);

@pragma('wasm:import', 'skwasm.path_reset')
external void path_reset(PathHandle path);

@pragma('wasm:import', 'skwasm.path_contains')
external RawBool path_contains(PathHandle path, RawScalar x, RawScalar y);

@pragma('wasm:import', 'skwasm.path_transform')
external void path_transform(PathHandle path, RawMatrix33 matrix33);

@pragma('wasm:import', 'skwasm.path_getBounds')
external void path_getBounds(PathHandle path, RawRect outRect);

@pragma('wasm:import', 'skwasm.path_combine')
external PathHandle path_combine(
    RawPathOperation operation, PathHandle path1, PathHandle path2);
