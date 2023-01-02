// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import 'browser_detection.dart';
import 'dom.dart';
import 'svg.dart';
import 'util.dart';

/// Used for clipping and filter svg resources.
///
/// Position needs to be absolute since these svgs are sandwiched between
/// canvas elements and can cause layout shifts otherwise.
final SVGSVGElement kSvgResourceHeader = createSVGSVGElement()
  ..setAttribute('width', 0)
  ..setAttribute('height', 0)
  ..style.position = 'absolute';

SvgFilter svgFilterFromBlendMode(
    ui.Color? filterColor, ui.BlendMode colorFilterBlendMode) {
  final SvgFilter svgFilter;
  switch (colorFilterBlendMode) {
    case ui.BlendMode.srcIn:
    case ui.BlendMode.srcATop:
      svgFilter = _srcInColorFilterToSvg(filterColor);
      break;
    case ui.BlendMode.srcOut:
      svgFilter = _srcOutColorFilterToSvg(filterColor);
      break;
    case ui.BlendMode.dstATop:
      svgFilter = _dstATopColorFilterToSvg(filterColor);
      break;
    case ui.BlendMode.xor:
      svgFilter = _xorColorFilterToSvg(filterColor);
      break;
    case ui.BlendMode.plus:
      // Porter duff source + destination.
      svgFilter = _compositeColorFilterToSvg(filterColor, 0, 1, 1, 0);
      break;
    case ui.BlendMode.modulate:
      // Porter duff source * destination but preserves alpha.
      svgFilter = _modulateColorFilterToSvg(filterColor!);
      break;
    case ui.BlendMode.overlay:
      // Since overlay is the same as hard-light by swapping layers,
      // pass hard-light blend function.
      svgFilter = _blendColorFilterToSvg(
        filterColor,
        blendModeToSvgEnum(ui.BlendMode.hardLight)!,
        swapLayers: true,
      );
      break;
    // Several of the filters below (although supported) do not render the
    // same (close but not exact) as native flutter when used as blend mode
    // for a background-image with a background color. They only look
    // identical when feBlend is used within an svg filter definition.
    //
    // Saturation filter uses destination when source is transparent.
    // cMax = math.max(r, math.max(b, g));
    // cMin = math.min(r, math.min(b, g));
    // delta = cMax - cMin;
    // lightness = (cMax + cMin) / 2.0;
    // saturation = delta / (1.0 - (2 * lightness - 1.0).abs());
    case ui.BlendMode.saturation:
    case ui.BlendMode.colorDodge:
    case ui.BlendMode.colorBurn:
    case ui.BlendMode.hue:
    case ui.BlendMode.color:
    case ui.BlendMode.luminosity:
    case ui.BlendMode.multiply:
    case ui.BlendMode.screen:
    case ui.BlendMode.darken:
    case ui.BlendMode.lighten:
    case ui.BlendMode.hardLight:
    case ui.BlendMode.softLight:
    case ui.BlendMode.difference:
    case ui.BlendMode.exclusion:
      svgFilter = _blendColorFilterToSvg(
          filterColor, blendModeToSvgEnum(colorFilterBlendMode)!);
      break;
    case ui.BlendMode.src:
    case ui.BlendMode.dst:
    case ui.BlendMode.dstIn:
    case ui.BlendMode.dstOut:
    case ui.BlendMode.dstOver:
    case ui.BlendMode.clear:
    case ui.BlendMode.srcOver:
      throw UnimplementedError(
        'Blend mode not supported in HTML renderer: $colorFilterBlendMode',
      );
  }
  return svgFilter;
}

// See: https://www.w3.org/TR/SVG11/types.html#InterfaceSVGUnitTypes
const int kUserSpaceOnUse = 1;
const int kObjectBoundingBox = 2;

// See: https://www.w3.org/TR/SVG11/filters.html#InterfaceSVGFEColorMatrixElement
const int kMatrixType = 1;

// See: https://www.w3.org/TR/SVG11/filters.html#InterfaceSVGFECompositeElement
const int kOperatorOut = 3;
const int kOperatorAtop = 4;
const int kOperatorXor = 5;
const int kOperatorArithmetic = 6;

