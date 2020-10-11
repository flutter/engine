// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.10
part of engine;

/// A surface that applies a [Shader] to its children.
class PersistedShaderMask extends PersistedContainerSurface
    implements ui.ShaderMaskEngineLayer {
  PersistedShaderMask(
    PersistedShaderMask? oldLayer,
    this.shader,
    this.maskRect,
    this.blendMode,
  ) : super(oldLayer);

  @override
  html.Element? get childContainer => _childContainer;

  /// The dedicated child container element that's separate from the
  /// [rootElement] is used to compensate for the coordinate system shift
  /// introduced by the [rootElement] translation.
  html.Element? _childContainer;

  final ui.Shader shader;
  final ui.Rect maskRect;
  final ui.BlendMode blendMode;
  html.Element? _shaderElement;
  bool containerVisible = true;

  @override
  void adoptElements(PersistedShaderMask oldSurface) {
    super.adoptElements(oldSurface);
    _childContainer = oldSurface._childContainer;
    _shaderElement = oldSurface._shaderElement;
    oldSurface._childContainer = null;
    oldSurface._shaderElement = null;
  }

  @override
  void discard() {
    super.discard();
    // Do not detach the child container from the root. It is permanently
    // attached. The elements are reused together and are detached from the DOM
    // together.
    _childContainer = null;
  }

  @override
  html.Element createElement() {
    html.Element element = defaultCreateElement('flt-shader-mask');
    html.Element container = html.Element.tag('flt-mask-interior');
    container.style.position = 'absolute';
    _childContainer = container;
    element.append(_childContainer!);
    return element;
  }

  @override
  void apply() {
    if (_shaderElement != null) {
      _shaderElement?.remove();
    }

    // TODO(flutter-web): Support non-gradient shaders for ShaderMask.
    if (!(shader is ui.Gradient)) {
      return;
    }
    final gradient = shader as ui.Gradient;
    // TODO(flutter-web): Support conical and sweep gradients for ShaderMask.
    if (gradient is GradientConical || gradient is GradientSweep) {
      return;
    }

    String inputImageSvg;
    if (gradient is GradientLinear) {
      // TODO(flutter-web): More accurate linear gradients for ShaderMask.
      // This seems to be accurate enough, but currently it only uses the angle.
      _FastMatrix64? matrix4 = gradient.matrix4;
      double fromX = gradient.from.dx;
      double fromY = gradient.from.dy;
      double toX = gradient.to.dx;
      double toY = gradient.to.dy;
      if (matrix4 != null) {
        final centerX = (gradient.from.dx + gradient.to.dx) / 2.0;
        final centerY = (gradient.from.dy + gradient.to.dy) / 2.0;
        matrix4.transform(
          gradient.from.dx - centerX,
          gradient.from.dy - centerY,
        );
        fromX = matrix4.transformedX + centerX;
        fromY = matrix4.transformedY + centerY;
        matrix4.transform(gradient.to.dx - centerX, gradient.to.dy - centerY);
        toX = matrix4.transformedX + centerX;
        toY = matrix4.transformedY + centerY;
      }
      final angle = math.atan2(toY - fromY, toX - fromX) * (180 / math.pi) + 90;
      final lines = [
        '<svg xmlns="http://www.w3.org/2000/svg" x=\'${maskRect.left}px\' y=\'${maskRect.top}px\' width=\'${maskRect.width}px\' height=\'${maskRect.height}px\'>'
            '<defs><linearGradient id=\'gradient\' gradientTransform=\'rotate($angle)\'>'
      ];
      for (int i = 0; i < gradient.colors.length; i++) {
        final stop =
            gradient.colorStops?[i] ?? i / (gradient.colors.length - 1);
        final color = gradient.colors[i];
        lines.add(
            '<stop stop-color=\'${colorToCssString(color)}\' offset=\'${(stop * 100).round()}%\'/>');
      }
      lines.add('</linearGradient></defs><rect x=\'0%\' y=\'0%\' '
          'width=\'100%\' height=\'100%\' fill=\'url(#gradient)\'></rect></svg>');
      inputImageSvg = lines.join('');
    } else if (gradient is GradientRadial) {
      final lines = [
        '<svg xmlns="http://www.w3.org/2000/svg" x=\'${maskRect.left}px\' y=\'${maskRect.top}px\' width=\'${maskRect.width}px\' height=\'${maskRect.height}px\'><defs>'
            '<radialGradient id=\'gradient\' '
            'cx=\'${maskRect.left + gradient.center.dx}px\' cy=\'${maskRect.top + gradient.center.dy}px\' r=\'${gradient.radius}px\'>'
      ];
      for (int i = 0; i < gradient.colors.length; i++) {
        final stop =
            gradient.colorStops?[i] ?? i / (gradient.colors.length - 1);
        final color = gradient.colors[i];
        lines.add(
            '<stop stop-color=\'${colorToCssString(color)}\' offset=\'${(stop * 100).round()}%\'/>');
      }
      lines.add(
          '</radialGradient></defs><rect x=\'0%\' y=\'0%\' '
          'width=\'100%\' height=\'100%\' fill=\'url(#gradient)\'></rect></svg>');
      inputImageSvg = lines.join('');
    } else {
      // unreachable, just added to make analyzer happy
      return;
    }
    // This is required since Firefox doesn't support feImage pointing to
    // a fragment of the SVG yet.
    // https://bugzilla.mozilla.org/show_bug.cgi?id=1538554
    inputImageSvg = Uri.encodeComponent(inputImageSvg);
    String imageUrl = 'data:image/svg+xml;charset=utf-8,$inputImageSvg';

    ui.BlendMode blendModeTemp = blendMode;
    switch (blendModeTemp) {
      case ui.BlendMode.clear:
      case ui.BlendMode.dstOut:
      case ui.BlendMode.srcOut:
        childContainer?.style.visibility = 'hidden';
        return;
      case ui.BlendMode.dst:
      case ui.BlendMode.dstIn:
        // Noop.
        return;
      case ui.BlendMode.src:
      case ui.BlendMode.srcOver:
        // Uses source filter color.
        // Since we don't have a size, we can't use background color.
        // Use svg filter srcIn instead.
        blendModeTemp = ui.BlendMode.srcIn;
        break;
      case ui.BlendMode.dstOver:
      case ui.BlendMode.srcIn:
      case ui.BlendMode.srcATop:
      case ui.BlendMode.dstATop:
      case ui.BlendMode.xor:
      case ui.BlendMode.plus:
      case ui.BlendMode.modulate:
      case ui.BlendMode.screen:
      case ui.BlendMode.overlay:
      case ui.BlendMode.darken:
      case ui.BlendMode.lighten:
      case ui.BlendMode.colorDodge:
      case ui.BlendMode.colorBurn:
      case ui.BlendMode.hardLight:
      case ui.BlendMode.softLight:
      case ui.BlendMode.difference:
      case ui.BlendMode.exclusion:
      case ui.BlendMode.multiply:
      case ui.BlendMode.hue:
      case ui.BlendMode.saturation:
      case ui.BlendMode.color:
      case ui.BlendMode.luminosity:
        break;
    }

    String code = svgMaskFilterFromImageAndBlendMode(imageUrl, blendModeTemp)!;

    _shaderElement =
        html.Element.html(code, treeSanitizer: _NullTreeSanitizer());
    rootElement!.append(_shaderElement!);
    rootElement!.style.filter = 'url(#_fmf${_maskFilterIdCounter}';
  }

  @override
  void update(PersistedShaderMask oldSurface) {
    super.update(oldSurface);

    if (oldSurface.shader != shader ||
        oldSurface.maskRect != maskRect ||
        oldSurface.blendMode != blendMode) {
      apply();
    }
  }
}

