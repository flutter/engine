// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


part of engine;

js.JsArray<Float32List> _encodeRawColorList(Int32List rawColors) {
  final int colorCount = rawColors.length;
  final List<ui.Color> colors = <ui.Color>[];
  for (int i = 0; i < colorCount; ++i) {
    colors.add(ui.Color(rawColors[i]));
  }
  return makeColorList(colors);
}

class CkVertices implements ui.Vertices {
  late SkVertices skVertices;

  CkVertices(
    ui.VertexMode mode,
    List<ui.Offset> positions, {
    List<ui.Offset>? textureCoordinates,
    List<ui.Color>? colors,
    List<int>? indices,
  })  : assert(mode != null), // ignore: unnecessary_null_comparison
        assert(positions != null) { // ignore: unnecessary_null_comparison
    if (textureCoordinates != null &&
        textureCoordinates.length != positions.length)
      throw ArgumentError(
          '"positions" and "textureCoordinates" lengths must match.');
    if (colors != null && colors.length != positions.length)
      throw ArgumentError('"positions" and "colors" lengths must match.');
    if (indices != null &&
        indices.any((int i) => i < 0 || i >= positions.length))
      throw ArgumentError(
          '"indices" values must be valid indices in the positions list.');

    skVertices = canvasKitJs.MakeSkVertices(
      toSkVertexMode(mode),
      toSkPoints2d(positions),
      textureCoordinates != null ? toSkPoints2d(textureCoordinates) : null,
      colors != null ? toSkIntColorList(colors) : null,
      indices != null ? toUint16List(indices) : null,
    );
  }

  CkVertices.raw(
    ui.VertexMode mode,
    Float32List positions, {
    Float32List? textureCoordinates,
    Int32List? colors,
    Uint16List? indices,
  })  : assert(mode != null), // ignore: unnecessary_null_comparison
        assert(positions != null) { // ignore: unnecessary_null_comparison
    if (textureCoordinates != null &&
        textureCoordinates.length != positions.length)
      throw ArgumentError(
          '"positions" and "textureCoordinates" lengths must match.');
    if (colors != null && colors.length * 2 != positions.length)
      throw ArgumentError('"positions" and "colors" lengths must match.');
    if (indices != null &&
        indices.any((int i) => i < 0 || i >= positions.length))
      throw ArgumentError(
          '"indices" values must be valid indices in the positions list.');

    skVertices = canvasKitJs.MakeSkVertices(
      toSkVertexMode(mode),
      rawPointsToSkPoints2d(positions),
      textureCoordinates != null ? rawPointsToSkPoints2d(textureCoordinates) : null,
      colors != null ? Uint32List.view(colors.buffer) : null,
      indices,
    );
  }
}
