import 'dart:ffi';

import 'raw_geometry.dart';

class RawPath extends Opaque {}

typedef PathHandle = Pointer<RawPath>;

@FfiNative<PathHandle Function()>('skwasm.path_create', isLeaf: true)
external PathHandle pathCreate();

@FfiNative<Void Function(PathHandle)>('skwasm.path_destroy', isLeaf: true)
external void pathDestroy(PathHandle path);

@FfiNative<PathHandle Function(PathHandle)>('skwasm.path_copy', isLeaf: true)
external PathHandle pathCopy(PathHandle path);

@FfiNative<Void Function(PathHandle, Int)>('skwasm.path_setFillType', isLeaf: true)
external void pathSetFillType(PathHandle path, int fillType);

@FfiNative<Int Function(PathHandle)>('skwasm.path_getFillType', isLeaf: true)
external int pathGetFillType(PathHandle path);

@FfiNative<Void Function(PathHandle, Float, Float)>('skwasm.path_moveTo', isLeaf: true)
external void pathMoveTo(PathHandle path, double x, double y);

@FfiNative<Void Function(PathHandle, Float, Float)>('skwasm.path_relativeMoveTo', isLeaf: true)
external void pathRelativeMoveTo(PathHandle path, double x, double y);

@FfiNative<Void Function(PathHandle, Float, Float)>('skwasm.path_lineTo', isLeaf: true)
external void pathLineTo(PathHandle path, double x, double y);

@FfiNative<Void Function(PathHandle, Float, Float)>(
  'skwasm.path_relativeLineTo',
  isLeaf: true)
external void pathRelativeLineTo(PathHandle path, double x, double y);

@FfiNative<Void Function(PathHandle, Float, Float, Float, Float)>(
  'skwasm.path_quadraticBezierTo',
  isLeaf: true)
external void pathQuadraticBezierTo(
    PathHandle path, double x1, double y1, double x2, double y2);

@FfiNative<Void Function(PathHandle, Float, Float, Float, Float)>(
  'skwasm.path_relativeQuadraticBezierTo',
  isLeaf: true)
external void pathRelativeQuadraticBezierTo(
    PathHandle path, double x1, double y1, double x2, double y2);

@FfiNative<Void Function(PathHandle, Float, Float, Float, Float, Float, Float)>(
  'skwasm.path_cubicTo',
  isLeaf: true)
external void pathCubicTo(
  PathHandle path,
  double x1,
  double y1,
  double x2,
  double y2,
  double x3,
  double y3
);

@FfiNative<Void Function(PathHandle, Float, Float, Float, Float, Float, Float)>(
  'skwasm.path_relativeCubicTo',
  isLeaf: true)
external void pathRelativeCubicTo(
  PathHandle path,
  double x1,
  double y1,
  double x2,
  double y2,
  double x3,
  double y3
);

@FfiNative<Void Function(PathHandle, Float, Float, Float, Float, Float)>(
  'skwasm.path_conicTo',
  isLeaf: true)
external void pathConicTo(
  PathHandle path,
  double x1,
  double y1,
  double x2,
  double y2,
  double w
);

@FfiNative<Void Function(PathHandle, Float, Float, Float, Float, Float)>(
  'skwasm.path_relativeConicTo',
  isLeaf: true)
external void pathRelativeConicTo(
  PathHandle path,
  double x1,
  double y1,
  double x2,
  double y2,
  double w
);

@FfiNative<Void Function(PathHandle, RawRect, Float, Float, Bool)>(
  'skwasm.path_arcToOval',
  isLeaf: true)
external void pathArcToOval(
  PathHandle path,
  RawRect rect,
  double startAngle,
  double sweepAngle,
  bool forceMoveto
);

@FfiNative<Void Function(PathHandle, Float, Float, Float, Int, Int, Float, Float)>(
  'skwasm.path_arcToRotated',
  isLeaf: true)
external void pathArcToRotated(
    PathHandle path,
    double rx,
    double ry,
    double xAxisRotate,
    int arcSize,
    int pathDirection,
    double x,
    double y
);

@FfiNative<Void Function(PathHandle, Float, Float, Float, Int, Int, Float, Float)>(
  'skwasm.path_relativeArcToRotated',
  isLeaf: true)
external void pathRelativeArcToRotated(
    PathHandle path,
    double rx,
    double ry,
    double xAxisRotate,
    int arcSize,
    int pathDirection,
    double x,
    double y
);

@FfiNative<Void Function(PathHandle, RawRect)>('skwasm.path_addRect', isLeaf: true)
external void pathAddRect(PathHandle path, RawRect oval);

@FfiNative<Void Function(PathHandle, RawRect)>('skwasm.path_addOval', isLeaf: true)
external void pathAddOval(PathHandle path, RawRect oval);

@FfiNative<Void Function(PathHandle, RawRect, Float, Float)>(
  'skwasm.path_addArc',
  isLeaf: true)
external void pathAddArc(
  PathHandle path,
  RawRect ovalRect,
  double startAngleDegrees,
  double sweepAngleDegrees
);

@FfiNative<Void Function(PathHandle, RawPointArray, Int, Bool)>(
  'skwasm.path_addPolygon',
  isLeaf: true)
external void pathAddPolygon(
  PathHandle path,
  RawPointArray points,
  int pointCount,
  bool close
);

@FfiNative<Void Function(PathHandle, RawRRect)>('skwasm.path_addRRect', isLeaf: true)
external void pathAddRRect(PathHandle path, RawRRect rrectValues);

@FfiNative<Void Function(PathHandle, PathHandle, RawMatrix33, Bool)>(
  'skwasm.path_addPath',
  isLeaf: true)
external void pathAddPath(
  PathHandle path,
  PathHandle other,
  RawMatrix33 matrix33,
  bool extendPath
);

@FfiNative<Void Function(PathHandle)>('skwasm.path_close', isLeaf: true)
external void pathClose(PathHandle path);

@FfiNative<Void Function(PathHandle)>('skwasm.path_reset', isLeaf: true)
external void pathReset(PathHandle path);

@FfiNative<Bool Function(PathHandle, Float, Float)>('skwasm.path_contains', isLeaf: true)
external bool pathContains(PathHandle path, double x, double y);

@FfiNative<Void Function(PathHandle, RawMatrix33)>('skwasm.path_transform', isLeaf: true)
external void pathTransform(PathHandle path, RawMatrix33 matrix33);

@FfiNative<Void Function(PathHandle, RawRect)>('skwasm.path_getBounds', isLeaf: true)
external void pathGetBounds(PathHandle path, RawRect outRect);

@FfiNative<PathHandle Function(Int, PathHandle, PathHandle)>(
  'skwasm.path_getBounds',
  isLeaf: true)
external PathHandle pathCombine(int operation, PathHandle path1, PathHandle path2);
