// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:collection' show IterableBase;
import 'dart:math' as math;
import 'dart:typed_data';

import 'package:ui/ui.dart' as ui;

import 'conic.dart';
import 'cubic.dart';
import 'path_iterator.dart';
import 'path_ref.dart';
import 'path_utils.dart';

const double kEpsilon = 0.000000001;

/// An iterable collection of [PathMetric] objects describing a [Path].
///
/// A [PathMetrics] object is created by using the [Path.computeMetrics] method,
/// and represents the path as it stood at the time of the call. Subsequent
/// modifications of the path do not affect the [PathMetrics] object.
///
/// Each path metric corresponds to a segment, or contour, of a path.
///
/// For example, a path consisting of a [Path.lineTo], a [Path.moveTo], and
/// another [Path.lineTo] will contain two contours and thus be represented by
/// two [PathMetric] objects.
///
/// When iterating across a [PathMetrics]' contours, the [PathMetric] objects
/// are only valid until the next one is obtained.
class SurfacePathMetrics extends IterableBase<ui.PathMetric>
    implements ui.PathMetrics {
  SurfacePathMetrics(PathRef path, bool forceClosed)
      : _iterator =
            SurfacePathMetricIterator._(_SurfacePathMeasure(PathRef.shallowCopy(path), forceClosed));

  final SurfacePathMetricIterator _iterator;

  @override
  Iterator<ui.PathMetric> get iterator => _iterator;
}

/// Maintains a single instance of computed segments for set of PathMetric
/// objects exposed through iterator.
class _SurfacePathMeasure {
  _SurfacePathMeasure(this._path, this.forceClosed)
      :
        // nextContour will increment this to the zero based index.
        _currentContourIndex = -1,
        _pathIterator = PathIterator(_path, forceClosed);

  final PathRef _path;
  final PathIterator _pathIterator;
  final List<_PathContourMeasure> _contours = <_PathContourMeasure>[];

  // If the contour ends with a call to [Path.close] (which may
  // have been implied when using [Path.addRect])
  final bool forceClosed;

  int _currentContourIndex;
  int get currentContourIndex => _currentContourIndex;

  double length(int contourIndex) {
    assert(contourIndex <= currentContourIndex,
        'Iterator must be advanced before index $contourIndex can be used.');
    return _contours[contourIndex].length;
  }

  /// Computes the position of hte current contour at the given offset, and the
  /// angle of the path at that point.
  ///
  /// For example, calling this method with a distance of 1.41 for a line from
  /// 0.0,0.0 to 2.0,2.0 would give a point 1.0,1.0 and the angle 45 degrees
  /// (but in radians).
  ///
  /// Returns null if the contour has zero [length].
  ///
  /// The distance is clamped to the [length] of the current contour.
  ui.Tangent? getTangentForOffset(int contourIndex, double distance) {
    return _contours[contourIndex].getTangentForOffset(distance);
  }

  bool isClosed(int contourIndex) => _contours[contourIndex].isClosed;

  // Move to the next contour in the path.
  //
  // A path can have a next contour if [Path.moveTo] was called after drawing began.
  // Return true if one exists, or false.
  bool _nextContour() {
    final next = _nativeNextContour();
    if (next) {
      _currentContourIndex++;
    }
    return next;
  }

  // Iterator index into next contour.
  int _verbIterIndex = 0;

  // Move to the next contour in the path.
  //
  // A path can have a next contour if [Path.moveTo] was called after drawing
  // began. Return true if one exists, or false.
  //
  // This is not exactly congruent with a regular [Iterator.moveNext].
  // Typically, [Iterator.moveNext] should be called before accessing the
  // [Iterator.current]. In this case, the [PathMetric] is valid before
  // calling `_moveNext` - `_moveNext` should be called after the first
  // iteration is done instead of before.
  bool _nativeNextContour() {
    if (_verbIterIndex == _path.countVerbs()) {
      return false;
    }
    final measure =
        _PathContourMeasure(_path, _pathIterator, forceClosed);
    _verbIterIndex = measure.verbEndIndex;
    _contours.add(measure);
    return true;
  }

