// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Stub implementation. See docs in `../ui/`.
abstract class OffsetBase {
  /// Stub implementation. See docs in `../ui/`.
  const OffsetBase(this._dx, this._dy);

  final double _dx;
  final double _dy;

  /// Stub implementation. See docs in `../ui/`.
  bool get isInfinite => _dx >= double.infinity || _dy >= double.infinity;

  /// Stub implementation. See docs in `../ui/`.
  bool get isFinite => _dx.isFinite && _dy.isFinite;

  /// Stub implementation. See docs in `../ui/`.
  bool operator <(OffsetBase other) => _dx < other._dx && _dy < other._dy;

  /// Stub implementation. See docs in `../ui/`.
  bool operator <=(OffsetBase other) => _dx <= other._dx && _dy <= other._dy;

  /// Stub implementation. See docs in `../ui/`.
  bool operator >(OffsetBase other) => _dx > other._dx && _dy > other._dy;

  /// Stub implementation. See docs in `../ui/`.
  bool operator >=(OffsetBase other) => _dx >= other._dx && _dy >= other._dy;

  /// Stub implementation. See docs in `../ui/`.
  @override
  bool operator ==(dynamic other) {
    if (other is! OffsetBase)
      return false;
    final OffsetBase typedOther = other;
    return _dx == typedOther._dx &&
           _dy == typedOther._dy;
  }

  @override
  int get hashCode => hashValues(_dx, _dy);

  @override
  String toString() => '$runtimeType(${_dx?.toStringAsFixed(1)}, ${_dy?.toStringAsFixed(1)})';
}

/// Stub implementation. See docs in `../ui/`.
class Offset extends OffsetBase {
  /// Stub implementation. See docs in `../ui/`.
  const Offset(double dx, double dy) : super(dx, dy);

  /// Stub implementation. See docs in `../ui/`.
  factory Offset.fromDirection(double direction, [ double distance = 1.0 ]) {
    return new Offset(distance * math.cos(direction), distance * math.sin(direction));
  }

  /// Stub implementation. See docs in `../ui/`.
  double get dx => _dx;

  /// Stub implementation. See docs in `../ui/`.
  double get dy => _dy;

  /// Stub implementation. See docs in `../ui/`.
  double get distance => math.sqrt(dx * dx + dy * dy);

  /// Stub implementation. See docs in `../ui/`.
  double get distanceSquared => dx * dx + dy * dy;

  /// Stub implementation. See docs in `../ui/`.
  double get direction => math.atan2(dy, dx);

  /// Stub implementation. See docs in `../ui/`.
  static const Offset zero = const Offset(0.0, 0.0);

  /// Stub implementation. See docs in `../ui/`.
  // This is included for completeness, because [Size.infinite] exists.
  static const Offset infinite = const Offset(double.infinity, double.infinity);

  /// Stub implementation. See docs in `../ui/`.
  Offset scale(double scaleX, double scaleY) => new Offset(dx * scaleX, dy * scaleY);

  /// Stub implementation. See docs in `../ui/`.
  Offset translate(double translateX, double translateY) => new Offset(dx + translateX, dy + translateY);

  /// Stub implementation. See docs in `../ui/`.
  Offset operator -() => new Offset(-dx, -dy);

  /// Stub implementation. See docs in `../ui/`.
  Offset operator -(Offset other) => new Offset(dx - other.dx, dy - other.dy);

  /// Stub implementation. See docs in `../ui/`.
  Offset operator +(Offset other) => new Offset(dx + other.dx, dy + other.dy);

  /// Stub implementation. See docs in `../ui/`.
  Offset operator *(double operand) => new Offset(dx * operand, dy * operand);

  /// Stub implementation. See docs in `../ui/`.
  Offset operator /(double operand) => new Offset(dx / operand, dy / operand);

  /// Stub implementation. See docs in `../ui/`.
  Offset operator ~/(double operand) => new Offset((dx ~/ operand).toDouble(), (dy ~/ operand).toDouble());

  /// Stub implementation. See docs in `../ui/`.
  Offset operator %(double operand) => new Offset(dx % operand, dy % operand);

  /// Stub implementation. See docs in `../ui/`.
  Rect operator &(Size other) => new Rect.fromLTWH(dx, dy, other.width, other.height);

  /// Stub implementation. See docs in `../ui/`.
  static Offset lerp(Offset a, Offset b, double t) {
    assert(t != null);
    if (a == null && b == null)
      return null;
    if (a == null)
      return b * t;
    if (b == null)
      return a * (1.0 - t);
    return new Offset(lerpDouble(a.dx, b.dx, t), lerpDouble(a.dy, b.dy, t));
  }

  /// Stub implementation. See docs in `../ui/`.
  @override
  bool operator ==(dynamic other) {
    if (other is! Offset)
      return false;
    final Offset typedOther = other;
    return dx == typedOther.dx &&
           dy == typedOther.dy;
  }

  @override
  int get hashCode => hashValues(dx, dy);

  @override
  String toString() => 'Offset(${dx?.toStringAsFixed(1)}, ${dy?.toStringAsFixed(1)})';
}

/// Stub implementation. See docs in `../ui/`.
class Size extends OffsetBase {
  /// Stub implementation. See docs in `../ui/`.
  const Size(double width, double height) : super(width, height);