/// Configures an SVG filter for a specific target type.
///
/// SVG filters needs to be configured differently depending on whether they are
/// applied to HTML or SVG. This enum communicates to [SvgFilterBuilder],
/// [SvgFilter], and other code constructing SVG filters what the indended
/// target is.
enum SvgFilterTargetType {
  /// The target of the SVG filter is an SVG element.
  svg,

  /// The target of the SVG filter is an HTML element.
  html,
}

/// Builds an [SvgFilter].
class SvgFilterBuilder {
  SvgFilterBuilder({ required SvgFilterTargetType targetType }) : id = '_fcf${++_filterIdCounter}' {
    filter.id = id;

    switch (targetType) {
      case SvgFilterTargetType.svg:
        filter.filterUnits.baseVal = kUserSpaceOnUse;
        break;
      case SvgFilterTargetType.html:
        // SVG filters that contain `<feImage>` will fail on several browsers
        // (e.g. Firefox) if bounds are not specified.
        filter.filterUnits.baseVal = kObjectBoundingBox;

        // On Firefox percentage width/height 100% works however fails in Chrome 88.
        filter.x.baseVal.valueAsString = '0%';
        filter.y.baseVal.valueAsString = '0%';
        filter.width.baseVal.valueAsString = '100%';
        filter.height.baseVal.valueAsString = '100%';
        break;
    }
  }

  static int _filterIdCounter = 0;

  final String id;
  final SVGFilterElement filter = createSVGFilterElement();

  set colorInterpolationFilters(String filters) {
    filter.setAttribute('color-interpolation-filters', filters);
  }

  void setFeGaussianBlur({
    required double sigmaX,
    required double sigmaY,
    required ui.Rect areaOfEffect,
    String in1 = 'SourceGraphic',
    String result = 'comp',
  }) {
    final SVGFEGaussianBlurElement element = createSVGFEGaussianBlurElement();
    element.in1.baseVal = in1;
    element.stdDeviationX.baseVal = sigmaX;
    element.stdDeviationY.baseVal = sigmaY;
    element.result.baseVal = result;
    filter.x.baseVal.valueAsString = '${areaOfEffect.left}';
    filter.y.baseVal.valueAsString = '${areaOfEffect.top}';
    filter.width.baseVal.valueAsString = '${areaOfEffect.width}';
    filter.height.baseVal.valueAsString = '${areaOfEffect.height}';
    filter.append(element);
  }

  void setFeColorMatrix(List<double> matrix, { required String result }) {
    final SVGFEColorMatrixElement element = createSVGFEColorMatrixElement();
    element.type.baseVal = kMatrixType;
    element.result.baseVal = result;
    final SVGNumberList value = element.values.baseVal!;
    for (int i = 0; i < matrix.length; i++) {
      value.appendItem(SVGNumber.create(matrix[i]));
    }
    filter.append(element);
  }

  void setFeFlood({
    required String floodColor,
    required String floodOpacity,
    required String result,
  }) {
    final SVGFEFloodElement element = createSVGFEFloodElement();
    element.setAttribute('flood-color', floodColor);
    element.setAttribute('flood-opacity', floodOpacity);
    element.result.baseVal = result;
    filter.append(element);
  }

  void setFeBlend({
    required String in1,
    required String in2,
    required int mode,
  }) {
    final SVGFEBlendElement element = createSVGFEBlendElement();
    element.in1.baseVal = in1;
    element.in2.baseVal = in2;
    element.mode.baseVal = mode;
    filter.append(element);
  }

  void setFeComposite({
    required String in1,
    required String in2,
    required int operator,
    num? k1,
    num? k2,
    num? k3,
    num? k4,
    required String result,
  }) {
    final SVGFECompositeElement element = createSVGFECompositeElement();
    element.in1.baseVal = in1;
    element.in2.baseVal = in2;
    element.operator.baseVal = operator;
    if (k1 != null) {
      element.k1.baseVal = k1;
    }
    if (k2 != null) {
      element.k2.baseVal = k2;
    }
    if (k3 != null) {
      element.k3.baseVal = k3;
    }
    if (k4 != null) {
      element.k4.baseVal = k4;
    }
    element.result.baseVal = result;
    filter.append(element);
  }

