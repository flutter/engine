// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:math' as math;
import 'dart:typed_data';

import 'path_utils.dart';

/// Chops cubic at Y extrema points and writes result to [dest].
///
/// [points] and [dest] are allowed to share underlying storage as long.
int chopCubicAtYExtrema(Float32List points, Float32List dest) {
  final y0 = points[1];
  final y1 = points[3];
  final y2 = points[5];
  final y3 = points[7];
  final quadRoots = _findCubicExtrema(y0, y1, y2, y3);
  final roots = quadRoots.roots;
  if (roots.isEmpty) {
    // No roots, just use input cubic.
    return 0;
  }
  _chopCubicAt(roots, points, dest);
  final rootCount = roots.length;
  if (rootCount > 0) {
    // Cleanup to ensure Y extrema are flat.
    dest[5] = dest[9] = dest[7];
    if (rootCount == 2) {
      dest[11] = dest[15] = dest[13];
    }
  }
  return rootCount;
}

QuadRoots _findCubicExtrema(double a, double b, double c, double d) {
  // A,B,C scaled by 1/3 to simplify
  final A = d - a + 3 * (b - c);
  final B = 2 * (a - b - b + c);
  final C = b - a;
  return QuadRoots()..findRoots(A, B, C);
}

/// Subdivides cubic curve for a list of t values.
void _chopCubicAt(
    List<double> tValues, Float32List points, Float32List outPts) {
  assert(() {
    for (var i = 0; i < tValues.length - 1; i++) {
      final tValue = tValues[i];
      assert(tValue > 0 && tValue < 1,
          'Not expecting to chop curve at start, end points');
    }
    for (var i = 0; i < tValues.length - 1; i++) {
      final tValue = tValues[i];
      final nextTValue = tValues[i + 1];
      assert(
          nextTValue > tValue, 'Expecting t value to monotonically increase');
    }
    return true;
  }());
  final rootCount = tValues.length;
  if (0 == rootCount) {
    for (var i = 0; i < 8; i++) {
      outPts[i] = points[i];
    }
  } else {
    // Chop curve at t value and loop through right side of curve
    // while normalizing t value based on prior t.
    double? t = tValues[0];
    var bufferPos = 0;
    for (var i = 0; i < rootCount; i++) {
      _chopCubicAtT(points, bufferPos, outPts, bufferPos, t!);
      if (i == rootCount - 1) {
        break;
      }
      bufferPos += 6;

      // watch out in case the renormalized t isn't in range
      if ((t = validUnitDivide(
              tValues[i + 1] - tValues[i], 1.0 - tValues[i])) ==
          null) {
        // Can't renormalize last point, just create a degenerate cubic.
        outPts[bufferPos + 4] = outPts[bufferPos + 5] =
            outPts[bufferPos + 6] = points[bufferPos + 3];
        break;
      }
    }
  }
}

/// Subdivides cubic curve at [t] and writes to [outPts] at position [outIndex].
///
/// The cubic points are read from [points] at [bufferPos] offset.
void _chopCubicAtT(Float32List points, int bufferPos, Float32List outPts,
    int outIndex, double t) {
  assert(t > 0 && t < 1);
  final p3y = points[bufferPos + 7];
  final p0x = points[bufferPos + 0];
  final p0y = points[bufferPos + 1];
  final p1x = points[bufferPos + 2];
  final p1y = points[bufferPos + 3];
  final p2x = points[bufferPos + 4];
  final p2y = points[bufferPos + 5];
  final p3x = points[bufferPos + 6];
  // If startT == 0 chop at end point and return curve.
  final ab1x = interpolate(p0x, p1x, t);
  final ab1y = interpolate(p0y, p1y, t);
  final bc1x = interpolate(p1x, p2x, t);
  final bc1y = interpolate(p1y, p2y, t);
  final cd1x = interpolate(p2x, p3x, t);
  final cd1y = interpolate(p2y, p3y, t);
  final abc1x = interpolate(ab1x, bc1x, t);
  final abc1y = interpolate(ab1y, bc1y, t);
  final bcd1x = interpolate(bc1x, cd1x, t);
  final bcd1y = interpolate(bc1y, cd1y, t);
  final abcd1x = interpolate(abc1x, bcd1x, t);
  final abcd1y = interpolate(abc1y, bcd1y, t);

  // Return left side of curve.
  outPts[outIndex++] = p0x;
  outPts[outIndex++] = p0y;
  outPts[outIndex++] = ab1x;
  outPts[outIndex++] = ab1y;
  outPts[outIndex++] = abc1x;
  outPts[outIndex++] = abc1y;
  outPts[outIndex++] = abcd1x;
  outPts[outIndex++] = abcd1y;
  // Return right side of curve.
  outPts[outIndex++] = bcd1x;
  outPts[outIndex++] = bcd1y;
  outPts[outIndex++] = cd1x;
  outPts[outIndex++] = cd1y;
  outPts[outIndex++] = p3x;
  outPts[outIndex++] = p3y;
}