  ui.Path extractPath(int contourIndex, double start, double end,
      {bool startWithMoveTo = true}) {
    return _contours[contourIndex].extractPath(start, end, startWithMoveTo);
  }
}

/// Builds segments for a single contour to measure distance, compute tangent
/// and extract a sub path.
class _PathContourMeasure {
  _PathContourMeasure(this.pathRef, PathIterator iter, this.forceClosed) {
    _verbEndIndex = _buildSegments(iter);
  }

  final PathRef pathRef;
  int _verbEndIndex = 0;
  final List<_PathSegment> _segments = <_PathSegment>[];
  // Allocate buffer large enough for returning cubic curve chop result.
  // 2 floats for each coordinate x (start, end & control point 1 & 2).
  static final Float32List _buffer = Float32List(8);

  final bool forceClosed;
  double get length => _contourLength;
  bool get isClosed => _isClosed;
  int get verbEndIndex => _verbEndIndex;

  double _contourLength = 0.0;
  bool _isClosed = false;

  ui.Tangent? getTangentForOffset(double distance) {
    final segmentIndex = _segmentIndexAtDistance(distance);
    if (segmentIndex == -1) {
      return null;
    }
    return _getPosTan(segmentIndex, distance);
  }

  // Returns segment at [distance].
  int _segmentIndexAtDistance(double distance) {
    if (distance.isNaN) {
      return -1;
    }
    // Pin distance to legal range.
    if (distance < 0.0) {
      distance = 0.0;
    } else if (distance > _contourLength) {
      distance = _contourLength;
    }

    // Binary search through segments to find segment at distance.
    if (_segments.isEmpty) {
      return -1;
    }
    var lo = 0;
    var hi = _segments.length - 1;
    while (lo < hi) {
      final mid = (lo + hi) >> 1;
      if (_segments[mid].distance < distance) {
        lo = mid + 1;
      } else {
        hi = mid;
      }
    }
    if (_segments[hi].distance < distance) {
      hi++;
    }
    return hi;
  }

  _SurfaceTangent _getPosTan(int segmentIndex, double distance) {
    final segment = _segments[segmentIndex];
    // Compute distance to segment. Since distance is cumulative to find
    // t = 0..1 on the segment, we need to calculate start distance using prior
    // segment.
    final startDistance =
        segmentIndex == 0 ? 0 : _segments[segmentIndex - 1].distance;
    final totalDistance = segment.distance - startDistance;
    final t = totalDistance < kEpsilon
        ? 0
        : (distance - startDistance) / totalDistance;
    return segment.computeTangent(t);
  }

  ui.Path extractPath(
      double startDistance, double stopDistance, bool startWithMoveTo) {
    if (startDistance < 0) {
      startDistance = 0;
    }
    if (stopDistance > _contourLength) {
      stopDistance = _contourLength;
    }
    final path = ui.Path();
    if (startDistance > stopDistance || _segments.isEmpty) {
      return path;
    }
    final startSegmentIndex = _segmentIndexAtDistance(startDistance);
    final stopSegmentIndex = _segmentIndexAtDistance(stopDistance);
    if (startSegmentIndex == -1 || stopSegmentIndex == -1) {
      return path;
    }
    var currentSegmentIndex = startSegmentIndex;
    var seg = _segments[currentSegmentIndex];
    final startTangent =
        _getPosTan(startSegmentIndex, startDistance);
    if (startWithMoveTo) {
      final startPosition = startTangent.position;
      path.moveTo(startPosition.dx, startPosition.dy);
    }
    final stopTangent =
        _getPosTan(stopSegmentIndex, stopDistance);
    var startT = startTangent.t;
    final stopT = stopTangent.t;
    if (startSegmentIndex == stopSegmentIndex) {
      // We only have a single segment that covers the complete distance.
      _outputSegmentTo(seg, startT, stopT, path);
    } else {
      do {
        // Write this segment from startT to end (t = 1.0).
        _outputSegmentTo(seg, startT, 1.0, path);
        // Move to next segment until we hit stop segment.
        ++currentSegmentIndex;
        seg = _segments[currentSegmentIndex];
        startT = 0;
      } while (currentSegmentIndex != stopSegmentIndex);
      // Final write last segment from t=0.0 to t=stopT.
      _outputSegmentTo(seg, 0.0, stopT, path);
    }
    return path;
  }

