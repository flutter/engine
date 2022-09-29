import 'dart:ffi';

class RawPaint extends Opaque {}

typedef PaintHandle = Pointer<RawPaint>;

@FfiNative<PaintHandle Function()>('skwasm.paint_create', isLeaf: true)
external PaintHandle paintCreate();

@FfiNative<Void Function(PaintHandle)>('skwasm.paint_destroy', isLeaf: true)
external void paintDestroy(PaintHandle paint);

@FfiNative<Void Function(PaintHandle, Int)>('skwasm.paint_setBlendMode', isLeaf: true)
external void paintSetBlendMode(PaintHandle paint, int blendMode);

@FfiNative<Void Function(PaintHandle, Int)>('skwasm.paint_setPaintStyle', isLeaf: true)
external void paintSetPaintStyle(PaintHandle paint, int paintStyle);

@FfiNative<Int Function(PaintHandle)>('skwasm.paint_getPaintStyle', isLeaf: true)
external int paintGetPaintStyle(PaintHandle paint);

@FfiNative<Void Function(PaintHandle, Float)>('skwasm.paint_setStrokeWidth', isLeaf: true)
external void paintSetStrokeWidth(PaintHandle paint, double strokeWidth);

@FfiNative<Float Function(PaintHandle)>('skwasm.paint_getStrokeWidth', isLeaf: true)
external double paintGetStrokeWidth(PaintHandle paint);

@FfiNative<Void Function(PaintHandle, Int)>('skwasm.paint_setStrokeCap', isLeaf: true)
external void paintSetStrokeCap(PaintHandle paint, int cap);

@FfiNative<Int Function(PaintHandle)>('skwasm.paint_getStrokeCap', isLeaf: true)
external int paintGetStrokeCap(PaintHandle paint);

@FfiNative<Void Function(PaintHandle, Int)>('skwasm.paint_setStrokeJoin', isLeaf: true)
external void paintSetStrokeJoin(PaintHandle paint, int join);

@FfiNative<Int Function(PaintHandle)>('skwasm.paint_getStrokeJoin', isLeaf: true)
external int paintGetStrokeJoin(PaintHandle paint);

@FfiNative<Void Function(PaintHandle, Bool)>('skwasm.paint_setAntiAlias', isLeaf: true)
external void paintSetAntiAlias(PaintHandle paint, bool antiAlias);

@FfiNative<Bool Function(PaintHandle)>('skwasm.paint_getAntiAlias', isLeaf: true)
external bool paintGetAntiAlias(PaintHandle paint);

@FfiNative<Void Function(PaintHandle, Uint32)>('skwasm.paint_setColorInt', isLeaf: true)
external void paintSetColorInt(PaintHandle paint, int color);

@FfiNative<Uint32 Function(PaintHandle)>('skwasm.paint_getColorInt', isLeaf: true)
external int paintGetColorInt(PaintHandle paint);

@FfiNative<Void Function(PaintHandle, Float)>('skwasm.paint_setMiterLimit', isLeaf: true)
external void paintSetMiterLimit(PaintHandle paint, double miterLimit);

@FfiNative<Float Function(PaintHandle)>('skwasm.paint_getMiterLimit', isLeaf: true)
external double paintGetMiterLimit(PaintHandle paint);