String? svgMaskFilterFromImageAndBlendMode(
    String imageUrl, ui.BlendMode blendMode) {
  String? svgFilter;
  switch (blendMode) {
    case ui.BlendMode.srcIn:
    case ui.BlendMode.srcATop:
      svgFilter = _srcInImageToSvg(imageUrl);
      break;
    case ui.BlendMode.srcOut:
      svgFilter = _srcOutImageToSvg(imageUrl);
      break;
    case ui.BlendMode.xor:
      svgFilter = _xorImageToSvg(imageUrl);
      break;
    case ui.BlendMode.plus:
      // Porter duff source + destination.
      svgFilter = _compositeImageToSvg(imageUrl, 0, 1, 1, 0);
      break;
    case ui.BlendMode.modulate:
      // Porter duff source * destination but preserves alpha.
      svgFilter = _modulateImageToSvg(imageUrl);
      break;
    case ui.BlendMode.overlay:
      // Since overlay is the same as hard-light by swapping layers,
      // pass hard-light blend function.
      svgFilter = _blendImageToSvg(imageUrl, 'hard-light', swapLayers: true);
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
    case ui.BlendMode.overlay:
    case ui.BlendMode.darken:
    case ui.BlendMode.lighten:
    case ui.BlendMode.colorDodge:
    case ui.BlendMode.colorBurn:
    case ui.BlendMode.hardLight:
    case ui.BlendMode.softLight:
    case ui.BlendMode.difference:
    case ui.BlendMode.exclusion:
      svgFilter = _blendImageToSvg(imageUrl, _stringForBlendMode(blendMode));
      break;
    case ui.BlendMode.src:
    case ui.BlendMode.dst:
    case ui.BlendMode.dstATop:
    case ui.BlendMode.dstIn:
    case ui.BlendMode.dstOut:
    case ui.BlendMode.dstOver:
    case ui.BlendMode.clear:
    case ui.BlendMode.srcOver:
      assert(
          false,
          'Invalid svg filter request for blend-mode '
          '$blendMode');
      break;
  }
  return svgFilter;
}

