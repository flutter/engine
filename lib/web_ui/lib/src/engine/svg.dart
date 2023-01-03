// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Because JS binding classes are empty shells, any static members can stand
// alone.
// ignore_for_file: avoid_classes_with_only_static_members

import 'package:js/js.dart';
import 'package:js/js_util.dart' as js_util;
import 'package:ui/src/engine/vector_math.dart';

import 'dom.dart';

// Because SVG is silly and doesn't have direct constructors for simple values,
// such as `SVGNumber` and `SVGLength`, we need a dummy `<svg>` element whose
// methods we can use to create such values. See all methods that begin with
// "create" on MDN:
//
// https://developer.mozilla.org/en-US/docs/Web/API/SVGSVGElement
final SVGSVGElement _svgValueFactory = createSVGSVGElement();

@JS()
@staticInterop
class SVGElement extends DomElement {}

SVGElement createSVGElement(String tag) =>
    domDocument.createElementNS('http://www.w3.org/2000/svg', tag)
        as SVGElement;

@JS()
@staticInterop
class SVGGraphicsElement extends SVGElement {}
extension SVGGraphicsElementExtension on SVGGraphicsElement {
  external SVGAnimatedTransformList get transform;
}

@JS()
@staticInterop
class SVGAnimatedTransformList extends SVGElement {}
extension SVGAnimatedTransformListExtension on SVGAnimatedTransformList {
  external SVGTransformList get baseVal;
}

@JS()
@staticInterop
class SVGTransform extends SVGElement {}
extension SVGTransformExtension on SVGTransform {
  /// The matrix representing this transform.
  ///
  /// The returned object is live, meaning that any changes made to the
  /// [SVGTransform] object are immediately reflected in the matrix object and
  /// vice versa.
  external SVGMatrix get matrix;

  external void setMatrix(SVGMatrix matrix);
  external void setTranslate(double tx, double ty);
  external void setScale(double sx, double sy);
  external void setRotate(double angle, double cx, double cy);
  external void setSkewX(double angle);
  external void setSkewY(double angle);
}

@JS()
@staticInterop
class SVGTransformList extends SVGElement {}
extension SVGTransformListExtension on SVGTransformList {
  external SVGTransform appendItem(SVGTransform newItem);
  external SVGTransform getItem(int index);
}

/// The `<svg>` element containing an SVG picture.
@JS()
@staticInterop
class SVGSVGElement extends SVGGraphicsElement {}

SVGSVGElement createSVGSVGElement() {
  final SVGElement el = createSVGElement('svg');
  el.setAttribute('version', '1.1');
  return el as SVGSVGElement;
}

extension SVGSVGElementExtension on SVGSVGElement {
  external SVGNumber createSVGNumber();
  external SVGLength createSVGLength();
  external SVGMatrix createSVGMatrix();

  external SVGAnimatedLength get height;
  void setHeight(double height) {
    this.height.baseVal.newValueSpecifiedUnits(height);
  }

  external SVGAnimatedLength get width;
  void setWidth(double width) {
    this.width.baseVal.newValueSpecifiedUnits(width);
  }
}

@JS()
@staticInterop
class SVGGElement extends SVGGraphicsElement {
}

SVGGElement createSVGGElement() {
  return createSVGElement('g') as SVGGElement;
}

@JS()
@staticInterop
class SVGImageElement extends SVGGraphicsElement {}
extension SVGImageElementExtension on SVGImageElement {
  external SVGAnimatedString get href;
  external SVGAnimatedLength get width;
  external SVGAnimatedLength get height;
}

SVGImageElement createSVGImageElement() {
  return createSVGElement('image') as SVGImageElement;
}

@JS()
@staticInterop
class SVGGeometryElement extends SVGGraphicsElement {}


@JS()
@staticInterop
class SVGForeignObjectElement extends SVGGraphicsElement {}

SVGForeignObjectElement createSVGForeignObjectElement() {
  return createSVGElement('foreignObject') as SVGForeignObjectElement;
}

