// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine/renderer.dart';
import 'package:ui/ui.dart' as ui;

enum ColorFilterType {
  mode,
  matrix,
  linearToSrgbGamma,
  srgbToLinearGamma,
}

/// A description of a color filter to apply when drawing a shape or compositing
/// a layer with a particular [Paint]. A color filter is a function that takes
/// two colors, and outputs one color. When applied during compositing, it is
/// independently applied to each pixel of the layer being drawn before the
/// entire layer is merged with the destination.
///
/// Instances of this class are used with [Paint.colorFilter] on [Paint]
/// objects.
class EngineColorFilter implements ui.ColorFilter {
  /// Creates a color filter that applies the blend mode given as the second
  /// argument. The source color is the one given as the first argument, and the
  /// destination color is the one from the layer being composited.
  ///
  /// The output of this filter is then composited into the background according
  /// to the [Paint.blendMode], using the output of this filter as the source
  /// and the background as the destination.
  const EngineColorFilter.mode(ui.Color color, ui.BlendMode blendMode)
    : _color = color,
      _blendMode = blendMode,
      _matrix = null,
      _type = ColorFilterType.mode;

  /// Construct a color filter that transforms a color by a 5x5 matrix, where
  /// the fifth row is implicitly added in an identity configuration.
  ///
  /// Every pixel's color value, repsented as an `[R, G, B, A]`, is matrix
  /// multiplied to create a new color:
  ///
  /// ```text
  /// | R' |   | a00 a01 a02 a03 a04 |   | R |
  /// | G' |   | a10 a11 a22 a33 a44 |   | G |
  /// | B' | = | a20 a21 a22 a33 a44 | * | B |
  /// | A' |   | a30 a31 a22 a33 a44 |   | A |
  /// | 1  |   |  0   0   0   0   1  |   | 1 |
  /// ```
  ///
  /// The matrix is in row-major order and the translation column is specified
  /// in unnormalized, 0...255, space. For example, the identity matrix is:
  ///
  /// ```
  /// const ColorMatrix identity = ColorFilter.matrix(<double>[
  ///   1, 0, 0, 0, 0,
  ///   0, 1, 0, 0, 0,
  ///   0, 0, 1, 0, 0,
  ///   0, 0, 0, 1, 0,
  /// ]);
  /// ```
  ///
  /// ## Examples
  ///
  /// An inversion color matrix:
  ///
  /// ```
  /// const ColorFilter invert = ColorFilter.matrix(<double>[
  ///   -1,  0,  0, 0, 255,
  ///    0, -1,  0, 0, 255,
  ///    0,  0, -1, 0, 255,
  ///    0,  0,  0, 1,   0,
  /// ]);
  /// ```
  ///
  /// A sepia-toned color matrix (values based on the [Filter Effects Spec](https://www.w3.org/TR/filter-effects-1/#sepiaEquivalent)):
  ///
  /// ```
  /// const ColorFilter sepia = ColorFilter.matrix(<double>[
  ///   0.393, 0.769, 0.189, 0, 0,
  ///   0.349, 0.686, 0.168, 0, 0,
  ///   0.272, 0.534, 0.131, 0, 0,
  ///   0,     0,     0,     1, 0,
  /// ]);
  /// ```
  ///
  /// A greyscale color filter (values based on the [Filter Effects Spec](https://www.w3.org/TR/filter-effects-1/#grayscaleEquivalent)):
  ///
  /// ```
  /// const ColorFilter greyscale = ColorFilter.matrix(<double>[
  ///   0.2126, 0.7152, 0.0722, 0, 0,
  ///   0.2126, 0.7152, 0.0722, 0, 0,
  ///   0.2126, 0.7152, 0.0722, 0, 0,
  ///   0,      0,      0,      1, 0,
  /// ]);
  /// ```
  const EngineColorFilter.matrix(List<double> matrix)
      : _color = null,
        _blendMode = null,
        _matrix = matrix,
        _type = ColorFilterType.matrix;

  /// Construct a color filter that applies the sRGB gamma curve to the RGB
  /// channels.
  const EngineColorFilter.linearToSrgbGamma()
      : _color = null,
        _blendMode = null,
        _matrix = null,
        _type = ColorFilterType.linearToSrgbGamma;

  /// Creates a color filter that applies the inverse of the sRGB gamma curve
  /// to the RGB channels.
  const EngineColorFilter.srgbToLinearGamma()
      : _color = null,
        _blendMode = null,
        _matrix = null,
        _type = ColorFilterType.srgbToLinearGamma;

  final ui.Color? _color;
  final ui.BlendMode? _blendMode;
  final List<double>? _matrix;
  final ColorFilterType _type;

  /// Convert the current [ColorFilter] to either a [CkColorFilter] or [HtmlEngineColorFilter]
  /// depending on the renderer.
  ///
  /// After calling this function and getting the renderer specific ColorFilter,
  /// cast the correct type on the converted ColorFilter based on the renderer backend:
  ///
  /// canvaskit: [CkColorFilter]
  /// Html: [HtmlEngineColorFilter]
  ///
  /// ## Example uses:
  /// ```
  /// CkColorFilter ckColorFilter =
  ///   (ColorFilter.mode(Color color, BlendMode blendMode) as EngineColorFilter).toRendererColorFilter() as CkColorFilter;
  /// ```
  ///
  /// This workaround allows ColorFilter to be const constructbile and
  /// efficiently comparable, so that widgets can check for ColorFilter equality to
  /// avoid repainting.
  dynamic toRendererColorFilter() {
    switch (_type) {
      case ColorFilterType.mode:
        if (_color == null || _blendMode == null) {
          return null;
        }
        return renderer.createModeColorFilter(this, _color!, _blendMode!);
      case ColorFilterType.matrix:
        if (_matrix == null) {
          return null;
        }
        assert(_matrix!.length == 20, 'Color Matrix must have 20 entries.');
        return renderer.createMatrixColorFilter(this, _matrix!);
      case ColorFilterType.linearToSrgbGamma:
        return renderer.createLinearToSrgbGammaColorFilter(this);
      case ColorFilterType.srgbToLinearGamma:
        return renderer.createSrgbToLinearGammaColorFilter(this);
      default:
        throw StateError('Unknown mode $_type for ColorFilter.');
    }
  }
}