  /// Stub implementation. See docs in `../ui/`.
  // Used by the rendering library's _DebugSize hack.
  Size.copy(Size source) : super(source.width, source.height);

  /// Stub implementation. See docs in `../ui/`.
  const Size.square(double dimension) : super(dimension, dimension);

  /// Stub implementation. See docs in `../ui/`.
  const Size.fromWidth(double width) : super(width, double.infinity);

  /// Stub implementation. See docs in `../ui/`.
  const Size.fromHeight(double height) : super(double.infinity, height);

  /// Stub implementation. See docs in `../ui/`.
  const Size.fromRadius(double radius) : super(radius * 2.0, radius * 2.0);

  /// Stub implementation. See docs in `../ui/`.
  double get width => _dx;

  /// Stub implementation. See docs in `../ui/`.
  double get height => _dy;

  /// Stub implementation. See docs in `../ui/`.
  double get aspectRatio {
    if (height != 0.0)
      return width / height;
    if (width > 0.0)
      return double.infinity;
    if (width < 0.0)
      return double.negativeInfinity;
    return 0.0;
  }

  /// Stub implementation. See docs in `../ui/`.
  static const Size zero = const Size(0.0, 0.0);

  /// Stub implementation. See docs in `../ui/`.
  static const Size infinite = const Size(double.infinity, double.infinity);

  /// Stub implementation. See docs in `../ui/`.
  bool get isEmpty => width <= 0.0 || height <= 0.0;

  /// Stub implementation. See docs in `../ui/`.
  OffsetBase operator -(OffsetBase other) {
    if (other is Size)
      return new Offset(width - other.width, height - other.height);
    if (other is Offset)
      return new Size(width - other.dx, height - other.dy);
    throw new ArgumentError(other);
  }

  /// Stub implementation. See docs in `../ui/`.
  Size operator +(Offset other) => new Size(width + other.dx, height + other.dy);

  /// Stub implementation. See docs in `../ui/`.
  Size operator *(double operand) => new Size(width * operand, height * operand);

  /// Stub implementation. See docs in `../ui/`.
  Size operator /(double operand) => new Size(width / operand, height / operand);

  /// Stub implementation. See docs in `../ui/`.
  Size operator ~/(double operand) => new Size((width ~/ operand).toDouble(), (height ~/ operand).toDouble());

  /// Stub implementation. See docs in `../ui/`.
  Size operator %(double operand) => new Size(width % operand, height % operand);

  /// Stub implementation. See docs in `../ui/`.
  double get shortestSide => math.min(width.abs(), height.abs());

  /// Stub implementation. See docs in `../ui/`.
  double get longestSide => math.max(width.abs(), height.abs());

  // Convenience methods that do the equivalent of calling the similarly named
  // methods on a Rect constructed from the given origin and this size.

  /// Stub implementation. See docs in `../ui/`.
  Offset topLeft(Offset origin) => origin;

  /// Stub implementation. See docs in `../ui/`.
  Offset topCenter(Offset origin) => new Offset(origin.dx + width / 2.0, origin.dy);

  /// Stub implementation. See docs in `../ui/`.
  Offset topRight(Offset origin) => new Offset(origin.dx + width, origin.dy);

  /// Stub implementation. See docs in `../ui/`.
  Offset centerLeft(Offset origin) => new Offset(origin.dx, origin.dy + height / 2.0);

  /// Stub implementation. See docs in `../ui/`.
  Offset center(Offset origin) => new Offset(origin.dx + width / 2.0, origin.dy + height / 2.0);

  /// Stub implementation. See docs in `../ui/`.
  Offset centerRight(Offset origin) => new Offset(origin.dx + width, origin.dy + height / 2.0);

  /// Stub implementation. See docs in `../ui/`.
  Offset bottomLeft(Offset origin) => new Offset(origin.dx, origin.dy + height);

  /// Stub implementation. See docs in `../ui/`.
  Offset bottomCenter(Offset origin) => new Offset(origin.dx + width / 2.0, origin.dy + height);

  /// Stub implementation. See docs in `../ui/`.
  Offset bottomRight(Offset origin) => new Offset(origin.dx + width, origin.dy + height);

  /// Stub implementation. See docs in `../ui/`.
  bool contains(Offset offset) {
    return offset.dx >= 0.0 && offset.dx < width && offset.dy >= 0.0 && offset.dy < height;
  }

  /// Stub implementation. See docs in `../ui/`.
  Size get flipped => new Size(height, width);

  /// Stub implementation. See docs in `../ui/`.
  static Size lerp(Size a, Size b, double t) {
    assert(t != null);
    if (a == null && b == null)
      return null;
    if (a == null)
      return b * t;
    if (b == null)
      return a * (1.0 - t);
    return new Size(lerpDouble(a.width, b.width, t), lerpDouble(a.height, b.height, t));
  }

  /// Stub implementation. See docs in `../ui/`.
  // We don't compare the runtimeType because of _DebugSize in the framework.
  @override
  bool operator ==(dynamic other) {
    if (other is! Size)
      return false;
    final Size typedOther = other;
    return _dx == typedOther._dx &&
           _dy == typedOther._dy;
  }

  @override
  int get hashCode => hashValues(_dx, _dy);