extension SVGForeignObjectElementExtension on SVGForeignObjectElement {
  external SVGAnimatedLength get height;
  void setHeight(double height) {
    this.height.baseVal.newValueSpecifiedUnits(height);
  }

  external SVGAnimatedLength get width;
  void setWidth(double width) {
    this.width.baseVal.newValueSpecifiedUnits(width);
  }

  external SVGAnimatedLength get x;
  void setX(double x) {
    this.x.baseVal.newValueSpecifiedUnits(x);
  }

  external SVGAnimatedLength get y;
  void setY(double y) {
    this.y.baseVal.newValueSpecifiedUnits(y);
  }
}

@JS()
@staticInterop
class SVGTextContentElement extends SVGGraphicsElement {
}

extension SVGTextContentElementExtension on SVGTextContentElement {
  external SVGAnimatedLength get textLength;
  external SVGAnimatedEnumeration get lengthAdjust;
}

@JS()
@staticInterop
class SVGTextPositioningElement extends SVGTextContentElement {}

extension SVGTextPositioningElementExtension on SVGTextPositioningElement {
  external SVGAnimatedLengthList get x;
  void addX(double x) {
    this.x.baseVal.appendItem(SVGLength.create(x));
  }

  external SVGAnimatedLengthList get y;
  void addY(double y) {
    this.y.baseVal.appendItem(SVGLength.create(y));
  }

  external SVGAnimatedLengthList get dx;
  void addDx(double dx) {
    this.dx.baseVal.appendItem(SVGLength.create(dx));
  }

  external SVGAnimatedLengthList get dy;
  void addDy(double dy) {
    this.dy.baseVal.appendItem(SVGLength.create(dy));
  }
}

@JS()
@staticInterop
class SVGTextElement extends SVGTextPositioningElement {}

extension SVGTextElementExtension on SVGTextElement {
  external SVGAnimatedLength get textLength;
}

SVGTextElement createSVGTextElement() {
  return createSVGElement('text') as SVGTextElement;
}

@JS()
@staticInterop
class SVGTSpanElement extends SVGTextPositioningElement {}

SVGTSpanElement createSVGTSpanElement() {
  return createSVGElement('tspan') as SVGTSpanElement;
}

extension SVGTSpanElementExtension on SVGTSpanElement {
  external SVGAnimatedLength get textLength;
}

/// Bindings for https://developer.mozilla.org/en-US/docs/Web/API/SVGRectElement.
@JS()
@staticInterop
class SVGRectElement extends SVGGeometryElement {}
extension SVGRectElementExtension on SVGRectElement {
  external SVGAnimatedLength get height;
  void setHeight(double height) {
    this.height.baseVal.newValueSpecifiedUnits(height);
  }

  external SVGAnimatedLength get width;
  void setWidth(double width) {
    this.width.baseVal.newValueSpecifiedUnits(width);
  }

  external SVGAnimatedLength get x;
  void setX(double x) {
    this.x.baseVal.newValueSpecifiedUnits(x);
  }

  external SVGAnimatedLength get y;
  void setY(double y) {
    this.y.baseVal.newValueSpecifiedUnits(y);
  }

  external SVGAnimatedLength get rx;
  void setRx(double rx) {
    this.rx.baseVal.newValueSpecifiedUnits(rx);
  }

  external SVGAnimatedLength get ry;
  void setRy(double ry) {
    this.ry.baseVal.newValueSpecifiedUnits(ry);
  }
}

SVGRectElement createSVGRectElement() {
  return createSVGElement('rect') as SVGRectElement;
}

@JS()
@staticInterop
class SVGLineElement extends SVGGeometryElement {}
extension SVGLineElementExtension on SVGLineElement {
  external SVGAnimatedLength get x1;
  void setX1(double x1) {
    this.x1.baseVal.newValueSpecifiedUnits(x1);
  }

  external SVGAnimatedLength get y1;
  void setY1(double y1) {
    this.y1.baseVal.newValueSpecifiedUnits(y1);
  }

  external SVGAnimatedLength get x2;
  void setX2(double x2) {
    this.x2.baseVal.newValueSpecifiedUnits(x2);
  }

