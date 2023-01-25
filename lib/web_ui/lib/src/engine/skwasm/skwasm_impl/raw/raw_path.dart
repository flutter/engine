import 'dart:ffi';

import 'raw_geometry.dart';

class RawPath extends Opaque {}

typedef PathHandle = Pointer<RawPath>;

@Native<PathHandle Function()>(symbol: 'skwasm.path_create', isLeaf: true)
external PathHandle pathCreate();

@Native<Void Function(PathHandle)>(symbol: 'skwasm.path_destroy', isLeaf: true)
external void pathDestroy(PathHandle path);

@Native<PathHandle Function(PathHandle)>(symbol: 'skwasm.path_copy', isLeaf: true)
external PathHandle pathCopy(PathHandle path);

@Native<Void Function(PathHandle, Int)>(symbol: 'skwasm.path_setFillType', isLeaf: true)
external void pathSetFillType(PathHandle path, int fillType);

@Native<Int Function(PathHandle)>(symbol: 'skwasm.path_getFillType', isLeaf: true)
external int pathGetFillType(PathHandle path);

@Native<Void Function(PathHandle, Float, Float)>(symbol: 'skwasm.path_moveTo', isLeaf: true)
external void pathMoveTo(PathHandle path, double x, double y);

@Native<Void Function(PathHandle, Float, Float)>(symbol: 'skwasm.path_relativeMoveTo', isLeaf: true)
external void pathRelativeMoveTo(PathHandle path, double x, double y);

@Native<Void Function(PathHandle, Float, Float)>(symbol: 'skwasm.path_lineTo', isLeaf: true)
external void pathLineTo(PathHandle path, double x, double y);

@Native<Void Function(PathHandle, Float, Float)>(
  symbol: 'skwasm.path_relativeLineTo',
  isLeaf: true)
external void pathRelativeLineTo(PathHandle path, double x, double y);

@Native<Void Function(PathHandle, Float, Float, Float, Float)>(
  symbol: 'skwasm.path_quadraticBezierTo',
  isLeaf: true)
external void pathQuadraticBezierTo(
    PathHandle path, double x1, double y1, double x2, double y2);

@Native<Void Function(PathHandle, Float, Float, Float, Float)>(
  symbol: 'skwasm.path_relativeQuadraticBezierTo',
  isLeaf: true)
external void pathRelativeQuadraticBezierTo(
    PathHandle path, double x1, double y1, double x2, double y2);

@Native<Void Function(PathHandle, Float, Float, Float, Float, Float, Float)>(
  symbol: 'skwasm.path_cubicTo',
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

@Native<Void Function(PathHandle, Float, Float, Float, Float, Float, Float)>(
  symbol: 'skwasm.path_relativeCubicTo',
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

@Native<Void Function(PathHandle, Float, Float, Float, Float, Float)>(
  symbol: 'skwasm.path_conicTo',
  isLeaf: true)
external void pathConicTo(
  PathHandle path,
  double x1,
  double y1,
  double x2,
  double y2,
  double w
);

@Native<Void Function(PathHandle, Float, Float, Float, Float, Float)>(
  symbol: 'skwasm.path_relativeConicTo',
  isLeaf: true)
external void pathRelativeConicTo(
  PathHandle path,
  double x1,
  double y1,
  double x2,
  double y2,
  double w
);

@Native<Void Function(PathHandle, RawRect, Float, Float, Bool)>(
  symbol: 'skwasm.path_arcToOval',
  isLeaf: true)
external void pathArcToOval(
  PathHandle path,
  RawRect rect,
  double startAngle,
  double sweepAngle,
  bool forceMoveto
);

@Native<Void Function(PathHandle, Float, Float, Float, Int, Int, Float, Float)>(
  symbol: 'skwasm.path_arcToRotated',
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

@Native<Void Function(PathHandle, Float, Float, Float, Int, Int, Float, Float)>(
  symbol: 'skwasm.path_relativeArcToRotated',
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

@Native<Void Function(PathHandle, RawRect)>(symbol: 'skwasm.path_addRect', isLeaf: true)
external void pathAddRect(PathHandle path, RawRect oval);

@Native<Void Function(PathHandle, RawRect)>(symbol: 'skwasm.path_addOval', isLeaf: true)
external void pathAddOval(PathHandle path, RawRect oval);

@Native<Void Function(PathHandle, RawRect, Float, Float)>(
  symbol: 'skwasm.path_addArc',
  isLeaf: true)
external void pathAddArc(
  PathHandle path,
  RawRect ovalRect,
  double startAngleDegrees,
  double sweepAngleDegrees
);

@Native<Void Function(PathHandle, RawPointArray, Int, Bool)>(
  symbol: 'skwasm.path_addPolygon',
  isLeaf: true)
external void pathAddPolygon(
  PathHandle path,
  RawPointArray points,
  int pointCount,
  bool close
);

@Native<Void Function(PathHandle, RawRRect)>(symbol: 'skwasm.path_addRRect', isLeaf: true)
external void pathAddRRect(PathHandle path, RawRRect rrectValues);

@Native<Void Function(PathHandle, PathHandle, RawMatrix33, Bool)>(
  symbol: 'skwasm.path_addPath',
  isLeaf: true)
external void pathAddPath(
  PathHandle path,
  PathHandle other,
  RawMatrix33 matrix33,
  bool extendPath
);

@Native<Void Function(PathHandle)>(symbol: 'skwasm.path_close', isLeaf: true)
external void pathClose(PathHandle path);

@Native<Void Function(PathHandle)>(symbol: 'skwasm.path_reset', isLeaf: true)
external void pathReset(PathHandle path);

@Native<Bool Function(PathHandle, Float, Float)>(symbol: 'skwasm.path_contains', isLeaf: true)
external bool pathContains(PathHandle path, double x, double y);

@Native<Void Function(PathHandle, RawMatrix33)>(symbol: 'skwasm.path_transform', isLeaf: true)
external void pathTransform(PathHandle path, RawMatrix33 matrix33);

@Native<Void Function(PathHandle, RawRect)>(symbol: 'skwasm.path_getBounds', isLeaf: true)
external void pathGetBounds(PathHandle path, RawRect outRect);

@Native<PathHandle Function(Int, PathHandle, PathHandle)>(
  symbol: 'skwasm.path_getBounds',
  isLeaf: true)
external PathHandle pathCombine(int operation, PathHandle path1, PathHandle path2);
