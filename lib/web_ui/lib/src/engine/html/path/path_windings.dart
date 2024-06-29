// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:math' as math;
import 'dart:typed_data';

import 'conic.dart';
import 'cubic.dart';
import 'path_iterator.dart';
import 'path_ref.dart';
import 'path_utils.dart';

/// Computes winding number and onCurveCount for a path and point.
class PathWinding {
  PathWinding(this.pathRef, this.x, this.y) {
    _walkPath();
  }

  final PathRef pathRef;
  final double x;
  final double y;
  int _w = 0;
  int _onCurveCount = 0;

  int get w => _w;

  int get onCurveCount => _onCurveCount;

  /// Buffer used for max(iterator result, chopped 3 cubics).
  final Float32List _buffer = Float32List(8 + 10);

  /// Iterates through path and computes winding.
  void _walkPath() {
    final iter = PathIterator(pathRef, true);
    int verb;
    while ((verb = iter.next(_buffer)) != SPath.kDoneVerb) {
      switch (verb) {
        case SPath.kMoveVerb:
        case SPath.kCloseVerb:
          break;
        case SPath.kLineVerb:
          _computeLineWinding();
        case SPath.kQuadVerb:
          _computeQuadWinding();
        case SPath.kConicVerb:
          _computeConicWinding(pathRef.conicWeights![iter.conicWeightIndex]);
        case SPath.kCubicVerb:
          _computeCubicWinding();
      }
    }
  }

  void _computeLineWinding() {
    final x0 = _buffer[0];
    final startY = _buffer[1];
    var y0 = startY;
    final x1 = _buffer[2];
    final endY = _buffer[3];
    var y1 = endY;
    final dy = y1 - y0;
    var dir = 1;
    // Swap so that y0 <= y1 holds.
    if (y0 > y1) {
      final temp = y0;
      y0 = y1;
      y1 = temp;
      dir = -1;
    }
    // If point is outside top/bottom bounds, winding is 0.
    if (y < y0 || y > y1) {
      return;
    }
    if (_checkOnCurve(x, y, x0, startY, x1, endY)) {
      _onCurveCount++;
      return;
    }
    if (y == y1) {
      return;
    }
    // c = ax*by − ay*bx where a is the line and b is line formed from start
    // to the given point(x,y).
    final crossProduct = (x1 - x0) * (y - startY) - dy * (x - x0);
    if (crossProduct == 0) {
      // zero cross means the point is on the line, and since the case where
      // y of the query point is at the end point is handled above, we can be
      // sure that we're on the line (excluding the end point) here.
      if (x != x1 || y != endY) {
        _onCurveCount++;
      }
      dir = 0;
    } else if (SPath.scalarSignedAsInt(crossProduct) == dir) {
      // Direction of cross product and line the same.
      dir = 0;
    }
    _w += dir;
  }

  // Check if point starts the line, handle special case for horizontal lines
  // where and point except the end point is considered on curve.
  static bool _checkOnCurve(double x, double y, double startX, double startY,
      double endX, double endY) {
    if (startY == endY) {
      // Horizontal line.
      return SPath.between(startX, x, endX) && x != endX;
    } else {
      return x == startX && y == startY;
    }
  }

  void _computeQuadWinding() {
    // Check if we need to chop quadratic at extrema to compute 2 separate
    // windings.
    var n = 0;
    if (!_isQuadMonotonic(_buffer)) {
      n = _chopQuadAtExtrema(_buffer);
    }
    var winding = _computeMonoQuadWinding(
        _buffer[0], _buffer[1], _buffer[2], _buffer[3], _buffer[4], _buffer[5]);
    if (n > 0) {
      winding += _computeMonoQuadWinding(_buffer[4], _buffer[5], _buffer[6],
          _buffer[7], _buffer[8], _buffer[9]);
    }
    _w += winding;
  }

  int _computeMonoQuadWinding(
      double x0, double y0, double x1, double y1, double x2, double y2) {
    var dir = 1;
    final startY = y0;
    final endY = y2;
    if (y0 > y2) {
      final temp = y0;
      y0 = y2;
      y2 = temp;
      dir = -1;
    }
    if (y < y0 || y > y2) {
      return 0;
    }
    if (_checkOnCurve(x, y, x0, startY, x2, endY)) {
      _onCurveCount++;
      return 0;
    }
    if (y == y2) {
      return 0;
    }

    final quadRoots = QuadRoots();
    final n = quadRoots.findRoots(
        startY - 2 * y1 + endY, 2 * (y1 - startY), startY - y);
    assert(n <= 1);
    double xt;
    if (0 == n) {
      // zero roots are returned only when y0 == y
      xt = dir == 1 ? x0 : x2;
    } else {
      final t = quadRoots.root0!;
      final C = x0;
      final A = x2 - 2 * x1 + C;
      final B = 2 * (x1 - C);
      xt = polyEval(A, B, C, t);
    }
    if (SPath.nearlyEqual(xt, x)) {
      if (x != x2 || y != endY) {
        // don't test end points; they're start points
        _onCurveCount += 1;
        return 0;
      }
    }
    return xt < x ? dir : 0;
  }