  external SVGAnimatedLength get y2;
  void setY2(double y2) {
    this.y2.baseVal.newValueSpecifiedUnits(y2);
  }
}

SVGLineElement createSVGLineElement() {
  return createSVGElement('line') as SVGLineElement;
}

@JS()
@staticInterop
class SVGClipPathElement extends SVGGraphicsElement {}

SVGClipPathElement createSVGClipPathElement() =>
    domDocument.createElementNS('http://www.w3.org/2000/svg', 'clipPath')
        as SVGClipPathElement;

@JS()
@staticInterop
class SVGDefsElement extends SVGGraphicsElement {}

SVGDefsElement createSVGDefsElement() =>
    domDocument.createElementNS('http://www.w3.org/2000/svg', 'defs')
        as SVGDefsElement;

@JS()
@staticInterop
class SVGPathElement extends SVGGeometryElement {}

SVGPathElement createSVGPathElement() =>
    domDocument.createElementNS('http://www.w3.org/2000/svg', 'path')
        as SVGPathElement;

@JS()
@staticInterop
class SVGFilterElement extends SVGElement {}

extension SVGFilterElementExtension on SVGFilterElement {
  external SVGAnimatedEnumeration get filterUnits;

  external SVGAnimatedLength get height;
  void setHeight(double height) {
    this.height.baseVal.newValueSpecifiedUnits(height);
  }

  external SVGAnimatedLength get width;
  void setWidth(double width) {
    this.width.baseVal.newValueSpecifiedUnits(width);
  }

  external SVGAnimatedLength get x;
  void setX(double x) {
    this.x.baseVal.newValueSpecifiedUnits(x);
  }

  external SVGAnimatedLength get y;
  void setY(double y) {
    this.y.baseVal.newValueSpecifiedUnits(y);
  }
}

SVGFilterElement createSVGFilterElement() =>
    domDocument.createElementNS('http://www.w3.org/2000/svg', 'filter')
        as SVGFilterElement;

@JS()
@staticInterop
class SVGAnimatedLength {}
extension SVGAnimatedLengthExtension on SVGAnimatedLength {
  external SVGLength get baseVal;
}

@JS()
@staticInterop
class SVGAnimatedLengthList {}
extension SVGAnimatedLengthListExtension on SVGAnimatedLengthList {
  external SVGLengthList get baseVal;
}

@JS()
@staticInterop
class SVGLengthList {}

extension SVGLengthListExtension on SVGLengthList {
  external SVGLength appendItem(SVGLength newItem);
}

@JS()
@staticInterop
class SVGLength {
  static SVGLength create(num valueInSpecifiedUnits, [SVGLengthUnitType unitType = SVGLengthUnitType.number]) {
    return _svgValueFactory.createSVGLength()
      ..newValueSpecifiedUnits(valueInSpecifiedUnits, unitType);
  }
}

extension SVGLengthExtension on SVGLength {
  external set valueAsString(String? value);

  /// A type-safe wrapper for the underlying `newValueSpecifiedUnits` method with
  /// reasonable defaults.
  void newValueSpecifiedUnits(num valueInSpecifiedUnits, [SVGLengthUnitType unitType = SVGLengthUnitType.number]) {
    _newValueSpecifiedUnits(this, unitType._svgValue, valueInSpecifiedUnits);
  }
}

class SVGLengthUnitType {
  const SVGLengthUnitType(this._svgValue);

  static const SVGLengthUnitType unknown = SVGLengthUnitType(0);
  static const SVGLengthUnitType number = SVGLengthUnitType(1);
  static const SVGLengthUnitType percentage = SVGLengthUnitType(2);
  static const SVGLengthUnitType ems = SVGLengthUnitType(3);
  static const SVGLengthUnitType exs = SVGLengthUnitType(4);
  static const SVGLengthUnitType px = SVGLengthUnitType(5);
  static const SVGLengthUnitType cm = SVGLengthUnitType(6);
  static const SVGLengthUnitType mm = SVGLengthUnitType(7);
  static const SVGLengthUnitType inch = SVGLengthUnitType(8);
  static const SVGLengthUnitType pt = SVGLengthUnitType(9);
  static const SVGLengthUnitType pc = SVGLengthUnitType(10);

