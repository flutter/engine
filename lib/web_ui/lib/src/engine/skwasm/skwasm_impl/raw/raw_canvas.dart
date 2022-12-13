import 'dart:ffi';

import 'raw_geometry.dart';
import 'raw_paint.dart';
import 'raw_path.dart';
import 'raw_picture.dart';

class CanvasWrapper extends Opaque {}
typedef CanvasHandle = Pointer<CanvasWrapper>;

@FfiNative<Void Function(CanvasHandle)>('skwasm.canvas_destroy', isLeaf: true)
external void canvasDestroy(CanvasHandle canvas);

@FfiNative<Void Function(CanvasHandle)>('skwasm.canvas_save', isLeaf: true)
external void canvasSave(CanvasHandle canvas);

@FfiNative<Void Function(CanvasHandle, RawRect, PaintHandle)>('skwasm.canvas_saveLayer', isLeaf: true)
external void canvasSaveLayer(
    CanvasHandle canvas, RawRect rect, PaintHandle paint);

@FfiNative<Void Function(CanvasHandle)>('skwasm.canvas_restore', isLeaf: true)
external void canvasRestore(CanvasHandle canvas);

@FfiNative<Int Function(CanvasHandle)>('skwasm.canvas_getSaveCount', isLeaf: true)
external int canvasGetSaveCount(CanvasHandle canvas);

@FfiNative<Void Function(CanvasHandle, Float, Float)>('skwasm.canvas_translate', isLeaf: true)
external void canvasTranslate(CanvasHandle canvas, double dx, double dy);

@FfiNative<Void Function(CanvasHandle, Float, Float)>('skwasm.canvas_scale', isLeaf: true)
external void canvasScale(CanvasHandle canvas, double sx, double sy);

@FfiNative<Void Function(CanvasHandle, Float)>('skwasm.canvas_rotate', isLeaf: true)
external void canvasRotate(CanvasHandle canvas, double degrees);

@FfiNative<Void Function(CanvasHandle, Float, Float)>('skwasm.canvas_skew', isLeaf: true)
external void canvasSkew(CanvasHandle canvas, double sx, double sy);

@FfiNative<Void Function(CanvasHandle, RawMatrix44)>('skwasm.canvas_transform', isLeaf: true)
external void canvasTransform(CanvasHandle canvas, RawMatrix44 matrix);

@FfiNative<Void Function(CanvasHandle, RawRect, Int, Bool)>('skwasm.canvas_clipRect', isLeaf: true)
external void canvasClipRect(
    CanvasHandle canvas, RawRect rect, int op, bool antialias);

@FfiNative<Void Function(CanvasHandle, RawRRect, Bool)>('skwasm.canvas_clipRRect', isLeaf: true)
external void canvasClipRRect(
    CanvasHandle canvas, RawRRect rrect, bool antialias);

@FfiNative<Void Function(CanvasHandle, PathHandle, Bool)>('skwasm.canvas_clipPath', isLeaf: true)
external void canvasClipPath(
    CanvasHandle canvas, PathHandle path, bool antialias);

@FfiNative<Void Function(CanvasHandle, Int32, Int)>('skwasm.canvas_drawColor', isLeaf: true)
external void canvasDrawColor(
    CanvasHandle canvas, int color, int blendMode);

@FfiNative<Void Function(CanvasHandle, Float, Float, Float, Float, PaintHandle)>('skwasm.canvas_drawLine', isLeaf: true)
external void canvasDrawLine(CanvasHandle canvas, double x1, double y1,
    double x2, double y2, PaintHandle paint);

@FfiNative<Void Function(CanvasHandle, PaintHandle)>('skwasm.canvas_drawPaint', isLeaf: true)
external void canvasDrawPaint(CanvasHandle canvas, PaintHandle paint);

@FfiNative<Void Function(CanvasHandle, RawRect, PaintHandle)>('skwasm.canvas_drawRect', isLeaf: true)
external void canvasDrawRect(
    CanvasHandle canvas, RawRect rect, PaintHandle paint);

@FfiNative<Void Function(CanvasHandle, RawRRect, PaintHandle)>('skwasm.canvas_drawRRect', isLeaf: true)
external void canvasDrawRRect(
    CanvasHandle canvas, RawRRect rrect, PaintHandle paint);

@FfiNative<Void Function(CanvasHandle, RawRRect, RawRRect, PaintHandle)>('skwasm.canvas_drawDRRect', isLeaf: true)
external void canvasDrawDRRect(
    CanvasHandle canvas, RawRRect outer, RawRRect inner, PaintHandle paint);

@FfiNative<Void Function(CanvasHandle, RawRect, PaintHandle)>('skwasm.canvas_drawOval', isLeaf: true)
external void canvasDrawOval(
    CanvasHandle canvas, RawRect oval, PaintHandle paint);

@FfiNative<Void Function(CanvasHandle, Float, Float, Float, PaintHandle)>('skwasm.canvas_drawCircle', isLeaf: true)
external void canvasDrawCircle(CanvasHandle canvas, double x, double y,
    double radius, PaintHandle paint);

@FfiNative<Void Function(CanvasHandle, RawRect, Float, Float, Bool, PaintHandle)>('skwasm.canvas_drawCircle', isLeaf: true)
external void canvasDrawArc(
    CanvasHandle canvas,
    RawRect rect,
    double startAngleDegrees,
    double sweepAngleDegrees,
    bool useCenter,
    PaintHandle paint);

@FfiNative<Void Function(CanvasHandle, PathHandle, PaintHandle)>('skwasm.canvas_drawPath', isLeaf: true)
external void canvasDrawPath(
    CanvasHandle canvas, PathHandle path, PaintHandle paint);

@FfiNative<Void Function(CanvasHandle, PictureHandle)>('skwasm.canvas_drawPicture', isLeaf: true)
external void canvasDrawPicture(CanvasHandle canvas, PictureHandle picture);