  /// Chops a non-monotonic quadratic curve, returns subdivisions and writes
  /// result into [buffer].
  static int _chopQuadAtExtrema(Float32List buffer) {
    final x0 = buffer[0];
    final y0 = buffer[1];
    final x1 = buffer[2];
    final y1 = buffer[3];
    final x2 = buffer[4];
    final y2 = buffer[5];
    final tValueAtExtrema = validUnitDivide(y0 - y1, y0 - y1 - y1 + y2);
    if (tValueAtExtrema != null) {
      // Chop quad at t value by interpolating along p0-p1 and p1-p2.
      final p01x = x0 + (tValueAtExtrema * (x1 - x0));
      final p01y = y0 + (tValueAtExtrema * (y1 - y0));
      final p12x = x1 + (tValueAtExtrema * (x2 - x1));
      final p12y = y1 + (tValueAtExtrema * (y2 - y1));
      final cx = p01x + (tValueAtExtrema * (p12x - p01x));
      final cy = p01y + (tValueAtExtrema * (p12y - p01y));
      buffer[2] = p01x;
      buffer[3] = p01y;
      buffer[4] = cx;
      buffer[5] = cy;
      buffer[6] = p12x;
      buffer[7] = p12y;
      buffer[8] = x2;
      buffer[9] = y2;
      return 1;
    }
    // if we get here, we need to force output to be monotonic, even though
    // we couldn't compute a unit divide value (probably underflow).
    buffer[3] = (y0 - y1).abs() < (y1 - y2).abs() ? y0 : y2;
    return 0;
  }

  static bool _isQuadMonotonic(Float32List quad) {
    final y0 = quad[1];
    final y1 = quad[3];
    final y2 = quad[5];
    if (y0 == y1) {
      return true;
    }
    if (y0 < y1) {
      return y1 <= y2;
    } else {
      return y1 >= y2;
    }
  }

  void _computeConicWinding(double weight) {
    final conic = Conic(_buffer[0], _buffer[1], _buffer[2], _buffer[3],
        _buffer[4], _buffer[5], weight);
    // If the data points are very large, the conic may not be monotonic but may also
    // fail to chop. Then, the chopper does not split the original conic in two.
    final isMono = _isQuadMonotonic(_buffer);
    final conics = <Conic>[];
    conic.chopAtYExtrema(conics);
    _computeMonoConicWinding(conics[0]);
    if (!isMono && conics.length == 2) {
      _computeMonoConicWinding(conics[1]);
    }
  }

  void _computeMonoConicWinding(Conic conic) {
    var y0 = conic.p0y;
    var y2 = conic.p2y;
    var dir = 1;
    if (y0 > y2) {
      final swap = y0;
      y0 = y2;
      y2 = swap;
      dir = -1;
    }
    if (y < y0 || y > y2) {
      return;
    }
    if (_checkOnCurve(x, y, conic.p0x, conic.p0y, conic.p2x, conic.p2y)) {
      _onCurveCount += 1;
      return;
    }
    if (y == y2) {
      return;
    }

    var A = conic.p2y;
    var B = conic.p1y * conic.fW - y * conic.fW + y;
    var C = conic.p0y;
    // A = a + c - 2*(b*w - yCept*w + yCept)
    A += C - 2 * B;
    // B = b*w - w * yCept + yCept - a
    B -= C;
    C -= y;
    final quadRoots = QuadRoots();
    final n = quadRoots.findRoots(A, 2 * B, C);
    assert(n <= 1);
    double xt;
    if (0 == n) {
      // zero roots are returned only when y0 == y
      // Need [0] if dir == 1
      // and  [2] if dir == -1
      xt = dir == 1 ? conic.p0x : conic.p2x;
    } else {
      final root = quadRoots.root0!;
      xt =
          Conic.evalNumerator(conic.p0x, conic.p1x, conic.p2x, conic.fW, root) /
              Conic.evalDenominator(conic.fW, root);
    }
    if (SPath.nearlyEqual(xt, x)) {
      if (x != conic.p2x || y != conic.p2y) {
        // don't test end points; they're start points
        _onCurveCount += 1;
        return;
      }
    }
    _w += xt < x ? dir : 0;
  }

  void _computeCubicWinding() {
    final n = chopCubicAtYExtrema(_buffer, _buffer);
    for (var i = 0; i <= n; ++i) {
      _windingMonoCubic(i * 3 * 2);
    }
  }

  void _windingMonoCubic(int bufferIndex) {
    final bufferStartPos = bufferIndex;
    final px0 = _buffer[bufferIndex++];
    final py0 = _buffer[bufferIndex++];
    final px1 = _buffer[bufferIndex++];
    bufferIndex++;
    final px2 = _buffer[bufferIndex++];
    bufferIndex++;
    final px3 = _buffer[bufferIndex++];
    final py3 = _buffer[bufferIndex++];

    var y0 = py0;
    var y3 = py3;

    var dir = 1;
    if (y0 > y3) {
      final swap = y0;
      y0 = y3;
      y3 = swap;
      dir = -1;
    }
    if (y < y0 || y > y3) {
      return;
    }
    if (_checkOnCurve(x, y, px0, py0, px3, py3)) {
      _onCurveCount += 1;
      return;
    }
    if (y == y3) {
      return;
    }

    // Quickly reject or accept
    final double min = math.min(px0, math.min(px1, math.min(px2, px3)));
    final double max = math.max(px0, math.max(px1, math.max(px2, px3)));
    if (x < min) {
      return;
    }
    if (x > max) {
      _w += dir;
      return;
    }
    // Compute the actual x(t) value.
    final t = chopMonoAtY(_buffer, bufferStartPos, y);
    if (t == null) {
      return;
    }
    final xt = evalCubicPts(px0, px1, px2, px3, t);
    if (SPath.nearlyEqual(xt, x)) {
      if (x != px3 || y != py3) {
        // don't test end points; they're start points
        _onCurveCount += 1;
        return;
      }
    }
    _w += xt < x ? dir : 0;
  }
}