  void setFeImage({
    required String href,
    required String result,
    required double width,
    required double height,
  }) {
    final SVGFEImageElement element = createSVGFEImageElement();
    element.href.baseVal = href;
    element.result.baseVal = result;

    // WebKit will not render if x/y/width/height is specified. So we return
    // explicit size here unless running on WebKit.
    if (browserEngine != BrowserEngine.webkit) {
      element.setX(0);
      element.setY(0);
      element.setWidth(width);
      element.setHeight(height);
    }
    filter.append(element);
  }

  SvgFilter build([SVGElement? host]) {
    host ??= kSvgResourceHeader.cloneNode(false) as SVGSVGElement;
    host.append(filter);
    return SvgFilter._(id, host);
  }
}

class SvgFilter {
  SvgFilter._(this.id, this.element);

  final String id;
  final SVGElement element;

  void applyToSvg(SVGElement target) {
    target.setAttribute('filter', 'url(#$id)');
  }
}

SvgFilter svgFilterFromColorMatrix(List<double> matrix) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeColorMatrix(matrix, result: 'comp');
  return builder.build();
}

// The color matrix for feColorMatrix element changes colors based on
// the following:
//
// | R' |     | r1 r2 r3 r4 r5 |   | R |
// | G' |     | g1 g2 g3 g4 g5 |   | G |
// | B' |  =  | b1 b2 b3 b4 b5 | * | B |
// | A' |     | a1 a2 a3 a4 a5 |   | A |
// | 1  |     | 0  0  0  0  1  |   | 1 |
//
// R' = r1*R + r2*G + r3*B + r4*A + r5
// G' = g1*R + g2*G + g3*B + g4*A + g5
// B' = b1*R + b2*G + b3*B + b4*A + b5
// A' = a1*R + a2*G + a3*B + a4*A + a5
SvgFilter _srcInColorFilterToSvg(ui.Color? color) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.colorInterpolationFilters = 'sRGB';
  builder.setFeColorMatrix(
    const <double>[
      0, 0, 0, 0, 1,
      0, 0, 0, 0, 1,
      0, 0, 0, 0, 1,
      0, 0, 0, 1, 0,
    ],
    result: 'destalpha',
  );
  builder.setFeFlood(
    floodColor: colorToCssString(color) ?? '',
    floodOpacity: '1',
    result: 'flood',
  );
  builder.setFeComposite(
    in1: 'flood',
    in2: 'destalpha',
    operator: kOperatorArithmetic,
    k1: 1,
    k2: 0,
    k3: 0,
    k4: 0,
    result: 'comp',
  );
  return builder.build();
}

/// The destination that overlaps the source is composited with the source and
/// replaces the destination. dst-atop	CR = CB*αB*αA+CA*αA*(1-αB)	αR=αA
SvgFilter _dstATopColorFilterToSvg(ui.Color? color) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeFlood(
    floodColor: colorToCssString(color) ?? '',
    floodOpacity: '1',
    result: 'flood',
  );
  builder.setFeComposite(
    in1: 'SourceGraphic',
    in2: 'flood',
    operator: kOperatorAtop,
    result: 'comp',
  );
  return builder.build();
}

SvgFilter _srcOutColorFilterToSvg(ui.Color? color) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeFlood(
    floodColor: colorToCssString(color) ?? '',
    floodOpacity: '1',
    result: 'flood',
  );
  builder.setFeComposite(
    in1: 'flood',
    in2: 'SourceGraphic',
    operator: kOperatorOut,
    result: 'comp',
  );
  return builder.build();
}

SvgFilter _xorColorFilterToSvg(ui.Color? color) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeFlood(
    floodColor: colorToCssString(color) ?? '',
    floodOpacity: '1',
    result: 'flood',
  );
  builder.setFeComposite(
    in1: 'flood',
    in2: 'SourceGraphic',
    operator: kOperatorXor,
    result: 'comp',
  );
  return builder.build();
}