  @override
  String toString() => 'Size(${width?.toStringAsFixed(1)}, ${height?.toStringAsFixed(1)})';
}

/// Stub implementation. See docs in `../ui/`.
class Rect {
  /// Stub implementation. See docs in `../ui/`.
  const Rect.fromLTRB(this.left, this.top, this.right, this.bottom)
    : assert(left != null),
      assert(top != null),
      assert(right != null),
      assert(bottom != null);

  /// Stub implementation. See docs in `../ui/`.
  const Rect.fromLTWH(double left, double top, double width, double height) : this.fromLTRB(left, top, left + width, top + height);

  /// Stub implementation. See docs in `../ui/`.
  Rect.fromCircle({ Offset center, double radius }) : this.fromCenter(
    center: center,
    width: radius * 2,
    height: radius * 2,
  );

  /// Stub implementation. See docs in `../ui/`.
  Rect.fromCenter({ Offset center, double width, double height }) : this.fromLTRB(
    center.dx - width / 2,
    center.dy - height / 2,
    center.dx + width / 2,
    center.dy + height / 2,
  );

  /// Stub implementation. See docs in `../ui/`.
  Rect.fromPoints(Offset a, Offset b) : this.fromLTRB(
    math.min(a.dx, b.dx),
    math.min(a.dy, b.dy),
    math.max(a.dx, b.dx),
    math.max(a.dy, b.dy),
  );

  Float32List get _value32 => Float32List.fromList(<double>[left, top, right, bottom]);

  /// Stub implementation. See docs in `../ui/`.
  final double left;

  /// Stub implementation. See docs in `../ui/`.
  final double top;

  /// Stub implementation. See docs in `../ui/`.
  final double right;

  /// Stub implementation. See docs in `../ui/`.
  final double bottom;

  /// Stub implementation. See docs in `../ui/`.
  double get width => right - left;

  /// Stub implementation. See docs in `../ui/`.
  double get height => bottom - top;

  /// Stub implementation. See docs in `../ui/`.
  Size get size => new Size(width, height);

  /// Stub implementation. See docs in `../ui/`.
  bool get hasNaN => left.isNaN || top.isNaN || right.isNaN || bottom.isNaN;

  /// Stub implementation. See docs in `../ui/`.
  static const Rect zero = Rect.fromLTRB(0.0, 0.0, 0.0, 0.0);

  static const double _giantScalar = 1.0E+9; // matches kGiantRect from layer.h

  /// Stub implementation. See docs in `../ui/`.
  static const Rect largest = Rect.fromLTRB(-_giantScalar, -_giantScalar, _giantScalar, _giantScalar);

  /// Stub implementation. See docs in `../ui/`.
  // included for consistency with Offset and Size
  bool get isInfinite {
    return left >= double.infinity
        || top >= double.infinity
        || right >= double.infinity
        || bottom >= double.infinity;
  }

  /// Stub implementation. See docs in `../ui/`.
  bool get isFinite => left.isFinite && top.isFinite && right.isFinite && bottom.isFinite;

  /// Stub implementation. See docs in `../ui/`.
  bool get isEmpty => left >= right || top >= bottom;

  /// Stub implementation. See docs in `../ui/`.
  Rect shift(Offset offset) {
    return new Rect.fromLTRB(left + offset.dx, top + offset.dy, right + offset.dx, bottom + offset.dy);
  }

  /// Stub implementation. See docs in `../ui/`.
  Rect translate(double translateX, double translateY) {
    return new Rect.fromLTRB(left + translateX, top + translateY, right + translateX, bottom + translateY);
  }

  /// Stub implementation. See docs in `../ui/`.
  Rect inflate(double delta) {
    return new Rect.fromLTRB(left - delta, top - delta, right + delta, bottom + delta);
  }

  /// Stub implementation. See docs in `../ui/`.
  Rect deflate(double delta) => inflate(-delta);

  /// Stub implementation. See docs in `../ui/`.
  Rect intersect(Rect other) {
    return new Rect.fromLTRB(
      math.max(left, other.left),
      math.max(top, other.top),
      math.min(right, other.right),
      math.min(bottom, other.bottom)
    );
  }

  /// Stub implementation. See docs in `../ui/`.
  Rect expandToInclude(Rect other) {
    return new Rect.fromLTRB(
        math.min(left, other.left),
        math.min(top, other.top),
        math.max(right, other.right),
        math.max(bottom, other.bottom),
    );
  }

  /// Stub implementation. See docs in `../ui/`.
  bool overlaps(Rect other) {
    if (right <= other.left || other.right <= left)
      return false;
    if (bottom <= other.top || other.bottom <= top)
      return false;
    return true;
  }

  /// Stub implementation. See docs in `../ui/`.
  double get shortestSide => math.min(width.abs(), height.abs());

  /// Stub implementation. See docs in `../ui/`.
  double get longestSide => math.max(width.abs(), height.abs());

  /// Stub implementation. See docs in `../ui/`.
  Offset get topLeft => new Offset(left, top);

  /// Stub implementation. See docs in `../ui/`.
  Offset get topCenter => new Offset(left + width / 2.0, top);

  /// Stub implementation. See docs in `../ui/`.
  Offset get topRight => new Offset(right, top);

