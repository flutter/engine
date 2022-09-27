import 'dart:ffi';
import 'dart:wasm';

import 'raw_geometry.dart';
import 'raw_memory.dart';
import 'raw_paint.dart';
import 'raw_path.dart';
import 'raw_picture.dart';

class CanvasWrapper extends Opaque {}

typedef CanvasHandle = Pointer<CanvasWrapper>;

typedef RawClipOp = WasmI32;

@pragma('wasm:import', 'skwasm.canvas_destroy')
external void canvas_destroy(CanvasHandle canvas);

@pragma('wasm:import', 'skwasm.canvas_save')
external void canvas_save(CanvasHandle canvas);

@pragma('wasm:import', 'skwasm.canvas_saveLayer')
external void canvas_saveLayer(
    CanvasHandle canvas, RawRect rect, PaintHandle paint);

@pragma('wasm:import', 'skwasm.canvas_restore')
external void canvas_restore(CanvasHandle canvas);

@pragma('wasm:import', 'skwasm.canvas_getSaveCount')
external RawSize canvas_getSaveCount(CanvasHandle canvas);

@pragma('wasm:import', 'skwasm.canvas_translate')
external void canvas_translate(CanvasHandle canvas, RawScalar dx, RawScalar dy);

@pragma('wasm:import', 'skwasm.canvas_scale')
external void canvas_scale(CanvasHandle canvas, RawScalar sx, RawScalar sy);

@pragma('wasm:import', 'skwasm.canvas_rotate')
external void canvas_rotate(CanvasHandle canvas, RawScalar degrees);

@pragma('wasm:import', 'skwasm.canvas_skew')
external void canvas_skew(CanvasHandle canvas, RawScalar sx, RawScalar sy);

@pragma('wasm:import', 'skwasm.canvas_transform')
external void canvas_transform(CanvasHandle canvas, RawMatrix44 matrix);

@pragma('wasm:import', 'skwasm.canvas_clipRect')
external void canvas_clipRect(
    CanvasHandle canvas, RawRect rect, RawClipOp op, RawBool antialias);

@pragma('wasm:import', 'skwasm.canvas_clipRRect')
external void canvas_clipRRect(
    CanvasHandle canvas, RawRRect rrect, RawBool antialias);

@pragma('wasm:import', 'skwasm.canvas_clipPath')
external void canvas_clipPath(
    CanvasHandle canvas, PathHandle path, RawBool antialias);

@pragma('wasm:import', 'skwasm.canvas_drawColor')
external void canvas_drawColor(
    CanvasHandle canvas, RawColor color, RawBlendMode blendMode);

@pragma('wasm:import', 'skwasm.canvas_drawLine')
external void canvas_drawLine(CanvasHandle canvas, RawScalar x1, RawScalar y1,
    RawScalar x2, RawScalar y2, PaintHandle paint);

@pragma('wasm:import', 'skwasm.canvas_drawPaint')
external void canvas_drawPaint(CanvasHandle canvas, PaintHandle paint);

@pragma('wasm:import', 'skwasm.canvas_drawRect')
external void canvas_drawRect(
    CanvasHandle canvas, RawRect rect, PaintHandle paint);

@pragma('wasm:import', 'skwasm.canvas_drawRRect')
external void canvas_drawRRect(
    CanvasHandle canvas, RawRRect rrect, PaintHandle paint);

@pragma('wasm:import', 'skwasm.canvas_drawDRRect')
external void canvas_drawDRRect(
    CanvasHandle canvas, RawRRect outer, RawRRect inner, PaintHandle paint);

@pragma('wasm:import', 'skwasm.canvas_drawOval')
external void canvas_drawOval(
    CanvasHandle canvas, RawRect oval, PaintHandle paint);

@pragma('wasm:import', 'skwasm.canvas_drawCircle')
external void canvas_drawCircle(CanvasHandle canvas, RawScalar x, RawScalar y,
    RawScalar radius, PaintHandle paint);

@pragma('wasm:import', 'skwasm.canvas_drawArc')
external void canvas_drawArc(
    CanvasHandle canvas,
    RawRect rect,
    RawScalar startAngleDegrees,
    RawScalar sweepAngleDegrees,
    RawBool useCenter,
    PaintHandle paint);

@pragma('wasm:import', 'skwasm.canvas_drawPath')
external void canvas_drawPath(
    CanvasHandle canvas, PathHandle path, PaintHandle paint);

@pragma('wasm:import', 'skwasm.canvas_drawPicture')
external void canvas_drawPicture(CanvasHandle canvas, PictureHandle picture);