// The source image and color are composited using :
// result = k1 *in*in2 + k2*in + k3*in2 + k4.
SvgFilter _compositeColorFilterToSvg(
    ui.Color? color, double k1, double k2, double k3, double k4) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeFlood(
    floodColor: colorToCssString(color) ?? '',
    floodOpacity: '1',
    result: 'flood',
  );
  builder.setFeComposite(
    in1: 'flood',
    in2: 'SourceGraphic',
    operator: kOperatorArithmetic,
    k1: k1,
    k2: k2,
    k3: k3,
    k4: k4,
    result: 'comp',
  );
  return builder.build();
}

// Porter duff source * destination , keep source alpha.
// First apply color filter to source to change it to [color], then
// composite using multiplication.
SvgFilter _modulateColorFilterToSvg(ui.Color color) {
  final double r = color.red / 255.0;
  final double b = color.blue / 255.0;
  final double g = color.green / 255.0;

  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeColorMatrix(
    <double>[
      0, 0, 0, 0, r,
      0, 0, 0, 0, g,
      0, 0, 0, 0, b,
      0, 0, 0, 1, 0,
    ],
    result: 'recolor',
  );
  builder.setFeComposite(
    in1: 'recolor',
    in2: 'SourceGraphic',
    operator: kOperatorArithmetic,
    k1: 1,
    k2: 0,
    k3: 0,
    k4: 0,
    result: 'comp',
  );
  return builder.build();
}

// Uses feBlend element to blend source image with a color.
SvgFilter _blendColorFilterToSvg(ui.Color? color, SvgBlendMode svgBlendMode,
    {bool swapLayers = false}) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeFlood(
    floodColor: colorToCssString(color) ?? '',
    floodOpacity: '1',
    result: 'flood',
  );
  if (swapLayers) {
    builder.setFeBlend(
      in1: 'SourceGraphic',
      in2: 'flood',
      mode: svgBlendMode.blendMode,
    );
  } else {
    builder.setFeBlend(
      in1: 'flood',
      in2: 'SourceGraphic',
      mode: svgBlendMode.blendMode,
    );
  }
  return builder.build();
}


// Source: https://www.w3.org/TR/SVG11/filters.html#InterfaceSVGFEBlendElement
// These constant names deviate from Dart's camelCase convention on purpose to
// make it easier to search for them in W3 specs and in Chromium sources.
const int SVG_FEBLEND_MODE_UNKNOWN = 0;
const int SVG_FEBLEND_MODE_NORMAL = 1;
const int SVG_FEBLEND_MODE_MULTIPLY = 2;
const int SVG_FEBLEND_MODE_SCREEN = 3;
const int SVG_FEBLEND_MODE_DARKEN = 4;
const int SVG_FEBLEND_MODE_LIGHTEN = 5;
const int SVG_FEBLEND_MODE_OVERLAY = 6;
const int SVG_FEBLEND_MODE_COLOR_DODGE = 7;
const int SVG_FEBLEND_MODE_COLOR_BURN = 8;
const int SVG_FEBLEND_MODE_HARD_LIGHT = 9;
const int SVG_FEBLEND_MODE_SOFT_LIGHT = 10;
const int SVG_FEBLEND_MODE_DIFFERENCE = 11;
const int SVG_FEBLEND_MODE_EXCLUSION = 12;
const int SVG_FEBLEND_MODE_HUE = 13;
const int SVG_FEBLEND_MODE_SATURATION = 14;
const int SVG_FEBLEND_MODE_COLOR = 15;
const int SVG_FEBLEND_MODE_LUMINOSITY = 16;

// Source: https://github.com/chromium/chromium/blob/e1e495b29e1178a451f65980a6c4ae017c34dc94/third_party/blink/renderer/platform/graphics/graphics_types.cc#L55
const String kCompositeClear = 'clear';
const String kCompositeCopy = 'copy';
const String kCompositeSourceOver = 'source-over';
const String kCompositeSourceIn = 'source-in';
const String kCompositeSourceOut = 'source-out';
const String kCompositeSourceAtop = 'source-atop';
const String kCompositeDestinationOver = 'destination-over';
const String kCompositeDestinationIn = 'destination-in';
const String kCompositeDestinationOut = 'destination-out';
const String kCompositeDestinationAtop = 'destination-atop';
const String kCompositeXor = 'xor';
const String kCompositeLighter = 'lighter';