  /// Stub implementation. See docs in `../ui/`.
  Offset get centerLeft => new Offset(left, top + height / 2.0);

  /// Stub implementation. See docs in `../ui/`.
  Offset get center => new Offset(left + width / 2.0, top + height / 2.0);

  /// Stub implementation. See docs in `../ui/`.
  Offset get centerRight => new Offset(right, top + height / 2.0);

  /// Stub implementation. See docs in `../ui/`.
  Offset get bottomLeft => new Offset(left, bottom);

  /// Stub implementation. See docs in `../ui/`.
  Offset get bottomCenter => new Offset(left + width / 2.0, bottom);

  /// Stub implementation. See docs in `../ui/`.
  Offset get bottomRight => new Offset(right, bottom);

  /// Stub implementation. See docs in `../ui/`.
  bool contains(Offset offset) {
    return offset.dx >= left && offset.dx < right && offset.dy >= top && offset.dy < bottom;
  }

  /// Stub implementation. See docs in `../ui/`.
  static Rect lerp(Rect a, Rect b, double t) {
    assert(t != null);
    if (a == null && b == null)
      return null;
    if (a == null)
      return new Rect.fromLTRB(b.left * t, b.top * t, b.right * t, b.bottom * t);
    if (b == null) {
      final double k = 1.0 - t;
      return new Rect.fromLTRB(a.left * k, a.top * k, a.right * k, a.bottom * k);
    }
    return new Rect.fromLTRB(
      lerpDouble(a.left, b.left, t),
      lerpDouble(a.top, b.top, t),
      lerpDouble(a.right, b.right, t),
      lerpDouble(a.bottom, b.bottom, t),
    );
  }

  @override
  bool operator ==(dynamic other) {
    if (identical(this, other))
      return true;
    if (runtimeType != other.runtimeType)
      return false;
    final Rect typedOther = other;
    return left   == typedOther.left   &&
           top    == typedOther.top    &&
           right  == typedOther.right  &&
           bottom == typedOther.bottom;
  }

  @override
  int get hashCode => hashValues(left, top, right, bottom);

  @override
  String toString() => 'Rect.fromLTRB(${left.toStringAsFixed(1)} ${top.toStringAsFixed(1)}, ${right.toStringAsFixed(1)}, ${bottom.toStringAsFixed(1)})';
}

/// Stub implementation. See docs in `../ui/`.
class Radius {
  /// Stub implementation. See docs in `../ui/`.
  const Radius.circular(double radius) : this.elliptical(radius, radius);

  /// Stub implementation. See docs in `../ui/`.
  const Radius.elliptical(this.x, this.y);

  /// Stub implementation. See docs in `../ui/`.
  final double x;

  /// Stub implementation. See docs in `../ui/`.
  final double y;

  /// Stub implementation. See docs in `../ui/`.
  static const Radius zero = const Radius.circular(0.0);

  /// Stub implementation. See docs in `../ui/`.
  Radius operator -() => new Radius.elliptical(-x, -y);

  /// Stub implementation. See docs in `../ui/`.
  Radius operator -(Radius other) => new Radius.elliptical(x - other.x, y - other.y);

  /// Stub implementation. See docs in `../ui/`.
  Radius operator +(Radius other) => new Radius.elliptical(x + other.x, y + other.y);

  /// Stub implementation. See docs in `../ui/`.
  Radius operator *(double operand) => new Radius.elliptical(x * operand, y * operand);

  /// Stub implementation. See docs in `../ui/`.
  Radius operator /(double operand) => new Radius.elliptical(x / operand, y / operand);

  /// Stub implementation. See docs in `../ui/`.
  Radius operator ~/(double operand) => new Radius.elliptical((x ~/ operand).toDouble(), (y ~/ operand).toDouble());

  /// Stub implementation. See docs in `../ui/`.
  Radius operator %(double operand) => new Radius.elliptical(x % operand, y % operand);

  /// Stub implementation. See docs in `../ui/`.
  static Radius lerp(Radius a, Radius b, double t) {
    assert(t != null);
    if (a == null && b == null)
      return null;
    if (a == null)
      return new Radius.elliptical(b.x * t, b.y * t);
    if (b == null) {
      final double k = 1.0 - t;
      return new Radius.elliptical(a.x * k, a.y * k);
    }
    return new Radius.elliptical(
      lerpDouble(a.x, b.x, t),
      lerpDouble(a.y, b.y, t),
    );
  }

  @override
  bool operator ==(dynamic other) {
    if (identical(this, other))
      return true;
    if (runtimeType != other.runtimeType)
      return false;
    final Radius typedOther = other;
    return typedOther.x == x && typedOther.y == y;
  }

  @override
  int get hashCode => hashValues(x, y);

  @override
  String toString() {
    return x == y ? 'Radius.circular(${x.toStringAsFixed(1)})' :
                    'Radius.elliptical(${x.toStringAsFixed(1)}, '
                    '${y.toStringAsFixed(1)})';
  }
}

/// Stub implementation. See docs in `../ui/`.
class RRect {
  /// Stub implementation. See docs in `../ui/`.
  const RRect.fromLTRBXY(double left, double top, double right, double bottom,
                   double radiusX, double radiusY) : this._raw(
    top: top,
    left: left,
    right: right,
    bottom: bottom,
    tlRadiusX: radiusX,
    tlRadiusY: radiusY,
    trRadiusX: radiusX,
    trRadiusY: radiusY,
    blRadiusX: radiusX,
    blRadiusY: radiusY,
    brRadiusX: radiusX,
    brRadiusY: radiusY,
  );

