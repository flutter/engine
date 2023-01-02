// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../browser_detection.dart';
import '../dom.dart';
import '../embedder.dart';
import '../svg_filter.dart';
import 'shaders/shader.dart';
import 'surface.dart';

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
    PersistedShaderMask? super.oldLayer,
    this.shader,
    this.maskRect,
    this.blendMode,
    this.filterQuality,
  );

  DomElement? _childContainer;
  final ui.Shader shader;
  final ui.Rect maskRect;
  final ui.BlendMode blendMode;
  final ui.FilterQuality filterQuality;
  DomElement? _shaderElement;
  final bool isWebKit = browserEngine == BrowserEngine.webkit;

  @override
  void adoptElements(PersistedShaderMask oldSurface) {
    super.adoptElements(oldSurface);
    _childContainer = oldSurface._childContainer;
    _shaderElement = oldSurface._shaderElement;
    oldSurface._childContainer = null;
    oldSurface._shaderElement = null;
  }

  @override
  DomElement? get childContainer => _childContainer;

  @override
  void discard() {
    super.discard();
    flutterViewEmbedder.removeResource(_shaderElement);
    _shaderElement = null;
    // Do not detach the child container from the root. It is permanently
    // attached. The elements are reused together and are detached from the DOM
    // together.
    _childContainer = null;
  }

  @override
  void preroll(PrerollSurfaceContext prerollContext) {
    ++prerollContext.activeShaderMaskCount;
    super.preroll(prerollContext);
    --prerollContext.activeShaderMaskCount;
  }

  @override
  DomElement createElement() {
    final DomElement element = defaultCreateElement('flt-shader-mask');
    final DomElement container = createDomElement('flt-mask-interior');
    container.style.position = 'absolute';
    _childContainer = container;
    element.append(_childContainer!);
    return element;
  }

  @override
  void apply() {
    flutterViewEmbedder.removeResource(_shaderElement);
    _shaderElement = null;
    if (shader is ui.Gradient) {
      rootElement!.style
        ..left = '${maskRect.left}px'
        ..top = '${maskRect.top}px'
        ..width = '${maskRect.width}px'
        ..height = '${maskRect.height}px';
      _childContainer!.style
        ..left = '${-maskRect.left}px'
        ..top = '${-maskRect.top}px';
      // Prevent ShaderMask from failing inside animations that size
      // area to empty.
      if (maskRect.width > 0 && maskRect.height > 0) {
        _applyGradientShader();
      }
      return;
    }
    // TODO(ferhat): Implement _applyImageShader();
    throw Exception('Shader type not supported for ShaderMask');
  }

  void _applyGradientShader() {
    if (shader is EngineGradient) {
      final EngineGradient gradientShader = shader as EngineGradient;

      // The gradient shader's bounds are in the context of the element itself,
      // rather than the global position, so translate it back to the origin.
      final ui.Rect translatedRect =
          maskRect.translate(-maskRect.left, -maskRect.top);
      final String imageUrl =
          gradientShader.createImageBitmap(translatedRect, 1, true) as String;
      ui.BlendMode blendModeTemp = blendMode;
      switch (blendModeTemp) {
        case ui.BlendMode.clear:
        case ui.BlendMode.dstOut:
        case ui.BlendMode.srcOut:
          childContainer?.style.visibility = 'hidden';
          return;
        case ui.BlendMode.dst:
        case ui.BlendMode.dstIn:
          // Noop. Should render existing destination.
          rootElement!.style.filter = '';
          return;
        case ui.BlendMode.srcOver:
          // Uses source filter color.
          // Since we don't have a size, we can't use background color.
          // Use svg filter srcIn instead.
          blendModeTemp = ui.BlendMode.srcIn;
          break;
        case ui.BlendMode.src:
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

      final SvgFilter svgFilter = svgMaskFilterFromImageAndBlendMode(
          imageUrl, blendModeTemp, maskRect.width, maskRect.height);
      _shaderElement = svgFilter.element;
      if (isWebKit) {
        _childContainer!.style.filter = 'url(#${svgFilter.id})';
      } else {
        rootElement!.style.filter = 'url(#${svgFilter.id})';
      }
      flutterViewEmbedder.addResource(_shaderElement!);
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