  final int _svgValue;
}

void _newValueSpecifiedUnits(SVGLength length, int unitType, num valueInSpecifiedUnits) {
  js_util.callMethod<void>(
    length,
    'newValueSpecifiedUnits',
    <Object>[unitType, valueInSpecifiedUnits],
  );
}

@JS()
@staticInterop
class SVGAnimatedEnumeration {}

extension SVGAnimatedEnumerationExtenson on SVGAnimatedEnumeration {
  external set baseVal(int? value);
}

@JS()
@staticInterop
class SVGFEColorMatrixElement extends SVGElement {}
extension SVGFEColorMatrixElementExtension on SVGFEColorMatrixElement {
  external SVGAnimatedEnumeration get type;
  external SVGAnimatedString get result;
  external SVGAnimatedNumberList get values;
}

SVGFEColorMatrixElement createSVGFEColorMatrixElement() =>
    domDocument.createElementNS('http://www.w3.org/2000/svg', 'feColorMatrix')
        as SVGFEColorMatrixElement;

@JS()
@staticInterop
class SVGFEGaussianBlurElement extends SVGElement {}
extension SVGFEGaussianBlurElementExtension on SVGFEGaussianBlurElement {
  external SVGAnimatedString get in1;
  external SVGAnimatedNumber get stdDeviationX;
  external SVGAnimatedNumber get stdDeviationY;
  external SVGAnimatedString get result;
}

SVGFEGaussianBlurElement createSVGFEGaussianBlurElement() =>
    domDocument.createElementNS('http://www.w3.org/2000/svg', 'feGaussianBlur')
        as SVGFEGaussianBlurElement;

@JS()
@staticInterop
class SVGFEFloodElement extends SVGElement {}

extension SVGFEFloodElementExtension on SVGFEFloodElement {
  external SVGAnimatedString get result;
}

SVGFEFloodElement createSVGFEFloodElement() =>
    domDocument.createElementNS('http://www.w3.org/2000/svg', 'feFlood')
        as SVGFEFloodElement;

@JS()
@staticInterop
class SVGFEBlendElement extends SVGElement {}

extension SVGFEBlendElementExtension on SVGFEBlendElement {
  external SVGAnimatedString get in1;
  external SVGAnimatedString get in2;
  external SVGAnimatedEnumeration get mode;
}

SVGFEBlendElement createSVGFEBlendElement() =>
    domDocument.createElementNS('http://www.w3.org/2000/svg', 'feBlend')
        as SVGFEBlendElement;

@JS()
@staticInterop
class SVGFEImageElement extends SVGElement {}

extension SVGFEImageElementExtension on SVGFEImageElement {
  external SVGAnimatedLength get height;
  void setHeight(double height) {
    this.height.baseVal.newValueSpecifiedUnits(height);
  }

  external SVGAnimatedLength get width;
  void setWidth(double width) {
    this.width.baseVal.newValueSpecifiedUnits(width);
  }

  external SVGAnimatedString get result;
  void setResult(String result) {
    this.result.baseVal = result;
  }

  external SVGAnimatedLength get x;
  void setX(double x) {
    this.x.baseVal.newValueSpecifiedUnits(x);
  }

  external SVGAnimatedLength get y;
  void setY(double y) {
    this.y.baseVal.newValueSpecifiedUnits(y);
  }

  external SVGAnimatedString get href;
  void setHref(String href) {
    this.href.baseVal = href;
  }
}

SVGFEImageElement createSVGFEImageElement() =>
    domDocument.createElementNS('http://www.w3.org/2000/svg', 'feImage')
        as SVGFEImageElement;

@JS()
@staticInterop
class SVGFECompositeElement extends SVGElement {}

SVGFECompositeElement createSVGFECompositeElement() =>
    domDocument.createElementNS('http://www.w3.org/2000/svg', 'feComposite')
        as SVGFECompositeElement;

