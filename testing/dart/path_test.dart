// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data' show Float64List;
import 'dart:ui';

import 'package:test/test.dart';

Matcher moreOrLessEquals(double value, {double epsilon: 1e-10}) {
  return new _MoreOrLessEquals(value, epsilon);
}

class _MoreOrLessEquals extends Matcher {
  const _MoreOrLessEquals(this.value, this.epsilon);

  final double value;
  final double epsilon;

  @override
  bool matches(Object object, Map<dynamic, dynamic> matchState) {
    if (object is! double) return false;
    if (object == value) return true;
    final double test = object;
    return (test - value).abs() <= epsilon;
  }

  @override
  Description describe(Description description) =>
      description.add('$value (±$epsilon)');
}

void main() {
  test('path getBounds', () {
    final Rect r = new Rect.fromLTRB(1.0, 3.0, 5.0, 7.0);
    final Path p = new Path()..addRect(r);
    expect(p.getBounds(), equals(r));
    p.lineTo(20.0, 15.0);
    expect(p.getBounds(), equals(new Rect.fromLTRB(1.0, 3.0, 20.0, 15.0)));
  });

  test('path combine rect', () {
    final Rect c1 =
        new Rect.fromCircle(center: const Offset(10.0, 10.0), radius: 10.0);
    final Rect c2 =
        new Rect.fromCircle(center: const Offset(5.0, 5.0), radius: 10.0);
    final Rect c1UnionC2 = c1.expandToInclude(c2);
    final Rect c1IntersectC2 = c1.intersect(c2);
    final Path pathCircle1 = new Path()..addRect(c1);
    final Path pathCircle2 = new Path()..addRect(c2);

    final Path difference =
        Path.combine(PathOperation.difference, pathCircle1, pathCircle2);
    expect(difference.getBounds(), equals(c1));

    final Path reverseDifference =
        Path.combine(PathOperation.reverseDifference, pathCircle1, pathCircle2);
    expect(reverseDifference.getBounds(), equals(c2));

    final Path union =
        Path.combine(PathOperation.union, pathCircle1, pathCircle2);
    expect(union.getBounds(), equals(c1UnionC2));

    final Path intersect =
        Path.combine(PathOperation.intersect, pathCircle1, pathCircle2);
    expect(intersect.getBounds(), equals(c1IntersectC2));

    // the bounds on this will be the same as union - but would draw a missing inside piece.
    final Path xor = Path.combine(PathOperation.xor, pathCircle1, pathCircle2);
    expect(xor.getBounds(), equals(c1UnionC2));
  });

  test('path combine oval', () {
    final Rect c1 =
        new Rect.fromCircle(center: const Offset(10.0, 10.0), radius: 10.0);
    final Rect c2 =
        new Rect.fromCircle(center: const Offset(5.0, 5.0), radius: 10.0);
    final Rect c1UnionC2 = c1.expandToInclude(c2);
    final Rect c1IntersectC2 = c1.intersect(c2);
    final Path pathCircle1 = new Path()..addOval(c1);
    final Path pathCircle2 = new Path()..addOval(c2);

    final Path difference =
        Path.combine(PathOperation.difference, pathCircle1, pathCircle2);

    expect(difference.getBounds().top, moreOrLessEquals(0.88, epsilon: 0.01));

    final Path reverseDifference =
        Path.combine(PathOperation.reverseDifference, pathCircle1, pathCircle2);
    expect(reverseDifference.getBounds().right,
        moreOrLessEquals(14.11, epsilon: 0.01));

    final Path union =
        Path.combine(PathOperation.union, pathCircle1, pathCircle2);
    expect(union.getBounds(), equals(c1UnionC2));

    final Path intersect =
        Path.combine(PathOperation.intersect, pathCircle1, pathCircle2);
    expect(intersect.getBounds(), equals(c1IntersectC2));

    // the bounds on this will be the same as union - but would draw a missing inside piece.
    final Path xor = Path.combine(PathOperation.xor, pathCircle1, pathCircle2);
    expect(xor.getBounds(), equals(c1UnionC2));
  });

  test('path clone', () {
    final Path p1 = new Path()..lineTo(20.0, 20.0);
    final Path p2 = new Path.from(p1);

    expect(p1.getBounds(), equals(p2.getBounds()));

    p1.lineTo(10.0, 30.0);
    expect(p1.getBounds().bottom, equals(p2.getBounds().bottom + 10));
  });

  test('transformation tests', () {
    final Rect bounds = new Rect.fromLTRB(0.0, 0.0, 10.0, 10.0);
    final Path p = new Path()..addRect(bounds);
    final Float64List scaleMatrix = new Float64List.fromList([
      2.5, 0.0, 0.0, 0.0, // first col
      0.0, 0.5, 0.0, 0.0, // second col
      0.0, 0.0, 1.0, 0.0, // third col
      0.0, 0.0, 0.0, 1.0, // fourth col
    ]);

    expect(p.getBounds(), equals(bounds));
    final Path pTransformed = p.transform(scaleMatrix);

    expect(pTransformed.getBounds(),
        equals(new Rect.fromLTRB(0.0, 0.0, 10 * 2.5, 10 * 0.5)));

    final Path p2 = new Path()..lineTo(10.0, 10.0);

    p.addPath(p2, const Offset(10.0, 10.0));
    expect(p.getBounds(), equals(new Rect.fromLTRB(0.0, 0.0, 20.0, 20.0)));

    p.addPath(p2, const Offset(20.0, 20.0), matrix4: scaleMatrix);
    expect(p.getBounds(),
        equals(new Rect.fromLTRB(0.0, 0.0, 20 + (10 * 2.5), 20 + (10 * .5))));

    p.extendWithPath(p2, const Offset(0.0, 0.0));
    expect(p.getBounds(), equals(new Rect.fromLTRB(0.0, 0.0, 45.0, 25.0)));

    p.extendWithPath(p2, const Offset(45.0, 25.0), matrix4: scaleMatrix);
    expect(p.getBounds(), equals(new Rect.fromLTRB(0.0, 0.0, 70.0, 30.0)));
  });

  test('path metrics tests', () {
    final Path simpleHorizontalLine = new Path()..lineTo(10.0, 0.0);

    // basic tests on horizontal line
    final PathMetrics simpleHorizontalMetrics =
        simpleHorizontalLine.computeMetrics();
    expect(simpleHorizontalMetrics.iterator.current, isNull);
    expect(simpleHorizontalMetrics.iterator.moveNext(), isTrue);
    expect(simpleHorizontalMetrics.iterator.current, isNotNull);
    expect(simpleHorizontalMetrics.iterator.current.length, equals(10.0));
    expect(simpleHorizontalMetrics.iterator.current.isClosed, isFalse);
    final Path simpleExtract =
        simpleHorizontalMetrics.iterator.current.extractPath(1.0, 9.0);
    expect(simpleExtract.getBounds(),
        equals(new Rect.fromLTRB(1.0, 0.0, 9.0, 0.0)));
    final Tangent posTan =
        simpleHorizontalMetrics.iterator.current.getTangentForOffset(1.0);
    expect(posTan, isNotNull);
    expect(posTan.position, equals(const Offset(1.0, 0.0)));
    expect(posTan.angle, equals(0.0));

    expect(simpleHorizontalMetrics.iterator.moveNext(), isFalse);
    expect(simpleHorizontalMetrics.iterator.current, isNull);

    // test with forceClosed
    final PathMetrics simpleMetricsClosed =
        simpleHorizontalLine.computeMetrics(forceClosed: true);
    expect(simpleMetricsClosed.iterator.current, isNull);
    expect(simpleMetricsClosed.iterator.moveNext(), isTrue);
    expect(simpleMetricsClosed.iterator.current, isNotNull);
    expect(simpleMetricsClosed.iterator.current.length,
        equals(20.0)); // because we forced close
    expect(simpleMetricsClosed.iterator.current.isClosed, isTrue);
    final Path simpleExtract2 =
        simpleMetricsClosed.iterator.current.extractPath(1.0, 9.0);
    expect(simpleExtract2.getBounds(),
        equals(new Rect.fromLTRB(1.0, 0.0, 9.0, 0.0)));
    expect(simpleMetricsClosed.iterator.moveNext(), isFalse);

    // test getTangentForOffset with vertical line
    final Path simpleVerticalLine = new Path()..lineTo(0.0, 10.0);
    final PathMetrics simpleMetricsVertical =
        simpleVerticalLine.computeMetrics()..iterator.moveNext();
    final Tangent posTanVertical =
        simpleMetricsVertical.iterator.current.getTangentForOffset(5.0);
    expect(posTanVertical.position, equals(const Offset(0.0, 5.0)));
    expect(posTanVertical.angle,
        moreOrLessEquals(1.5708, epsilon: .0001)); // 90 degrees

    // test getTangentForOffset with diagonal line
    final Path simpleDiagonalLine = new Path()..lineTo(10.0, 10.0);
    final PathMetrics simpleMetricsDiagonal =
        simpleDiagonalLine.computeMetrics()..iterator.moveNext();
    final double midPoint = simpleMetricsDiagonal.iterator.current.length / 2;
    final Tangent posTanDiagonal =
        simpleMetricsDiagonal.iterator.current.getTangentForOffset(midPoint);
    expect(posTanDiagonal.position, equals(new Offset(5.0, 5.0)));
    expect(posTanDiagonal.angle,
        moreOrLessEquals(0.7853981633974483, epsilon: .00001)); // ~45 degrees

    // test a multi-contour path
    final Path multiContour = new Path()
      ..lineTo(0.0, 10.0)
      ..moveTo(10.0, 10.0)
      ..lineTo(10.0, 15.0);

    final PathMetrics multiContourMetric = multiContour.computeMetrics();
    expect(multiContourMetric.iterator.current, isNull);
    expect(multiContourMetric.iterator.moveNext(), isTrue);
    expect(multiContourMetric.iterator.current, isNotNull);
    expect(multiContourMetric.iterator.current.length, equals(10.0));
    expect(multiContourMetric.iterator.moveNext(), isTrue);
    expect(multiContourMetric.iterator.current, isNotNull);
    expect(multiContourMetric.iterator.current.length, equals(5.0));
    expect(multiContourMetric.iterator.moveNext(), isFalse);
    expect(multiContourMetric.iterator.current, isNull);
  });
}
