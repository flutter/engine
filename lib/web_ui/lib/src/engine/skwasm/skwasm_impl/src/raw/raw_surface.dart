import 'dart:ffi';
import 'dart:wasm';
import 'raw_picture.dart';

class RawSurface extends Opaque {}

typedef SurfaceHandle = Pointer<RawSurface>;

@pragma('wasm:import', 'skwasm.surface_createFromCanvas')
external SurfaceHandle surface_createFromCanvas(Pointer<Int8> querySelector);

@pragma('wasm:import', 'skwasm.surface_destroy')
external void surface_destroy(SurfaceHandle surface);

@pragma('wasm:import', 'skwasm.surface_setCanvasSize')
external void surface_setCanvasSize(SurfaceHandle surface, WasmI32 width, WasmI32 height);

@pragma('wasm:import', 'skwasm.surface_renderPicture')
external void surface_renderPicture(SurfaceHandle surface, PictureHandle picture);