extension SVGFEBlendCompositeExtension on SVGFECompositeElement {
  external SVGAnimatedString get in1;
  external SVGAnimatedString get in2;
  external SVGAnimatedNumber get k1;
  external SVGAnimatedNumber get k2;
  external SVGAnimatedNumber get k3;
  external SVGAnimatedNumber get k4;
  external SVGAnimatedEnumeration get operator;
  external SVGAnimatedString get result;
}

@JS()
@staticInterop
class SVGAnimatedString {}

extension SVGAnimatedStringExtension on SVGAnimatedString {
  external set baseVal(String? value);
}

@JS()
@staticInterop
class SVGAnimatedNumber {}

extension SVGAnimatedNumberExtension on SVGAnimatedNumber {
  external set baseVal(num? value);
}

@JS()
@staticInterop
class SVGAnimatedNumberList {}

extension SVGAnimatedNumberListExtension on SVGAnimatedNumberList {
  external SVGNumberList? get baseVal;
}

@JS()
@staticInterop
class SVGNumberList {}

extension SVGNumberListExtension on SVGNumberList {
  external SVGNumber appendItem(SVGNumber newItem);
}

@JS()
@staticInterop
class SVGNumber {
  static SVGNumber create(num value) {
    return _svgValueFactory.createSVGNumber()..value = value;
  }
}

extension SVGNumberExtension on SVGNumber {
  external set value(num? value);
}

/// An SVG-specific 2x3 transform matrix.
///
/// This is not sufficient to represent arbitrary 4x4 matrices the Flutter uses,
/// but many basic things can be expressed. See [fromMatrix4].
///
/// W3C aims to replace this type with a more capable [DOMMatrix] in the future,
/// but right now many SVG APIs are still using it, and so must we. See:
///
/// - https://developer.mozilla.org/en-US/docs/Web/API/SVGMatrix
/// - https://github.com/w3c/svgwg/issues/706
@JS()
@staticInterop
class SVGMatrix {
  static SVGMatrix identity() => _svgValueFactory.createSVGMatrix();

  static SVGMatrix fromComponents({
    required double a,
    required double b,
    required double c,
    required double d,
    required double e,
    required double f,
  }) {
    final SVGMatrix matrix = _svgValueFactory.createSVGMatrix();
    matrix.a = a;
    matrix.b = b;
    matrix.c = c;
    matrix.d = d;
    matrix.e = e;
    matrix.f = f;
    return matrix;
  }

  /// SVGMatrix is a 2x3 matrix.
  ///
  /// In 3x3 matrix form assuming vector representation of (x, y, 1):
  ///
  /// a c e
  /// b d f
  /// 0 0 1
  ///
  /// This translates to 4x4 matrix with vector representation of (x, y, z, 1)
  /// as:
  ///
  /// a c 0 e
  /// b d 0 f
  /// 0 0 1 0
  /// 0 0 0 1
  ///
  /// This matrix is sufficient to represent 2D rotates, translates, scales,
  /// and skews.
  static SVGMatrix fromMatrix4(Matrix4 other) {
    return SVGMatrix.fromComponents(
      a: other[0],
      b: other[1],
      c: other[4],
      d: other[5],
      e: other[12],
      f: other[13],
    );
  }
}
extension SVGMatrixExtension on SVGMatrix {
  external double a;
  external double b;
  external double c;
  external double d;
  external double e;
  external double f;

  external SVGMatrix multiply(SVGMatrix secondMatrix);
  external SVGMatrix inverse();
  external SVGMatrix translate(double x, double y);
  external SVGMatrix scale(double scaleFactor);
  external SVGMatrix scaleNonUniform(double scaleFactorX, double scaleFactorY);
  external SVGMatrix rotate(double angle);
  external SVGMatrix rotateFromVector(double x, double y);
  external SVGMatrix flipX();
  external SVGMatrix flipY();
  external SVGMatrix skewX(double angle);
  external SVGMatrix skewY(double angle);
}