  // Chops the segment at startT and endT and writes it to output [path].
  void _outputSegmentTo(
      _PathSegment segment, double startT, double stopT, ui.Path path) {
    final points = segment.points;
    switch (segment.segmentType) {
      case SPath.kLineVerb:
        final toX = (points[2] * stopT) + (points[0] * (1.0 - stopT));
        final toY = (points[3] * stopT) + (points[1] * (1.0 - stopT));
        path.lineTo(toX, toY);
      case SPath.kCubicVerb:
        chopCubicBetweenT(points, startT, stopT, _buffer);
        path.cubicTo(_buffer[2], _buffer[3], _buffer[4], _buffer[5], _buffer[6],
            _buffer[7]);
      case SPath.kQuadVerb:
        _chopQuadBetweenT(points, startT, stopT, _buffer);
        path.quadraticBezierTo(_buffer[2], _buffer[3], _buffer[4], _buffer[5]);
      case SPath.kConicVerb:
        // Implement this once we start writing out conic segments.
        throw UnimplementedError();
      default:
        throw UnsupportedError('Invalid segment type');
    }
  }

  /// Builds segments from contour starting at verb [_verbStartIndex] and
  /// returns next contour verb index.
  int _buildSegments(PathIterator iter) {
    assert(_segments.isEmpty, '_buildSegments should be called once');
    _isClosed = false;
    var distance = 0.0;
    var haveSeenMoveTo = false;

    void lineToHandler(double fromX, double fromY, double x, double y) {
      final dx = fromX - x;
      final dy = fromY - y;
      final prevDistance = distance;
      distance += math.sqrt(dx * dx + dy * dy);
      // As we accumulate distance, we have to check that the result of +=
      // actually made it larger, since a very small delta might be > 0, but
      // still have no effect on distance (if distance >>> delta).
      if (distance > prevDistance) {
        _segments.add(
          _PathSegment(SPath.kLineVerb, distance, <double>[fromX, fromY, x, y]),
        );
      }
    }

    var verb = 0;
    final points = Float32List(PathRefIterator.kMaxBufferSize);
    do {
      if (iter.peek() == SPath.kMoveVerb && haveSeenMoveTo) {
        break;
      }
      verb = iter.next(points);
      switch (verb) {
        case SPath.kMoveVerb:
          haveSeenMoveTo = true;
        case SPath.kLineVerb:
          assert(haveSeenMoveTo);
          lineToHandler(points[0], points[1], points[2], points[3]);
        case SPath.kCubicVerb:
          assert(haveSeenMoveTo);
          // Compute cubic curve distance.
          distance = _computeCubicSegments(
              points[0],
              points[1],
              points[2],
              points[3],
              points[4],
              points[5],
              points[6],
              points[7],
              distance,
              0,
              _kMaxTValue,
              _segments);
        case SPath.kConicVerb:
          assert(haveSeenMoveTo);
          final w = iter.conicWeight;
          final conic = Conic(points[0], points[1], points[2], points[3],
              points[4], points[5], w);
          final conicPoints = conic.toQuads();
          final len = conicPoints.length;
          var startX = conicPoints[0].dx;
          var startY = conicPoints[0].dy;
          for (var i = 1; i < len; i += 2) {
            final p1x = conicPoints[i].dx;
            final p1y = conicPoints[i].dy;
            final p2x = conicPoints[i + 1].dx;
            final p2y = conicPoints[i + 1].dy;
            distance = _computeQuadSegments(
                startX, startY, p1x, p1y, p2x, p2y, distance, 0, _kMaxTValue);
            startX = p2x;
            startY = p2y;
          }
        case SPath.kQuadVerb:
          assert(haveSeenMoveTo);
          // Compute quad curve distance.
          distance = _computeQuadSegments(points[0], points[1], points[2],
              points[3], points[4], points[5], distance, 0, _kMaxTValue);
        case SPath.kCloseVerb:
          _contourLength = distance;
          return iter.pathVerbIndex;
        default:
          break;
      }
    } while (verb != SPath.kDoneVerb);
    _contourLength = distance;
    return iter.pathVerbIndex;
  }