/// Compositing and blending operation in SVG.
///
/// Flutter's [BlendMode] flattens what SVG expresses as two orthogonal
/// properties, a composite operator and blend mode. Instances of this class
/// are returned from [blendModeToSvgEnum] by mapping Flutter's [BlendMode]
/// enum onto the SVG equivalent.
///
/// See also:
///
///  * https://www.w3.org/TR/compositing-1
///  * https://github.com/chromium/chromium/blob/e1e495b29e1178a451f65980a6c4ae017c34dc94/third_party/blink/renderer/platform/graphics/graphics_types.cc#L55
///  * https://github.com/chromium/chromium/blob/e1e495b29e1178a451f65980a6c4ae017c34dc94/third_party/blink/renderer/modules/canvas/canvas2d/base_rendering_context_2d.cc#L725
class SvgBlendMode {
  const SvgBlendMode(this.compositeOperator, this.blendMode);

  /// The name of the SVG composite operator.
  ///
  /// If this mode represents a blend mode, this is set to [kCompositeSourceOver].
  final String compositeOperator;

  /// The identifier of the SVG blend mode.
  ///
  /// This is mode represents a compositing operation, this is set to [SVG_FEBLEND_MODE_UNKNOWN].
  final int blendMode;
}

/// Converts Flutter's [ui.BlendMode] to SVG's <compositing operation, blend mode> pair.
SvgBlendMode? blendModeToSvgEnum(ui.BlendMode? blendMode) {
  if (blendMode == null) {
    return null;
  }
  switch (blendMode) {
    case ui.BlendMode.clear:
      return const SvgBlendMode(kCompositeClear, SVG_FEBLEND_MODE_UNKNOWN);
    case ui.BlendMode.srcOver:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_UNKNOWN);
    case ui.BlendMode.srcIn:
      return const SvgBlendMode(kCompositeSourceIn, SVG_FEBLEND_MODE_UNKNOWN);
    case ui.BlendMode.srcOut:
      return const SvgBlendMode(kCompositeSourceOut, SVG_FEBLEND_MODE_UNKNOWN);
    case ui.BlendMode.srcATop:
      return const SvgBlendMode(kCompositeSourceAtop, SVG_FEBLEND_MODE_UNKNOWN);
    case ui.BlendMode.dstOver:
      return const SvgBlendMode(kCompositeDestinationOver, SVG_FEBLEND_MODE_UNKNOWN);
    case ui.BlendMode.dstIn:
      return const SvgBlendMode(kCompositeDestinationIn, SVG_FEBLEND_MODE_UNKNOWN);
    case ui.BlendMode.dstOut:
      return const SvgBlendMode(kCompositeDestinationOut, SVG_FEBLEND_MODE_UNKNOWN);
    case ui.BlendMode.dstATop:
      return const SvgBlendMode(kCompositeDestinationAtop, SVG_FEBLEND_MODE_UNKNOWN);
    case ui.BlendMode.plus:
      return const SvgBlendMode(kCompositeLighter, SVG_FEBLEND_MODE_UNKNOWN);
    case ui.BlendMode.src:
      return const SvgBlendMode(kCompositeCopy, SVG_FEBLEND_MODE_UNKNOWN);
    case ui.BlendMode.xor:
      return const SvgBlendMode(kCompositeXor, SVG_FEBLEND_MODE_UNKNOWN);
    case ui.BlendMode.multiply:
    // Falling back to multiply, ignoring alpha channel.
    // TODO(ferhat): only used for debug, find better fallback for web.
    case ui.BlendMode.modulate:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_MULTIPLY);
    case ui.BlendMode.screen:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_SCREEN);
    case ui.BlendMode.overlay:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_OVERLAY);
    case ui.BlendMode.darken:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_DARKEN);
    case ui.BlendMode.lighten:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_LIGHTEN);
    case ui.BlendMode.colorDodge:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_COLOR_DODGE);
    case ui.BlendMode.colorBurn:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_COLOR_BURN);
    case ui.BlendMode.hardLight:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_HARD_LIGHT);
    case ui.BlendMode.softLight:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_SOFT_LIGHT);
    case ui.BlendMode.difference:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_DIFFERENCE);
    case ui.BlendMode.exclusion:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_EXCLUSION);
    case ui.BlendMode.hue:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_HUE);
    case ui.BlendMode.saturation:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_SATURATION);
    case ui.BlendMode.color:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_COLOR);
    case ui.BlendMode.luminosity:
      return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_LUMINOSITY);
    default:
      assert(
        false,
        'Flutter Web does not support the blend mode: $blendMode',
      );

    return const SvgBlendMode(kCompositeSourceOver, SVG_FEBLEND_MODE_NORMAL);
  }
}