  /// Stub implementation. See docs in `../ui/`.
  RRect.fromLTRBR(double left, double top, double right, double bottom,
                  Radius radius)
    : this._raw(
        top: top,
        left: left,
        right: right,
        bottom: bottom,
        tlRadiusX: radius.x,
        tlRadiusY: radius.y,
        trRadiusX: radius.x,
        trRadiusY: radius.y,
        blRadiusX: radius.x,
        blRadiusY: radius.y,
        brRadiusX: radius.x,
        brRadiusY: radius.y,
      );

  /// Stub implementation. See docs in `../ui/`.
  RRect.fromRectXY(Rect rect, double radiusX, double radiusY)
    : this._raw(
        top: rect.top,
        left: rect.left,
        right: rect.right,
        bottom: rect.bottom,
        tlRadiusX: radiusX,
        tlRadiusY: radiusY,
        trRadiusX: radiusX,
        trRadiusY: radiusY,
        blRadiusX: radiusX,
        blRadiusY: radiusY,
        brRadiusX: radiusX,
        brRadiusY: radiusY,
      );

  /// Stub implementation. See docs in `../ui/`.
  RRect.fromRectAndRadius(Rect rect, Radius radius)
    : this._raw(
        top: rect.top,
        left: rect.left,
        right: rect.right,
        bottom: rect.bottom,
        tlRadiusX: radius.x,
        tlRadiusY: radius.y,
        trRadiusX: radius.x,
        trRadiusY: radius.y,
        blRadiusX: radius.x,
        blRadiusY: radius.y,
        brRadiusX: radius.x,
        brRadiusY: radius.y,
      );

  /// Stub implementation. See docs in `../ui/`.
  RRect.fromLTRBAndCorners(
    double left,
    double top,
    double right,
    double bottom, {
    Radius topLeft: Radius.zero,
    Radius topRight: Radius.zero,
    Radius bottomRight: Radius.zero,
    Radius bottomLeft: Radius.zero,
  }) : this._raw(
         top: top,
         left: left,
         right: right,
         bottom: bottom,
         tlRadiusX: topLeft.x,
         tlRadiusY: topLeft.y,
         trRadiusX: topRight.x,
         trRadiusY: topRight.y,
         blRadiusX: bottomLeft.x,
         blRadiusY: bottomLeft.y,
         brRadiusX: bottomRight.x,
         brRadiusY: bottomRight.y,
       );

  /// Stub implementation. See docs in `../ui/`.
  RRect.fromRectAndCorners(
    Rect rect,
    {
      Radius topLeft: Radius.zero,
      Radius topRight: Radius.zero,
      Radius bottomRight: Radius.zero,
      Radius bottomLeft: Radius.zero
    }
  ) : this._raw(
        top: rect.top,
        left: rect.left,
        right: rect.right,
        bottom: rect.bottom,
        tlRadiusX: topLeft.x,
        tlRadiusY: topLeft.y,
        trRadiusX: topRight.x,
        trRadiusY: topRight.y,
        blRadiusX: bottomLeft.x,
        blRadiusY: bottomLeft.y,
        brRadiusX: bottomRight.x,
        brRadiusY: bottomRight.y,
      );

  const RRect._raw({
    this.left,
    this.top,
    this.right,
    this.bottom,
    this.tlRadiusX,
    this.tlRadiusY,
    this.trRadiusX,
    this.trRadiusY,
    this.brRadiusX,
    this.brRadiusY,
    this.blRadiusX,
    this.blRadiusY,
  }) : assert(left != null),
       assert(top != null),
       assert(right != null),
       assert(bottom != null),
       assert(tlRadiusX != null),
       assert(tlRadiusY != null),
       assert(trRadiusX != null),
       assert(trRadiusY != null),
       assert(brRadiusX != null),
       assert(brRadiusY != null),
       assert(blRadiusX != null),
       assert(blRadiusY != null);

  Float32List get _value32 => Float32List.fromList(<double>[
    left,
    top,
    right,
    bottom,
    tlRadiusX,
    tlRadiusY,
    trRadiusX,
    trRadiusY,
    brRadiusX,
    brRadiusY,
    blRadiusX,
    blRadiusY,
  ]);

  /// Stub implementation. See docs in `../ui/`.
  final double left;

  /// Stub implementation. See docs in `../ui/`.
  final double top;

  /// Stub implementation. See docs in `../ui/`.
  final double right;

  /// Stub implementation. See docs in `../ui/`.
  final double bottom;

  /// Stub implementation. See docs in `../ui/`.
  final double tlRadiusX;

  /// Stub implementation. See docs in `../ui/`.
  final double tlRadiusY;

  /// Stub implementation. See docs in `../ui/`.
  Radius get tlRadius => new Radius.elliptical(tlRadiusX, tlRadiusY);

  /// Stub implementation. See docs in `../ui/`.
  final double trRadiusX;

  /// Stub implementation. See docs in `../ui/`.
  final double trRadiusY;

