// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';


import 'conic.dart';
import 'path_ref.dart';
import 'path_utils.dart';

/// Converts [path] to SVG path syntax to be used as "d" attribute in path
/// element.
String pathToSvg(PathRef pathRef, {double offsetX = 0, double offsetY = 0}) {
  final buffer = StringBuffer();
  final iter = PathRefIterator(pathRef);
  var verb = 0;
  final outPts = Float32List(PathRefIterator.kMaxBufferSize);
  while ((verb = iter.next(outPts)) != SPath.kDoneVerb) {
    switch (verb) {
      case SPath.kMoveVerb:
        buffer.write('M ${outPts[0] + offsetX} ${outPts[1] + offsetY}');
      case SPath.kLineVerb:
        buffer.write('L ${outPts[2] + offsetX} ${outPts[3] + offsetY}');
      case SPath.kCubicVerb:
        buffer.write('C ${outPts[2] + offsetX} ${outPts[3] + offsetY} '
            '${outPts[4] + offsetX} ${outPts[5] + offsetY} ${outPts[6] + offsetX} ${outPts[7] + offsetY}');
      case SPath.kQuadVerb:
        buffer.write('Q ${outPts[2] + offsetX} ${outPts[3] + offsetY} '
            '${outPts[4] + offsetX} ${outPts[5] + offsetY}');
      case SPath.kConicVerb:
        final w = iter.conicWeight;
        final conic = Conic(outPts[0], outPts[1], outPts[2], outPts[3],
            outPts[4], outPts[5], w);
        final points = conic.toQuads();
        final len = points.length;
        for (var i = 1; i < len; i += 2) {
          final p1x = points[i].dx;
          final p1y = points[i].dy;
          final p2x = points[i + 1].dx;
          final p2y = points[i + 1].dy;
          buffer.write('Q ${p1x + offsetX} ${p1y + offsetY} '
              '${p2x + offsetX} ${p2y + offsetY}');
        }
      case SPath.kCloseVerb:
        buffer.write('Z');
      default:
        throw UnimplementedError('Unknown path verb $verb');
    }
  }
  return buffer.toString();
}
