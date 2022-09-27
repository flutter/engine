import 'dart:ffi';
import 'dart:wasm';
import 'raw_memory.dart';

class RawPaint extends Opaque {}

typedef PaintHandle = Pointer<RawPaint>;

typedef RawBlendMode = WasmI32;
typedef RawPaintStyle = WasmI32;
typedef RawStrokeCap = WasmI32;
typedef RawStrokeJoin = WasmI32;
typedef RawColor = WasmI32;

@pragma('wasm:import', 'skwasm.paint_create')
external PaintHandle paint_create();

@pragma('wasm:import', 'skwasm.paint_destroy')
external void paint_destroy(PaintHandle paint);

@pragma('wasm:import', 'skwasm.paint_setBlendMode')
external void paint_setBlendMode(PaintHandle paint, RawBlendMode blendMode);

@pragma('wasm:import', 'skwasm.paint_setStyle')
external void paint_setPaintStyle(PaintHandle paint, RawPaintStyle paintStyle);

@pragma('wasm:import', 'skwasm.paint_getStyle')
external RawPaintStyle paint_getPaintStyle(PaintHandle paint);

@pragma('wasm:import', 'skwasm.paint_setStrokeWidth')
external void paint_setStrokeWidth(PaintHandle paint, RawScalar strokeWidth);

@pragma('wasm:import', 'skwasm.paint_getStrokeWidth')
external RawScalar paint_getStrokeWidth(PaintHandle paint);

@pragma('wasm:import', 'skwasm.paint_setStrokeCap')
external void paint_setStrokeCap(PaintHandle paint, RawStrokeCap cap);

@pragma('wasm:import', 'skwasm.paint_getStrokeCap')
external RawStrokeCap paint_getStrokeCap(PaintHandle paint);

@pragma('wasm:import', 'skwasm.paint_setStrokeJoin')
external void paint_setStrokeJoin(PaintHandle paint, RawStrokeJoin join);

@pragma('wasm:import', 'skwasm.paint_getStrokeJoin')
external RawStrokeJoin paint_getStrokeJoin(PaintHandle paint);

@pragma('wasm:import', 'skwasm.paint_setAntiAlias')
external void paint_setAntiAlias(PaintHandle paint, RawBool antiAlias);

@pragma('wasm:import', 'skwasm.paint_getAntiAlias')
external RawBool paint_getAntiAlias(PaintHandle paint);

@pragma('wasm:import', 'skwasm.paint_setColorInt')
external void paint_setColorInt(PaintHandle paint, RawColor color);

@pragma('wasm:import', 'skwasm.paint_getColorInt')
external RawColor paint_getColorInt(PaintHandle paint);

@pragma('wasm:import', 'skwasm.paint_setMiterLimit')
external void paint_setMiterLimit(PaintHandle paint, RawScalar miterLimit);

@pragma('wasm:import', 'skwasm.paint_getMiterLimit')
external RawScalar paint_getMiterLimit(PaintHandle paint);