  /// Stub implementation. See docs in `../ui/`.
  Radius get trRadius => new Radius.elliptical(trRadiusX, trRadiusY);

  /// Stub implementation. See docs in `../ui/`.
  final double brRadiusX;

  /// Stub implementation. See docs in `../ui/`.
  final double brRadiusY;

  /// Stub implementation. See docs in `../ui/`.
  Radius get brRadius => new Radius.elliptical(brRadiusX, brRadiusY);

  /// Stub implementation. See docs in `../ui/`.
  final double blRadiusX;

  /// Stub implementation. See docs in `../ui/`.
  final double blRadiusY;

  /// Stub implementation. See docs in `../ui/`.
  Radius get blRadius => new Radius.elliptical(blRadiusX, blRadiusY);

  /// Stub implementation. See docs in `../ui/`.
  static final RRect zero = new RRect._raw();

  /// Stub implementation. See docs in `../ui/`.
  RRect shift(Offset offset) {
    return new RRect._raw(
      left: left + offset.dx,
      top: top + offset.dy,
      right: right + offset.dx,
      bottom: bottom + offset.dy,
      tlRadiusX: tlRadiusX,
      tlRadiusY: tlRadiusY,
      trRadiusX: trRadiusX,
      trRadiusY: trRadiusY,
      blRadiusX: blRadiusX,
      blRadiusY: blRadiusY,
      brRadiusX: brRadiusX,
      brRadiusY: brRadiusY,
    );
  }

  /// Stub implementation. See docs in `../ui/`.
  RRect inflate(double delta) {
    return new RRect._raw(
      left: left - delta,
      top: top - delta,
      right: right + delta,
      bottom: bottom + delta,
      tlRadiusX: tlRadiusX + delta,
      tlRadiusY: tlRadiusY + delta,
      trRadiusX: trRadiusX + delta,
      trRadiusY: trRadiusY + delta,
      blRadiusX: blRadiusX + delta,
      blRadiusY: blRadiusY + delta,
      brRadiusX: brRadiusX + delta,
      brRadiusY: brRadiusY + delta,
    );
  }

  /// Stub implementation. See docs in `../ui/`.
  RRect deflate(double delta) => inflate(-delta);

  /// Stub implementation. See docs in `../ui/`.
  double get width => right - left;

  /// Stub implementation. See docs in `../ui/`.
  double get height => bottom - top;

  /// Stub implementation. See docs in `../ui/`.
  Rect get outerRect => new Rect.fromLTRB(left, top, right, bottom);

  /// Stub implementation. See docs in `../ui/`.
  Rect get safeInnerRect {
    const double kInsetFactor = 0.29289321881; // 1-cos(pi/4)

    final double leftRadius = math.max(blRadiusX, tlRadiusX);
    final double topRadius = math.max(tlRadiusY, trRadiusY);
    final double rightRadius = math.max(trRadiusX, brRadiusX);
    final double bottomRadius = math.max(brRadiusY, blRadiusY);

    return new Rect.fromLTRB(
      left + leftRadius * kInsetFactor,
      top + topRadius * kInsetFactor,
      right - rightRadius * kInsetFactor,
      bottom - bottomRadius * kInsetFactor
    );
  }

  /// Stub implementation. See docs in `../ui/`.
  Rect get middleRect {
    final double leftRadius = math.max(blRadiusX, tlRadiusX);
    final double topRadius = math.max(tlRadiusY, trRadiusY);
    final double rightRadius = math.max(trRadiusX, brRadiusX);
    final double bottomRadius = math.max(brRadiusY, blRadiusY);
    return new Rect.fromLTRB(
      left + leftRadius,
      top + topRadius,
      right - rightRadius,
      bottom - bottomRadius
    );
  }

  /// Stub implementation. See docs in `../ui/`.
  Rect get wideMiddleRect {
    final double topRadius = math.max(tlRadiusY, trRadiusY);
    final double bottomRadius = math.max(brRadiusY, blRadiusY);
    return new Rect.fromLTRB(
      left,
      top + topRadius,
      right,
      bottom - bottomRadius
    );
  }

  /// Stub implementation. See docs in `../ui/`.
  Rect get tallMiddleRect {
    final double leftRadius = math.max(blRadiusX, tlRadiusX);
    final double rightRadius = math.max(trRadiusX, brRadiusX);
    return new Rect.fromLTRB(
      left + leftRadius,
      top,
      right - rightRadius,
      bottom
    );
  }

  /// Stub implementation. See docs in `../ui/`.
  bool get isEmpty => left >= right || top >= bottom;

  /// Stub implementation. See docs in `../ui/`.
  bool get isFinite => left.isFinite && top.isFinite && right.isFinite && bottom.isFinite;

  /// Stub implementation. See docs in `../ui/`.
  bool get isRect {
    return (tlRadiusX == 0.0 || tlRadiusY == 0.0) &&
           (trRadiusX == 0.0 || trRadiusY == 0.0) &&
           (blRadiusX == 0.0 || blRadiusY == 0.0) &&
           (brRadiusX == 0.0 || brRadiusY == 0.0);
  }

  /// Stub implementation. See docs in `../ui/`.
  bool get isStadium {
    return tlRadius == trRadius
        && trRadius == brRadius
        && brRadius == blRadius
        && (width <= 2.0 * tlRadiusX || height <= 2.0 * tlRadiusY);
  }