// Returns t at Y for cubic curve. null if y is out of range.
//
// Options are Newton Raphson (quadratic convergence with typically
// 3 iterations or bisection with 16 iterations.
double? chopMonoAtY(Float32List buffer, int bufferStartPos, double y) {
  // Translate curve points relative to y.
  final ycrv0 = buffer[1 + bufferStartPos] - y;
  final ycrv1 = buffer[3 + bufferStartPos] - y;
  final ycrv2 = buffer[5 + bufferStartPos] - y;
  final ycrv3 = buffer[7 + bufferStartPos] - y;
  // Positive and negative function parameters.
  double tNeg, tPos;
  // Set initial t points to converge from.
  if (ycrv0 < 0) {
    if (ycrv3 < 0) {
      // Start and end points out of range.
      return null;
    }
    tNeg = 0;
    tPos = 1.0;
  } else if (ycrv0 > 0) {
    tNeg = 1.0;
    tPos = 0;
  } else {
    // Start is at y.
    return 0.0;
  }

  // Bisection / linear convergance.
  const tolerance = 1.0 / 65536;
  do {
    final tMid = (tPos + tNeg) / 2.0;
    final y01 = ycrv0 + (ycrv1 - ycrv0) * tMid;
    final y12 = ycrv1 + (ycrv2 - ycrv1) * tMid;
    final y23 = ycrv2 + (ycrv3 - ycrv2) * tMid;
    final y012 = y01 + (y12 - y01) * tMid;
    final y123 = y12 + (y23 - y12) * tMid;
    final y0123 = y012 + (y123 - y012) * tMid;
    if (y0123 == 0) {
      return tMid;
    }
    if (y0123 < 0) {
      tNeg = tMid;
    } else {
      tPos = tMid;
    }
  } while ((tPos - tNeg).abs() > tolerance);
  return (tNeg + tPos) / 2;
}

double evalCubicPts(double c0, double c1, double c2, double c3, double t) {
  final A = c3 + 3 * (c1 - c2) - c0;
  final B = 3 * (c2 - c1 - c1 + c0);
  final C = 3 * (c1 - c0);
  final D = c0;
  return polyEval4(A, B, C, D, t);
}

// Reusable class to compute bounds without object allocation.
class CubicBounds {
  double minX = 0.0;
  double maxX = 0.0;
  double minY = 0.0;
  double maxY = 0.0;