SvgFilter svgMaskFilterFromImageAndBlendMode(
    String imageUrl, ui.BlendMode blendMode, double width, double height) {
  final SvgFilter svgFilter;
  switch (blendMode) {
    case ui.BlendMode.src:
      svgFilter = _srcImageToSvg(imageUrl, width, height);
      break;
    case ui.BlendMode.srcIn:
    case ui.BlendMode.srcATop:
      svgFilter = _srcInImageToSvg(imageUrl, width, height);
      break;
    case ui.BlendMode.srcOut:
      svgFilter = _srcOutImageToSvg(imageUrl, width, height);
      break;
    case ui.BlendMode.xor:
      svgFilter = _xorImageToSvg(imageUrl, width, height);
      break;
    case ui.BlendMode.plus:
      // Porter duff source + destination.
      svgFilter = _compositeImageToSvg(imageUrl, 0, 1, 1, 0, width, height);
      break;
    case ui.BlendMode.modulate:
      // Porter duff source * destination but preserves alpha.
      svgFilter = _modulateImageToSvg(imageUrl, width, height);
      break;
    case ui.BlendMode.overlay:
      // Since overlay is the same as hard-light by swapping layers,
      // pass hard-light blend function.
      svgFilter = _blendImageToSvg(
        imageUrl,
        blendModeToSvgEnum(ui.BlendMode.hardLight)!,
        width,
        height,
        swapLayers: true,
      );
      break;
    // Several of the filters below (although supported) do not render the
    // same (close but not exact) as native flutter when used as blend mode
    // for a background-image with a background color. They only look
    // identical when feBlend is used within an svg filter definition.
    //
    // Saturation filter uses destination when source is transparent.
    // cMax = math.max(r, math.max(b, g));
    // cMin = math.min(r, math.min(b, g));
    // delta = cMax - cMin;
    // lightness = (cMax + cMin) / 2.0;
    // saturation = delta / (1.0 - (2 * lightness - 1.0).abs());
    case ui.BlendMode.saturation:
    case ui.BlendMode.colorDodge:
    case ui.BlendMode.colorBurn:
    case ui.BlendMode.hue:
    case ui.BlendMode.color:
    case ui.BlendMode.luminosity:
    case ui.BlendMode.multiply:
    case ui.BlendMode.screen:
    case ui.BlendMode.darken:
    case ui.BlendMode.lighten:
    case ui.BlendMode.hardLight:
    case ui.BlendMode.softLight:
    case ui.BlendMode.difference:
    case ui.BlendMode.exclusion:
      svgFilter = _blendImageToSvg(
          imageUrl, blendModeToSvgEnum(blendMode)!, width, height);
      break;
    case ui.BlendMode.dst:
    case ui.BlendMode.dstATop:
    case ui.BlendMode.dstIn:
    case ui.BlendMode.dstOut:
    case ui.BlendMode.dstOver:
    case ui.BlendMode.clear:
    case ui.BlendMode.srcOver:
      throw UnsupportedError(
          'Invalid svg filter request for blend-mode $blendMode');
  }
  return svgFilter;
}