  /// Stub implementation. See docs in `../ui/`.
  bool get isEllipse {
    return tlRadius == trRadius
        && trRadius == brRadius
        && brRadius == blRadius
        && width <= 2.0 * tlRadiusX
        && height <= 2.0 * tlRadiusY;
  }

  /// Stub implementation. See docs in `../ui/`.
  bool get isCircle => width == height && isEllipse;

  /// Stub implementation. See docs in `../ui/`.
  double get shortestSide => math.min(width.abs(), height.abs());

  /// Stub implementation. See docs in `../ui/`.
  double get longestSide => math.max(width.abs(), height.abs());

  /// Stub implementation. See docs in `../ui/`.
  bool get hasNaN => left.isNaN || top.isNaN || right.isNaN || bottom.isNaN ||
                     trRadiusX.isNaN || trRadiusY.isNaN || tlRadiusX.isNaN || tlRadiusY.isNaN ||
                     brRadiusX.isNaN || brRadiusY.isNaN || blRadiusX.isNaN || blRadiusY.isNaN;

  /// Stub implementation. See docs in `../ui/`.
  Offset get center => new Offset(left + width / 2.0, top + height / 2.0);

  // Returns the minimum between min and scale to which radius1 and radius2
  // should be scaled with in order not to exceed the limit.
  double _getMin(double min, double radius1, double radius2, double limit) {
    final double sum = radius1 + radius2;
    if (sum > limit && sum != 0.0)
      return math.min(min, limit / sum);
    return min;
  }

  // Scales all radii so that on each side their sum will not pass the size of
  // the width/height.
  //
  // Inspired from:
  //   https://github.com/google/skia/blob/master/src/core/SkRRect.cpp#L164
  RRect _scaleRadii() {
    double scale = 1.0;
    scale = _getMin(scale, blRadiusY, tlRadiusY, height);
    scale = _getMin(scale, tlRadiusX, trRadiusX, width);
    scale = _getMin(scale, trRadiusY, brRadiusY, height);
    scale = _getMin(scale, brRadiusX, blRadiusX, width);

    double scaledLeft = left;
    double scaledTop = top;
    double scaledRight = right;
    double scaledBottom = bottom;

    if (scale < 1.0) {
      scaledTop *= scale;
      scaledLeft *= scale;
      scaledRight *= scale;
      scaledBottom *= scale;
    }

    return new RRect._raw(
      top: scaledTop,
      left: scaledLeft,
      right: scaledRight,
      bottom: scaledBottom,
      tlRadiusX: tlRadiusX,
      tlRadiusY: tlRadiusY,
      trRadiusX: trRadiusX,
      trRadiusY: trRadiusY,
      blRadiusX: blRadiusX,
      blRadiusY: blRadiusY,
      brRadiusX: brRadiusX,
      brRadiusY: brRadiusY,
    );
  }

  /// Stub implementation. See docs in `../ui/`.
  bool contains(Offset point) {
    if (point.dx < left || point.dx >= right || point.dy < top || point.dy >= bottom)
      return false; // outside bounding box

    final RRect scaled = _scaleRadii();

    double x;
    double y;
    double radiusX;
    double radiusY;
    // check whether point is in one of the rounded corner areas
    // x, y -> translate to ellipse center
    if (point.dx < left + scaled.tlRadiusX &&
        point.dy < top + scaled.tlRadiusY) {
      x = point.dx - left - scaled.tlRadiusX;
      y = point.dy - top - scaled.tlRadiusY;
      radiusX = scaled.tlRadiusX;
      radiusY = scaled.tlRadiusY;
    } else if (point.dx > right - scaled.trRadiusX &&
               point.dy < top + scaled.trRadiusY) {
      x = point.dx - right + scaled.trRadiusX;
      y = point.dy - top - scaled.trRadiusY;
      radiusX = scaled.trRadiusX;
      radiusY = scaled.trRadiusY;
    } else if (point.dx > right - scaled.brRadiusX &&
               point.dy > bottom - scaled.brRadiusY) {
      x = point.dx - right + scaled.brRadiusX;
      y = point.dy - bottom + scaled.brRadiusY;
      radiusX = scaled.brRadiusX;
      radiusY = scaled.brRadiusY;
    } else if (point.dx < left + scaled.blRadiusX &&
               point.dy > bottom - scaled.blRadiusY) {
      x = point.dx - left - scaled.blRadiusX;
      y = point.dy - bottom + scaled.blRadiusY;
      radiusX = scaled.blRadiusX;
      radiusY = scaled.blRadiusY;
    } else {
      return true; // inside and not within the rounded corner area
    }

    x = x / radiusX;
    y = y / radiusY;
    // check if the point is outside the unit circle
    if (x * x + y * y > 1.0)
      return false;
    return true;
  }