int _maskFilterIdCounter = 0;

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
String _srcInImageToSvg(String imageUrl) {
  _maskFilterIdCounter += 1;
  return '<svg width="0" height="0" xmlns:xlink="http://www.w3.org/1999/xlink">'
      '<filter id="_fmf$_maskFilterIdCounter" '
      'filterUnits="objectBoundingBox" x="0%" y="0%" width="100%" height="100%">'
      '<feColorMatrix values="0 0 0 0 1 ' // Ignore input, set it to absolute.
      '0 0 0 0 1 '
      '0 0 0 0 1 '
      '0 0 0 1 0" result="destalpha"/>' // Just take alpha channel of destination
      '<feImage xlink:href="$imageUrl" result="image">'
      '</feImage>'
      '<feComposite in="image" in2="destalpha" '
      'operator="arithmetic" k1="1" k2="0" k3="0" k4="0" result="comp">'
      '</feComposite>'
      '</filter></svg>';
}

String _srcOutImageToSvg(String imageUrl) {
  _maskFilterIdCounter += 1;
  return '<svg width="0" height="0" xmlns:xlink="http://www.w3.org/1999/xlink">'
      '<filter id="_fmf$_maskFilterIdCounter" '
      'filterUnits="objectBoundingBox" x="0%" y="0%" width="100%" height="100%">'
      '<feImage xlink:href="$imageUrl" result="image">'
      '</feImage>'
      '<feComposite in="image" in2="SourceGraphic" operator="out" result="comp">'
      '</feComposite>'
      '</filter></svg>';
}

String _xorImageToSvg(String imageUrl) {
  _maskFilterIdCounter += 1;
  return '<svg width="0" height="0" xmlns:xlink="http://www.w3.org/1999/xlink">'
      '<filter id="_fmf$_maskFilterIdCounter" '
      'filterUnits="objectBoundingBox" x="0%" y="0%" width="100%" height="100%">'
      '<feImage xlink:href="$imageUrl" result="image">'
      '</feImage>'
      '<feComposite in="image" in2="SourceGraphic" operator="xor" result="comp">'
      '</feComposite>'
      '</filter></svg>';
}

// The source image and color are composited using :
// result = k1 *in*in2 + k2*in + k3*in2 + k4.
String _compositeImageToSvg(
    String imageUrl, double k1, double k2, double k3, double k4) {
  _maskFilterIdCounter += 1;
  return '<svg width="0" height="0" xmlns:xlink="http://www.w3.org/1999/xlink">'
      '<filter id="_fmf$_maskFilterIdCounter" '
      'filterUnits="objectBoundingBox" x="0%" y="0%" width="100%" height="100%">'
      '<feImage xlink:href="$imageUrl" result="image">'
      '</feImage>'
      '<feComposite in="image" in2="SourceGraphic" '
      'operator="arithmetic" k1="$k1" k2="$k2" k3="$k3" k4="$k4" result="comp">'
      '</feComposite>'
      '</filter></svg>';
}

// Porter duff source * destination , keep source alpha.
// First apply color filter to source to change it to [color], then
// composite using multiplication.
String _modulateImageToSvg(String imageUrl) {
  _maskFilterIdCounter += 1;
  return '<svg width="0" height="0" xmlns:xlink="http://www.w3.org/1999/xlink">'
      '<filter id="_fmf$_maskFilterIdCounter" '
      'filterUnits="objectBoundingBox" x="0%" y="0%" width="100%" height="100%">'
      '<feImage xlink:href="$imageUrl" result="image">'
      '</feImage>'
      '<feComposite in="image" in2="SourceGraphic" '
      'operator="arithmetic" k1="1" k2="0" k3="0" k4="0" result="comp">'
      '</feComposite>'
      '</filter></svg>';
}

// Uses feBlend element to blend source image with a color.
String _blendImageToSvg(String imageUrl, String? feBlend,
    {bool swapLayers = false}) {
  _maskFilterIdCounter += 1;
  return '<svg width="0" height="0" xmlns:xlink="http://www.w3.org/1999/xlink">'
          '<filter id="_fmf$_maskFilterIdCounter" filterUnits="objectBoundingBox" '
          'x="0%" y="0%" width="100%" height="100%">'
          '<feImage xlink:href="$imageUrl" result="image">'
          '</feImage>' +
      (swapLayers
          ? '<feBlend in="SourceGraphic" in2="image" mode="$feBlend"/>'
          : '<feBlend in="image" in2="SourceGraphic" mode="$feBlend"/>') +
      '</filter></svg>';
}