  static bool _tspanBigEnough(int tSpan) => (tSpan >> 10) != 0;

  static bool _cubicTooCurvy(double x0, double y0, double x1, double y1,
      double x2, double y2, double x3, double y3) {
    // Measure distance from start-end line at 1/3 and 2/3rds to control
    // points. If distance is less than _fTolerance we should continue
    // subdividing curve. Uses approx distance for speed.
    //
    // p1 = point 1/3rd between start,end points.
    final p1x = (x0 * 2 / 3) + (x3 / 3);
    final p1y = (y0 * 2 / 3) + (y3 / 3);
    if ((p1x - x1).abs() > _fTolerance) {
      return true;
    }
    if ((p1y - y1).abs() > _fTolerance) {
      return true;
    }
    // p2 = point 2/3rd between start,end points.
    final p2x = (x0 / 3) + (x3 * 2 / 3);
    final p2y = (y0 / 3) + (y3 * 2 / 3);
    if ((p2x - x2).abs() > _fTolerance) {
      return true;
    }
    if ((p2y - y2).abs() > _fTolerance) {
      return true;
    }
    return false;
  }

  // Recursively subdivides cubic and adds segments.
  static double _computeCubicSegments(
      double x0,
      double y0,
      double x1,
      double y1,
      double x2,
      double y2,
      double x3,
      double y3,
      double distance,
      int tMin,
      int tMax,
      List<_PathSegment> segments) {
    if (_tspanBigEnough(tMax - tMin) &&
        _cubicTooCurvy(x0, y0, x1, y1, x2, y2, x3, y3)) {
      // Chop cubic into two halves (De Cateljau's algorithm)
      // See https://en.wikipedia.org/wiki/De_Casteljau%27s_algorithm
      final abX = (x0 + x1) / 2;
      final abY = (y0 + y1) / 2;
      final bcX = (x1 + x2) / 2;
      final bcY = (y1 + y2) / 2;
      final cdX = (x2 + x3) / 2;
      final cdY = (y2 + y3) / 2;
      final abcX = (abX + bcX) / 2;
      final abcY = (abY + bcY) / 2;
      final bcdX = (bcX + cdX) / 2;
      final bcdY = (bcY + cdY) / 2;
      final abcdX = (abcX + bcdX) / 2;
      final abcdY = (abcY + bcdY) / 2;
      final tHalf = (tMin + tMax) >> 1;
      distance = _computeCubicSegments(x0, y0, abX, abY, abcX, abcY, abcdX,
          abcdY, distance, tMin, tHalf, segments);
      distance = _computeCubicSegments(abcdX, abcdY, bcdX, bcdY, cdX, cdY, x3,
          y3, distance, tHalf, tMax, segments);
    } else {
      final dx = x0 - x3;
      final dy = y0 - y3;
      final startToEndDistance = math.sqrt(dx * dx + dy * dy);
      final prevDistance = distance;
      distance += startToEndDistance;
      if (distance > prevDistance) {
        segments.add(_PathSegment(SPath.kCubicVerb, distance,
            <double>[x0, y0, x1, y1, x2, y2, x3, y3]));
      }
    }
    return distance;
  }

  static bool _quadTooCurvy(
      double x0, double y0, double x1, double y1, double x2, double y2) {
    // (a/4 + b/2 + c/4) - (a/2 + c/2)  =  -a/4 + b/2 - c/4
    final dx = (x1 / 2) - (x0 + x2) / 4;
    if (dx.abs() > _fTolerance) {
      return true;
    }
    final dy = (y1 / 2) - (y0 + y2) / 4;
    if (dy.abs() > _fTolerance) {
      return true;
    }
    return false;
  }