  /// Sets resulting bounds as [minX], [minY], [maxX], [maxY].
  ///
  /// The cubic is defined by 4 points (8 floats) in [points].
  void calculateBounds(Float32List points, int pointIndex) {
    final startX = points[pointIndex++];
    final startY = points[pointIndex++];
    final cpX1 = points[pointIndex++];
    final cpY1 = points[pointIndex++];
    final cpX2 = points[pointIndex++];
    final cpY2 = points[pointIndex++];
    final endX = points[pointIndex++];
    final endY = points[pointIndex++];
    // Bounding box is defined by all points on the curve where
    // monotonicity changes.
    minX = math.min(startX, endX);
    minY = math.min(startY, endY);
    maxX = math.max(startX, endX);
    maxY = math.max(startY, endY);

    double extremaX;
    double extremaY;
    double a, b, c;

    // Check for simple case of strong ordering before calculating
    // extrema
    if (!(((startX < cpX1) && (cpX1 < cpX2) && (cpX2 < endX)) ||
        ((startX > cpX1) && (cpX1 > cpX2) && (cpX2 > endX)))) {
      // The extrema point is dx/dt B(t) = 0
      // The derivative of B(t) for cubic bezier is a quadratic equation
      // with multiple roots
      // B'(t) = a*t*t + b*t + c*t
      a = -startX + (3 * (cpX1 - cpX2)) + endX;
      b = 2 * (startX - (2 * cpX1) + cpX2);
      c = -startX + cpX1;

      // Now find roots for quadratic equation with known coefficients
      // a,b,c
      // The roots are (-b+-sqrt(b*b-4*a*c)) / 2a
      num s = (b * b) - (4 * a * c);
      // If s is negative, we have no real roots
      if ((s >= 0.0) && (a.abs() > SPath.scalarNearlyZero)) {
        if (s == 0.0) {
          // we have only 1 root
          final t = -b / (2 * a);
          final tprime = 1.0 - t;
          if ((t >= 0.0) && (t <= 1.0)) {
            extremaX = ((tprime * tprime * tprime) * startX) +
                ((3 * tprime * tprime * t) * cpX1) +
                ((3 * tprime * t * t) * cpX2) +
                (t * t * t * endX);
            minX = math.min(extremaX, minX);
            maxX = math.max(extremaX, maxX);
          }
        } else {
          // we have 2 roots
          s = math.sqrt(s);
          var t = (-b - s) / (2 * a);
          var tprime = 1.0 - t;
          if ((t >= 0.0) && (t <= 1.0)) {
            extremaX = ((tprime * tprime * tprime) * startX) +
                ((3 * tprime * tprime * t) * cpX1) +
                ((3 * tprime * t * t) * cpX2) +
                (t * t * t * endX);
            minX = math.min(extremaX, minX);
            maxX = math.max(extremaX, maxX);
          }
          // check 2nd root
          t = (-b + s) / (2 * a);
          tprime = 1.0 - t;
          if ((t >= 0.0) && (t <= 1.0)) {
            extremaX = ((tprime * tprime * tprime) * startX) +
                ((3 * tprime * tprime * t) * cpX1) +
                ((3 * tprime * t * t) * cpX2) +
                (t * t * t * endX);

            minX = math.min(extremaX, minX);
            maxX = math.max(extremaX, maxX);
          }
        }
      }
    }

    // Now calc extremes for dy/dt = 0 just like above
    if (!(((startY < cpY1) && (cpY1 < cpY2) && (cpY2 < endY)) ||
        ((startY > cpY1) && (cpY1 > cpY2) && (cpY2 > endY)))) {
      // The extrema point is dy/dt B(t) = 0
      // The derivative of B(t) for cubic bezier is a quadratic equation
      // with multiple roots
      // B'(t) = a*t*t + b*t + c*t
      a = -startY + (3 * (cpY1 - cpY2)) + endY;
      b = 2 * (startY - (2 * cpY1) + cpY2);
      c = -startY + cpY1;

      // Now find roots for quadratic equation with known coefficients
      // a,b,c
      // The roots are (-b+-sqrt(b*b-4*a*c)) / 2a
      var s = (b * b) - (4 * a * c);
      // If s is negative, we have no real roots
      if ((s >= 0.0) && (a.abs() > SPath.scalarNearlyZero)) {
        if (s == 0.0) {
          // we have only 1 root
          final t = -b / (2 * a);
          final tprime = 1.0 - t;
          if ((t >= 0.0) && (t <= 1.0)) {
            extremaY = ((tprime * tprime * tprime) * startY) +
                ((3 * tprime * tprime * t) * cpY1) +
                ((3 * tprime * t * t) * cpY2) +
                (t * t * t * endY);
            minY = math.min(extremaY, minY);
            maxY = math.max(extremaY, maxY);
          }
        } else {
          // we have 2 roots
          s = math.sqrt(s);
          final t = (-b - s) / (2 * a);
          final tprime = 1.0 - t;
          if ((t >= 0.0) && (t <= 1.0)) {
            extremaY = ((tprime * tprime * tprime) * startY) +
                ((3 * tprime * tprime * t) * cpY1) +
                ((3 * tprime * t * t) * cpY2) +
                (t * t * t * endY);
            minY = math.min(extremaY, minY);
            maxY = math.max(extremaY, maxY);
          }
          // check 2nd root
          final t2 = (-b + s) / (2 * a);
          final tprime2 = 1.0 - t2;
          if ((t2 >= 0.0) && (t2 <= 1.0)) {
            extremaY = ((tprime2 * tprime2 * tprime2) * startY) +
                ((3 * tprime2 * tprime2 * t2) * cpY1) +
                ((3 * tprime2 * t2 * t2) * cpY2) +
                (t2 * t2 * t2 * endY);
            minY = math.min(extremaY, minY);
            maxY = math.max(extremaY, maxY);
          }
        }
      }
    }
  }
}

