// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(yjbanov): rename this to svg_clip.dart
import 'package:ui/ui.dart' as ui;

import '../browser_detection.dart';
import '../dom.dart';
import '../svg.dart';
import '../svg_filter.dart';
import 'path/path.dart';
import 'path/path_to_svg.dart';

/// Counter used for generating clip path id inside an svg <defs> tag.
int _clipIdCounter = 0;

class SvgClip {
  factory SvgClip.fromPath(ui.Path path, {
    double offsetX = 0,
    double offsetY = 0,
    double scaleX = 1.0,
    double scaleY = 1.0,
  }) {
    final SVGPathElement svgPath = createSVGPathElement();
    if (scaleX != 1 || scaleY != 1) {
      svgPath.setAttribute('transform', 'scale($scaleX, $scaleY)');
    }
    svgPath.setAttribute('d', pathToSvg((path as SurfacePath).pathRef, offsetX: offsetX, offsetY: offsetY));

    final SvgClipBuilder builder = SvgClipBuilder();
    builder.addGeometry(svgPath);
    return builder.build();
  }

  factory SvgClip.fromRect(ui.Rect rect) {
    final SVGRectElement svgRect = createSVGRectElement();
    svgRect.setX(rect.left);
    svgRect.setY(rect.top);
    svgRect.setWidth(rect.width);
    svgRect.setHeight(rect.height);

    final SvgClipBuilder builder = SvgClipBuilder();
    builder.addGeometry(svgRect);
    return builder.build();
  }

  SvgClip._({
    required this.id,
    required this.clipPath,
  }) : url = 'url(#$id)';

  final String id;
  final SVGClipPathElement clipPath;
  final String url;

  /// Wraps the clip to be usable for clipping HTML elements.
  ///
  /// `<clipPath>` cannot be directly added to an HTML node, so this function
  /// wraps it in an `<svg><defs>...</defs></svg>` structure. `clipPathUnits` is
  /// applied for compatibility with HTML's coordinate system.
  SvgHtmlClip wrapForHtml() {
    // Firefox objectBoundingBox fails to scale to 1x1 units, instead use
    // no clipPathUnits but write the path in target units.
    if (browserEngine != BrowserEngine.firefox) {
      clipPath.setAttribute('clipPathUnits', 'objectBoundingBox');
    }

    final SVGSVGElement host = kSvgResourceHeader.cloneNode(false) as SVGSVGElement;
    final SVGDefsElement defs = createSVGDefsElement();
    host.append(defs);
    defs.append(clipPath);

    return SvgHtmlClip(
      host: host,
      clip: this,
    );
  }
}

class SvgHtmlClip {
  SvgHtmlClip({
    required this.host,
    required this.clip,
  });

  final SVGSVGElement host;
  final SvgClip clip;
}

class SvgClipBuilder {
  SvgClipBuilder() : _id = 'svgClip${_clipIdCounter++}' {
    clipPath.id = _id;
  }

  final String _id;
  final SVGClipPathElement clipPath = createSVGClipPathElement();

  void addGeometry(SVGGeometryElement geometry) {
    clipPath.append(geometry);
  }

  SvgClip build() {
    return SvgClip._(
      id: _id,
      clipPath: clipPath,
    );
  }
}

/// Resets clip ids. Used for testing by [debugForgetFrameScene] API.
void debugResetSvgClipIds() {
  _clipIdCounter = 0;
}