  double _computeQuadSegments(double x0, double y0, double x1, double y1,
      double x2, double y2, double distance, int tMin, int tMax) {
    if (_tspanBigEnough(tMax - tMin) && _quadTooCurvy(x0, y0, x1, y1, x2, y2)) {
      final p01x = (x0 + x1) / 2;
      final p01y = (y0 + y1) / 2;
      final p12x = (x1 + x2) / 2;
      final p12y = (y1 + y2) / 2;
      final p012x = (p01x + p12x) / 2;
      final p012y = (p01y + p12y) / 2;
      final tHalf = (tMin + tMax) >> 1;
      distance = _computeQuadSegments(
          x0, y0, p01x, p01y, p012x, p012y, distance, tMin, tHalf);
      distance = _computeQuadSegments(
          p012x, p012y, p12x, p12y, x2, y2, distance, tMin, tHalf);
    } else {
      final dx = x0 - x2;
      final dy = y0 - y2;
      final startToEndDistance = math.sqrt(dx * dx + dy * dy);
      final prevDistance = distance;
      distance += startToEndDistance;
      if (distance > prevDistance) {
        _segments.add(_PathSegment(
            SPath.kQuadVerb, distance, <double>[x0, y0, x1, y1, x2, y2]));
      }
    }
    return distance;
  }
}

/// Tracks iteration from one segment of a path to the next for measurement.
class SurfacePathMetricIterator implements Iterator<ui.PathMetric> {
  SurfacePathMetricIterator._(this._pathMeasure);

  SurfacePathMetric? _pathMetric;
  final _SurfacePathMeasure _pathMeasure;

  @override
  SurfacePathMetric get current {
    if (_pathMetric == null) {
      throw RangeError(
          'PathMetricIterator is not pointing to a PathMetric. This can happen in two situations:\n'
          '- The iteration has not started yet. If so, call "moveNext" to start iteration.\n'
          '- The iterator ran out of elements. If so, check that "moveNext" returns true prior to calling "current".');
    }
    return _pathMetric!;
  }

  @override
  bool moveNext() {
    if (_pathMeasure._nextContour()) {
      _pathMetric = SurfacePathMetric._(_pathMeasure);
      return true;
    }
    _pathMetric = null;
    return false;
  }
}

// Maximum range value used in curve subdivision using Casteljau algorithm.
const int _kMaxTValue = 0x3FFFFFFF;
// Distance at which we stop subdividing cubic and quadratic curves.
const double _fTolerance = 0.5;

/// Utilities for measuring a [Path] and extracting sub-paths.
///
/// Iterate over the object returned by [Path.computeMetrics] to obtain
/// [PathMetric] objects. Callers that want to randomly access elements or
/// iterate multiple times should use `path.computeMetrics().toList()`, since
/// [PathMetrics] does not memoize.
///
/// Once created, the metrics are only valid for the path as it was specified
/// when [Path.computeMetrics] was called. If additional contours are added or
/// any contours are updated, the metrics need to be recomputed. Previously
/// created metrics will still refer to a snapshot of the path at the time they
/// were computed, rather than to the actual metrics for the new mutations to
/// the path.
///
/// Implementation is based on
/// https://github.com/google/skia/blob/main/src/core/SkContourMeasure.cpp
/// to maintain consistency with native platforms.
class SurfacePathMetric implements ui.PathMetric {
  SurfacePathMetric._(this._measure)
      : length = _measure.length(_measure.currentContourIndex),
        isClosed = _measure.isClosed(_measure.currentContourIndex),
        contourIndex = _measure.currentContourIndex;

  /// Return the total length of the current contour.
  @override
  final double length;

  /// Whether the contour is closed.
  ///
  /// Returns true if the contour ends with a call to [Path.close] (which may
  /// have been implied when using methods like [Path.addRect]) or if
  /// `forceClosed` was specified as true in the call to [Path.computeMetrics].
  /// Returns false otherwise.
  @override
  final bool isClosed;

  /// The zero-based index of the contour.
  ///
  /// [Path] objects are made up of zero or more contours. The first contour is
  /// created once a drawing command (e.g. [Path.lineTo]) is issued. A
  /// [Path.moveTo] command after a drawing command may create a new contour,
  /// although it may not if optimizations are applied that determine the move
  /// command did not actually result in moving the pen.
  ///
  /// This property is only valid with reference to its original iterator and
  /// the contours of the path at the time the path's metrics were computed. If
  /// additional contours were added or existing contours updated, this metric
  /// will be invalid for the current state of the path.
  @override
  final int contourIndex;

  final _SurfacePathMeasure _measure;