  /// Stub implementation. See docs in `../ui/`.
  static RRect lerp(RRect a, RRect b, double t) {
    assert(t != null);
    if (a == null && b == null)
      return null;
    if (a == null) {
      return new RRect._raw(
        left: b.left * t,
        top: b.top * t,
        right: b.right * t,
        bottom: b.bottom * t,
        tlRadiusX: b.tlRadiusX * t,
        tlRadiusY: b.tlRadiusY * t,
        trRadiusX: b.trRadiusX * t,
        trRadiusY: b.trRadiusY * t,
        brRadiusX: b.brRadiusX * t,
        brRadiusY: b.brRadiusY * t,
        blRadiusX: b.blRadiusX * t,
        blRadiusY: b.blRadiusY * t,
      );
    }
    if (b == null) {
      final double k = 1.0 - t;
      return new RRect._raw(
        left: a.left * k,
        top: a.top * k,
        right: a.right * k,
        bottom: a.bottom * k,
        tlRadiusX: a.tlRadiusX * k,
        tlRadiusY: a.tlRadiusY * k,
        trRadiusX: a.trRadiusX * k,
        trRadiusY: a.trRadiusY * k,
        brRadiusX: a.brRadiusX * k,
        brRadiusY: a.brRadiusY * k,
        blRadiusX: a.blRadiusX * k,
        blRadiusY: a.blRadiusY * k,
      );
    }
    return new RRect._raw(
      left: lerpDouble(a.left, b.left, t),
      top: lerpDouble(a.top, b.top, t),
      right: lerpDouble(a.right, b.right, t),
      bottom: lerpDouble(a.bottom, b.bottom, t),
      tlRadiusX: lerpDouble(a.tlRadiusX, b.tlRadiusX, t),
      tlRadiusY: lerpDouble(a.tlRadiusY, b.tlRadiusY, t),
      trRadiusX: lerpDouble(a.trRadiusX, b.trRadiusX, t),
      trRadiusY: lerpDouble(a.trRadiusY, b.trRadiusY, t),
      brRadiusX: lerpDouble(a.brRadiusX, b.brRadiusX, t),
      brRadiusY: lerpDouble(a.brRadiusY, b.brRadiusY, t),
      blRadiusX: lerpDouble(a.blRadiusX, b.blRadiusX, t),
      blRadiusY: lerpDouble(a.blRadiusY, b.blRadiusY, t),
    );
  }

  @override
  bool operator ==(dynamic other) {
    if (identical(this, other))
      return true;
    if (runtimeType != other.runtimeType)
      return false;
    final RRect typedOther = other;
    return left      == typedOther.left      &&
           top       == typedOther.top       &&
           right     == typedOther.right     &&
           bottom    == typedOther.bottom    &&
           tlRadiusX == typedOther.tlRadiusX &&
           tlRadiusY == typedOther.tlRadiusY &&
           trRadiusX == typedOther.trRadiusX &&
           trRadiusY == typedOther.trRadiusY &&
           blRadiusX == typedOther.blRadiusX &&
           blRadiusY == typedOther.blRadiusY &&
           brRadiusX == typedOther.brRadiusX &&
           brRadiusY == typedOther.brRadiusY;
  }

  @override
  int get hashCode => hashValues(left, top, right, bottom,
    tlRadiusX, tlRadiusY, trRadiusX, trRadiusY,
    blRadiusX, blRadiusY, brRadiusX, brRadiusY);

  @override
  String toString() {
    final String rect = '${left.toStringAsFixed(1)}, '
                        '${top.toStringAsFixed(1)}, '
                        '${right.toStringAsFixed(1)}, '
                        '${bottom.toStringAsFixed(1)}';
    if (tlRadius == trRadius &&
        trRadius == brRadius &&
        brRadius == blRadius) {
      if (tlRadius.x == tlRadius.y)
        return 'RRect.fromLTRBR($rect, ${tlRadius.x.toStringAsFixed(1)})';
      return 'RRect.fromLTRBXY($rect, ${tlRadius.x.toStringAsFixed(1)}, ${tlRadius.y.toStringAsFixed(1)})';
    }
    return 'RRect.fromLTRBAndCorners('
             '$rect, '
             'topLeft: $tlRadius, '
             'topRight: $trRadius, '
             'bottomRight: $brRadius, '
             'bottomLeft: $blRadius'
           ')';
  }
}

/// Stub implementation. See docs in `../ui/`.
// Modeled after Skia's SkRSXform.
class RSTransform {
  /// Stub implementation. See docs in `../ui/`.
  RSTransform(double scos, double ssin, double tx, double ty) {
    _value
      ..[0] = scos
      ..[1] = ssin
      ..[2] = tx
      ..[3] = ty;
  }

  /// Stub implementation. See docs in `../ui/`.
  factory RSTransform.fromComponents({
    double rotation,
    double scale,
    double anchorX,
    double anchorY,
    double translateX,
    double translateY
  }) {
    final double scos = math.cos(rotation) * scale;
    final double ssin = math.sin(rotation) * scale;
    final double tx = translateX + -scos * anchorX + ssin * anchorY;
    final double ty = translateY + -ssin * anchorX - scos * anchorY;
    return new RSTransform(scos, ssin, tx, ty);
  }

  final Float32List _value = new Float32List(4);

  /// Stub implementation. See docs in `../ui/`.
  double get scos => _value[0];

  /// Stub implementation. See docs in `../ui/`.
  double get ssin => _value[1];

  /// Stub implementation. See docs in `../ui/`.
  double get tx => _value[2];

  /// Stub implementation. See docs in `../ui/`.
  double get ty => _value[3];
}