// The color matrix for feColorMatrix element changes colors based on
// the following:
//
// | R' |     | r1 r2 r3 r4 r5 |   | R |
// | G' |     | g1 g2 g3 g4 g5 |   | G |
// | B' |  =  | b1 b2 b3 b4 b5 | * | B |
// | A' |     | a1 a2 a3 a4 a5 |   | A |
// | 1  |     | 0  0  0  0  1  |   | 1 |
//
// R' = r1*R + r2*G + r3*B + r4*A + r5
// G' = g1*R + g2*G + g3*B + g4*A + g5
// B' = b1*R + b2*G + b3*B + b4*A + b5
// A' = a1*R + a2*G + a3*B + a4*A + a5
SvgFilter _srcInImageToSvg(String imageUrl, double width, double height) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeColorMatrix(
    const <double>[
      0, 0, 0, 0, 1,
      0, 0, 0, 0, 1,
      0, 0, 0, 0, 1,
      0, 0, 0, 1, 0,
    ],
    result: 'destalpha',
  );
  builder.setFeImage(
    href: imageUrl,
    result: 'image',
    width: width,
    height: height,
  );
  builder.setFeComposite(
    in1: 'image',
    in2: 'destalpha',
    operator: kOperatorArithmetic,
    k1: 1,
    k2: 0,
    k3: 0,
    k4: 0,
    result: 'comp',
  );
  return builder.build();
}

SvgFilter _srcImageToSvg(String imageUrl, double width, double height) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeImage(
    href: imageUrl,
    result: 'comp',
    width: width,
    height: height,
  );
  return builder.build();
}

SvgFilter _srcOutImageToSvg(String imageUrl, double width, double height) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeImage(
    href: imageUrl,
    result: 'image',
    width: width,
    height: height,
  );
  builder.setFeComposite(
    in1: 'image',
    in2: 'SourceGraphic',
    operator: kOperatorOut,
    result: 'comp',
  );
  return builder.build();
}

SvgFilter _xorImageToSvg(String imageUrl, double width, double height) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeImage(
    href: imageUrl,
    result: 'image',
    width: width,
    height: height,
  );
  builder.setFeComposite(
    in1: 'image',
    in2: 'SourceGraphic',
    operator: kOperatorXor,
    result: 'comp',
  );
  return builder.build();
}

// The source image and color are composited using :
// result = k1 *in*in2 + k2*in + k3*in2 + k4.
SvgFilter _compositeImageToSvg(String imageUrl, double k1, double k2, double k3,
    double k4, double width, double height) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeImage(
    href: imageUrl,
    result: 'image',
    width: width,
    height: height,
  );
  builder.setFeComposite(
    in1: 'image',
    in2: 'SourceGraphic',
    operator: kOperatorArithmetic,
    k1: k1,
    k2: k2,
    k3: k3,
    k4: k4,
    result: 'comp',
  );
  return builder.build();
}

// Porter duff source * destination , keep source alpha.
// First apply color filter to source to change it to [color], then
// composite using multiplication.
SvgFilter _modulateImageToSvg(String imageUrl, double width, double height) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeImage(
    href: imageUrl,
    result: 'image',
    width: width,
    height: height,
  );
  builder.setFeComposite(
    in1: 'image',
    in2: 'SourceGraphic',
    operator: kOperatorArithmetic,
    k1: 1,
    k2: 0,
    k3: 0,
    k4: 0,
    result: 'comp',
  );
  return builder.build();
}

// Uses feBlend element to blend source image with a color.
SvgFilter _blendImageToSvg(
    String imageUrl, SvgBlendMode svgBlendMode, double width, double height,
    {bool swapLayers = false}) {
  final SvgFilterBuilder builder = SvgFilterBuilder(targetType: SvgFilterTargetType.html);
  builder.setFeImage(
    href: imageUrl,
    result: 'image',
    width: width,
    height: height,
  );
  if (swapLayers) {
    builder.setFeBlend(
      in1: 'SourceGraphic',
      in2: 'image',
      mode: svgBlendMode.blendMode,
    );
  } else {
    builder.setFeBlend(
      in1: 'image',
      in2: 'SourceGraphic',
      mode: svgBlendMode.blendMode,
    );
  }
  return builder.build();
}
