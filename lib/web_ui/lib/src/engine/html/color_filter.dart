// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.10
part of engine;

/// A surface that applies an [ColorFilter] to its children.
class PersistedColorFilter extends PersistedContainerSurface
    implements ui.ColorFilterEngineLayer {
  PersistedColorFilter(PersistedColorFilter? oldLayer, this.filter) : super(oldLayer);

  final ui.ColorFilter filter;
  html.Element? _filterElement;

  @override
  html.Element createElement() {
    return defaultCreateElement('flt-color-filter');
  }

  @override
  void apply() {
    rootElement!.style.background = '#FF0000';
    if (_filterElement != null) {
      _filterElement?.remove();
    }
    final EngineColorFilter? engineValue = filter as EngineColorFilter?;
    if (engineValue != null) {
      ui.Color filterColor = engineValue._color!;
      ui.BlendMode? colorFilterBlendMode = engineValue._blendMode;
      if (colorFilterBlendMode != null) {
        String? svgFilter = svgFilterFromBlendMode(filterColor,
            colorFilterBlendMode);
        if (svgFilter != null) {
          _filterElement =
              html.Element.html(svgFilter, treeSanitizer: _NullTreeSanitizer());
          rootElement!.append(_filterElement!);
          rootElement!.style.filter = 'url(#_fcf${_filterIdCounter})';
          if (colorFilterBlendMode == ui.BlendMode.saturation) {
            rootElement!.style.backgroundColor = colorToCssString(filterColor);
          }
          return;
        }
      }
    }
    rootElement!.style.backgroundColor = '';
  }

  @override
  void update(PersistedColorFilter oldSurface) {
    super.update(oldSurface);

    if (oldSurface.filter != filter) {
      apply();
    }
  }
}

String? svgFilterFromBlendMode(ui.Color? filterColor,
    ui.BlendMode colorFilterBlendMode) {
  String? svgFilter;
  switch (colorFilterBlendMode) {
    case ui.BlendMode.srcIn:
    case ui.BlendMode.srcATop:
      svgFilter = _srcInColorFilterToSvg(filterColor);
      break;
    case ui.BlendMode.srcOut:
      svgFilter = _srcOutColorFilterToSvg(filterColor);
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
      svgFilter = _blendColorFilterToSvg(filterColor, 'hard-light',
          swapLayers: true);
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
      svgFilter = _blendColorFilterToSvg(filterColor,
          _stringForBlendMode(colorFilterBlendMode));
      break;
    default:
      break;
  }
  return svgFilter;
}