/// Chops cubic spline at startT and stopT, writes result to buffer.
void chopCubicBetweenT(
    List<double> points, double startT, double stopT, Float32List buffer) {
  assert(startT != 0 || stopT != 0);
  final p3y = points[7];
  final p0x = points[0];
  final p0y = points[1];
  final p1x = points[2];
  final p1y = points[3];
  final p2x = points[4];
  final p2y = points[5];
  final p3x = points[6];
  // If startT == 0 chop at end point and return curve.
  final chopStart = startT != 0;
  final t = chopStart ? startT : stopT;

  final ab1x = interpolate(p0x, p1x, t);
  final ab1y = interpolate(p0y, p1y, t);
  final bc1x = interpolate(p1x, p2x, t);
  final bc1y = interpolate(p1y, p2y, t);
  final cd1x = interpolate(p2x, p3x, t);
  final cd1y = interpolate(p2y, p3y, t);
  final abc1x = interpolate(ab1x, bc1x, t);
  final abc1y = interpolate(ab1y, bc1y, t);
  final bcd1x = interpolate(bc1x, cd1x, t);
  final bcd1y = interpolate(bc1y, cd1y, t);
  final abcd1x = interpolate(abc1x, bcd1x, t);
  final abcd1y = interpolate(abc1y, bcd1y, t);
  if (!chopStart) {
    // Return left side of curve.
    buffer[0] = p0x;
    buffer[1] = p0y;
    buffer[2] = ab1x;
    buffer[3] = ab1y;
    buffer[4] = abc1x;
    buffer[5] = abc1y;
    buffer[6] = abcd1x;
    buffer[7] = abcd1y;
    return;
  }
  if (stopT == 1) {
    // Return right side of curve.
    buffer[0] = abcd1x;
    buffer[1] = abcd1y;
    buffer[2] = bcd1x;
    buffer[3] = bcd1y;
    buffer[4] = cd1x;
    buffer[5] = cd1y;
    buffer[6] = p3x;
    buffer[7] = p3y;
    return;
  }
  // We chopped at startT, now the right hand side of curve is at
  // abcd1, bcd1, cd1, p3x, p3y. Chop this part using endT;
  final endT = (stopT - startT) / (1 - startT);
  final ab2x = interpolate(abcd1x, bcd1x, endT);
  final ab2y = interpolate(abcd1y, bcd1y, endT);
  final bc2x = interpolate(bcd1x, cd1x, endT);
  final bc2y = interpolate(bcd1y, cd1y, endT);
  final cd2x = interpolate(cd1x, p3x, endT);
  final cd2y = interpolate(cd1y, p3y, endT);
  final abc2x = interpolate(ab2x, bc2x, endT);
  final abc2y = interpolate(ab2y, bc2y, endT);
  final bcd2x = interpolate(bc2x, cd2x, endT);
  final bcd2y = interpolate(bc2y, cd2y, endT);
  final abcd2x = interpolate(abc2x, bcd2x, endT);
  final abcd2y = interpolate(abc2y, bcd2y, endT);
  buffer[0] = abcd1x;
  buffer[1] = abcd1y;
  buffer[2] = ab2x;
  buffer[3] = ab2y;
  buffer[4] = abc2x;
  buffer[5] = abc2y;
  buffer[6] = abcd2x;
  buffer[7] = abcd2y;
}
