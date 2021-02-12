// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.12
part of engine;

/// A surface that applies a shader to its children.
///
/// Currently there are 2 types of shaders:
///   - Gradients
///   - ImageShader
///
/// Gradients
///   The gradients can be applied to the child tree by rendering the gradient
///   into an image and referencing the image in an svg filter to apply
///   to DOM tree.
///
class PersistedShaderMask extends PersistedContainerSurface
    implements ui.ShaderMaskEngineLayer {
  PersistedShaderMask(
    PersistedShaderMask? oldLayer,
    this.shader,
    this.maskRect,
    this.blendMode,
  ) : super(oldLayer);

  html.Element? _childContainer;
  final ui.Shader shader;
  final ui.Rect maskRect;
  final ui.BlendMode blendMode;
  html.Element? _shaderElement;

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
    _shaderElement?.remove();
    _shaderElement = null;
    if (shader is ui.Gradient) {
      element.style
        ..width = '${maskRect.width}px'
        ..height = '${maskRect.height}px';
      _applyGradientShader();
      return;
    }
    // TODO: Implement _applyImageShader();
    throw Exception('Shader type not supported for ShaderMask');
  }

  void _applyGradientShader() {
    if (shader is EngineGradient) {
      EngineGradient gradientShader = shader as EngineGradient;
      final String imageUrl =
          gradientShader.createImageBitmap(maskRect, 1, true) as String;
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

      String code =
          svgMaskFilterFromImageAndBlendMode(imageUrl, blendModeTemp)!;

      _shaderElement =
          html.Element.html(code, treeSanitizer: _NullTreeSanitizer());
      rootElement!.append(_shaderElement!);
      rootElement!.style.filter = 'url(#_fmf${_maskFilterIdCounter}';
    }
  }

  @override
  void update(PersistedShaderMask oldSurface) {
    super.update(oldSurface);
    if (shader != oldSurface.shader ||
        maskRect != oldSurface.maskRect ||
        blendMode != oldSurface.blendMode) {
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
      throw UnsupportedError(
          'Invalid svg filter request for blend-mode $blendMode');
  }
  return svgFilter;
}

int _maskFilterIdCounter = 0;

String _svgFilterWrapper(String content) {
  _maskFilterIdCounter++;
  return '<svg width="0" height="0" xmlns:xlink="http://www.w3.org/1999/xlink">'
          '<filter id="_fmf$_maskFilterIdCounter" '
          'filterUnits="objectBoundingBox">' +
      content +
      '</filter></svg>';
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
String _srcInImageToSvg(String imageUrl) {
  return _svgFilterWrapper(
    '<feColorMatrix values="0 0 0 0 1 ' // Ignore input, set it to absolute.
    '0 0 0 0 1 '
    '0 0 0 0 1 '
    '0 0 0 1 0" result="destalpha"/>' // Just take alpha channel of destination
    '<feImage href="$imageUrl" result="image">'
    '</feImage>'
    '<feComposite in="image" in2="destalpha" '
    'operator="arithmetic" k1="1" k2="0" k3="0" k4="0" result="comp">'
    '</feComposite>',
  );
}

String _srcOutImageToSvg(String imageUrl) {
  return _svgFilterWrapper('<feImage href="$imageUrl" result="image">'
      '</feImage>'
      '<feComposite in="image" in2="SourceGraphic" operator="out" result="comp">'
      '</feComposite>');
}

String _xorImageToSvg(String imageUrl) {
  return _svgFilterWrapper('<feImage href="$imageUrl" result="image">'
      '</feImage>'
      '<feComposite in="image" in2="SourceGraphic" operator="xor" result="comp">'
      '</feComposite>');
}

// The source image and color are composited using :
// result = k1 *in*in2 + k2*in + k3*in2 + k4.
String _compositeImageToSvg(
    String imageUrl, double k1, double k2, double k3, double k4) {
  return _svgFilterWrapper(
    '<feImage href="$imageUrl" result="image">'
    '</feImage>'
    '<feComposite in="image" in2="SourceGraphic" '
    'operator="arithmetic" k1="$k1" k2="$k2" k3="$k3" k4="$k4" result="comp">'
    '</feComposite>',
  );
}

// Porter duff source * destination , keep source alpha.
// First apply color filter to source to change it to [color], then
// composite using multiplication.
String _modulateImageToSvg(String imageUrl) {
  return _svgFilterWrapper(
    '<feImage href="$imageUrl" result="image">'
    '</feImage>'
    '<feComposite in="image" in2="SourceGraphic" '
    'operator="arithmetic" k1="1" k2="0" k3="0" k4="0" result="comp">'
    '</feComposite>',
  );
}

// Uses feBlend element to blend source image with a color.
String _blendImageToSvg(String imageUrl, String? feBlend,
    {bool swapLayers = false}) {
  return _svgFilterWrapper(
    '<feImage href="$imageUrl" result="image"></feImage>' +
        (swapLayers
            ? '<feBlend in="SourceGraphic" in2="image" mode="$feBlend"/>'
            : '<feBlend in="image" in2="SourceGraphic" mode="$feBlend"/>'),
  );
}