  /// Computes the position of the current contour at the given offset, and the
  /// angle of the path at that point.
  ///
  /// For example, calling this method with a distance of 1.41 for a line from
  /// 0.0,0.0 to 2.0,2.0 would give a point 1.0,1.0 and the angle 45 degrees
  /// (but in radians).
  ///
  /// Returns null if the contour has zero [length].
  ///
  /// The distance is clamped to the [length] of the current contour.
  @override
  ui.Tangent? getTangentForOffset(double distance) {
    return _measure.getTangentForOffset(contourIndex, distance);
  }

  /// Given a start and end distance, return the intervening segment(s).
  ///
  /// `start` and `end` are pinned to legal values (0..[length])
  /// Begin the segment with a moveTo if `startWithMoveTo` is true.
  @override
  ui.Path extractPath(double start, double end, {bool startWithMoveTo = true}) {
    return _measure.extractPath(contourIndex, start, end,
        startWithMoveTo: startWithMoveTo);
  }

  @override
  String toString() => 'PathMetric';
}

// Given a vector dx, dy representing slope, normalize and return as [ui.Offset].
ui.Offset _normalizeSlope(double dx, double dy) {
  final length = math.sqrt(dx * dx + dy * dy);
  return length < kEpsilon
      ? ui.Offset.zero
      : ui.Offset(dx / length, dy / length);
}

class _SurfaceTangent extends ui.Tangent {
  const _SurfaceTangent(super.position, super.vector, this.t);

  // Normalized distance of tangent point from start of a contour.
  final double t;
}

class _PathSegment {
  _PathSegment(this.segmentType, this.distance, this.points);

  final int segmentType;
  final double distance;
  final List<double> points;

  _SurfaceTangent computeTangent(double t) {
    switch (segmentType) {
      case SPath.kLineVerb:
        // Simple line. Position is simple interpolation from start to end point.
        final xAtDistance = (points[2] * t) + (points[0] * (1.0 - t));
        final yAtDistance = (points[3] * t) + (points[1] * (1.0 - t));
        return _SurfaceTangent(ui.Offset(xAtDistance, yAtDistance),
            _normalizeSlope(points[2] - points[0], points[3] - points[1]), t);
      case SPath.kCubicVerb:
        return tangentForCubicAt(t, points[0], points[1], points[2], points[3],
            points[4], points[5], points[6], points[7]);
      case SPath.kQuadVerb:
        return tangentForQuadAt(t, points[0], points[1], points[2], points[3],
            points[4], points[5]);
      default:
        throw UnsupportedError('Invalid segment type');
    }
  }

  _SurfaceTangent tangentForQuadAt(double t, double x0, double y0, double x1,
      double y1, double x2, double y2) {
    assert(t >= 0 && t <= 1);
    final quadEval =
        SkQuadCoefficients(x0, y0, x1, y1, x2, y2);
    final pos = ui.Offset(quadEval.evalX(t), quadEval.evalY(t));
    // Derivative of quad curve is 2(b - a + (a - 2b + c)t).
    // If control point is at start or end point, this yields 0 for t = 0 and
    // t = 1. In that case use the quad end points to compute tangent instead
    // of derivative.
    final tangentVector = ((t == 0 && x0 == x1 && y0 == y1) ||
            (t == 1 && x1 == x2 && y1 == y2))
        ? _normalizeSlope(x2 - x0, y2 - y0)
        : _normalizeSlope(
            2 * ((x2 - x0) * t + (x1 - x0)), 2 * ((y2 - y0) * t + (y1 - y0)));
    return _SurfaceTangent(pos, tangentVector, t);
  }

