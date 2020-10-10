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
    String? cssBlendMode = {
      ui.BlendMode.screen: 'screen',
      ui.BlendMode.overlay: 'overlay',
      ui.BlendMode.darken: 'darken',
      ui.BlendMode.lighten: 'lighten',
      ui.BlendMode.colorDodge: 'color-dodge',
      ui.BlendMode.colorBurn: 'color-burn',
      ui.BlendMode.hardLight: 'hard-light',
      ui.BlendMode.softLight: 'soft-light',
      ui.BlendMode.difference: 'difference',
      ui.BlendMode.exclusion: 'exclusion',
      ui.BlendMode.multiply: 'multiply',
      ui.BlendMode.hue: 'hue',
      ui.BlendMode.saturation: 'saturation',
      ui.BlendMode.color: 'color',
      ui.BlendMode.luminosity: 'luminosity',
    }[blendMode];
    if (cssBlendMode == null) {
      switch (blendMode) {
        case ui.BlendMode.clear:
        case ui.BlendMode.dstOut:
        case ui.BlendMode.srcOut:
          childContainer?.style.visibility = 'hidden';
          return;
        // TODO(flutter-web): Additional blend modes for ShaderMask
        case ui.BlendMode.dst:
        case ui.BlendMode.dstIn:
        case ui.BlendMode.src:
        case ui.BlendMode.srcOver:
        case ui.BlendMode.dstOver:
        case ui.BlendMode.srcIn:
        case ui.BlendMode.srcATop:
        case ui.BlendMode.dstATop:
        case ui.BlendMode.xor:
        case ui.BlendMode.plus:
        case ui.BlendMode.modulate:
          break;
        default:
          // Handled above
          break;
      }
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

    _shaderElement = html.Element.tag('flt-mask');
    rootElement!.append(_shaderElement!);
    if (cssBlendMode != null) {
      _shaderElement!.style.mixBlendMode = cssBlendMode;
    }

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
      final stops = <String>[];
      for (int i = 0; i < gradient.colors.length; i++) {
        final stop = gradient.colorStops?[i];
        final color = gradient.colors[i];
        stops.add('${colorToCssString(color)}' +
            (stop != null ? ' ${(stop * 100).round()}%' : ''));
      }
      _shaderElement!.style.background =
          'linear-gradient(${angle}deg, ${stops.join(', ')})';
    } else if (gradient is GradientRadial) {
      // TODO(flutter-web): Use the radius property of radial gradients.
      // Not sure what effect it is supposed to have and they seem to be working
      // well enough without it.
      final stops = <String>[];
      for (int i = 0; i < gradient.colors.length; i++) {
        final stop = gradient.colorStops?[i];
        final color = gradient.colors[i];
        stops.add('${colorToCssString(color)}' +
            (stop != null ? ' ${(stop * 100).round()}%' : ''));
      }
      _shaderElement!.style.background =
          'radial-gradient(circle at ${gradient.center.dx}px ${gradient.center.dy}px, ${stops.join(', ')}';
    }
    _shaderElement!.style
      ..left = '${maskRect.left}px'
      ..top = '${maskRect.top}px'
      ..width = '${maskRect.width}px'
      ..height = '${maskRect.height}px'
      ..position = 'absolute';
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