  _SurfaceTangent tangentForCubicAt(double t, double x0, double y0, double x1,
      double y1, double x2, double y2, double x3, double y3) {
    assert(t >= 0 && t <= 1);
    final cubicEval =
        _SkCubicCoefficients(x0, y0, x1, y1, x2, y2, x3, y3);
    final pos = ui.Offset(cubicEval.evalX(t), cubicEval.evalY(t));
    // Derivative of cubic is zero when t = 0 or 1 and adjacent control point
    // is on the start or end point of curve. Use the other control point
    // to compute the tangent or if both control points are on end points
    // use end points for tangent.
    final tAtZero = t == 0;
    ui.Offset tangentVector;
    if ((tAtZero && x0 == x1 && y0 == y1) || (t == 1 && x2 == x3 && y2 == y3)) {
      var dx = tAtZero ? x2 - x0 : x3 - x1;
      var dy = tAtZero ? y2 - y0 : y3 - y1;
      if (dx == 0 && dy == 0) {
        dx = x3 - x0;
        dy = y3 - y0;
      }
      tangentVector = _normalizeSlope(dx, dy);
    } else {
      final ax = x3 + (3 * (x1 - x2)) - x0;
      final ay = y3 + (3 * (y1 - y2)) - y0;
      final bx = 2 * (x2 - (2 * x1) + x0);
      final by = 2 * (y2 - (2 * y1) + y0);
      final cx = x1 - x0;
      final cy = y1 - y0;
      final tx = (ax * t + bx) * t + cx;
      final ty = (ay * t + by) * t + cy;
      tangentVector = _normalizeSlope(tx, ty);
    }
    return _SurfaceTangent(pos, tangentVector, t);
  }
}

// Evaluates A * t^3 + B * t^2 + Ct + D = 0 for cubic curve.
class _SkCubicCoefficients {
  _SkCubicCoefficients(double x0, double y0, double x1, double y1, double x2,
      double y2, double x3, double y3)
      : ax = x3 + (3 * (x1 - x2)) - x0,
        ay = y3 + (3 * (y1 - y2)) - y0,
        bx = 3 * (x2 - (2 * x1) + x0),
        by = 3 * (y2 - (2 * y1) + y0),
        cx = 3 * (x1 - x0),
        cy = 3 * (y1 - y0),
        dx = x0,
        dy = y0;

  final double ax, ay, bx, by, cx, cy, dx, dy;

  double evalX(double t) => (((ax * t + bx) * t) + cx) * t + dx;

  double evalY(double t) => (((ay * t + by) * t) + cy) * t + dy;
}

/// Chops quadratic curve at startT and stopT and writes result to buffer.
void _chopQuadBetweenT(
    List<double> points, double startT, double stopT, Float32List buffer) {
  assert(startT != 0 || stopT != 0);
  final p2y = points[5];
  final p0x = points[0];
  final p0y = points[1];
  final p1x = points[2];
  final p1y = points[3];
  final p2x = points[4];

  // If startT == 0 chop at end point and return curve.
  final chopStart = startT != 0;
  final t = chopStart ? startT : stopT;

  final ab1x = interpolate(p0x, p1x, t);
  final ab1y = interpolate(p0y, p1y, t);
  final bc1x = interpolate(p1x, p2x, t);
  final bc1y = interpolate(p1y, p2y, t);
  final abc1x = interpolate(ab1x, bc1x, t);
  final abc1y = interpolate(ab1y, bc1y, t);
  if (!chopStart) {
    // Return left side of curve.
    buffer[0] = p0x;
    buffer[1] = p0y;
    buffer[2] = ab1x;
    buffer[3] = ab1y;
    buffer[4] = abc1x;
    buffer[5] = abc1y;
    return;
  }
  if (stopT == 1) {
    // Return right side of curve.
    buffer[0] = abc1x;
    buffer[1] = abc1y;
    buffer[2] = bc1x;
    buffer[3] = bc1y;
    buffer[4] = p2x;
    buffer[5] = p2y;
    return;
  }
  // We chopped at startT, now the right hand side of curve is at
  // abc1x, abc1y, bc1x, bc1y, p2x, p2y
  final endT = (stopT - startT) / (1 - startT);
  final ab2x = interpolate(abc1x, bc1x, endT);
  final ab2y = interpolate(abc1y, bc1y, endT);
  final bc2x = interpolate(bc1x, p2x, endT);
  final bc2y = interpolate(bc1y, p2y, endT);
  final abc2x = interpolate(ab2x, bc2x, endT);
  final abc2y = interpolate(ab2y, bc2y, endT);

  buffer[0] = abc1x;
  buffer[1] = abc1y;
  buffer[2] = ab2x;
  buffer[3] = ab2y;
  buffer[4] = abc2x;
  buffer[5] = abc2y;
}
