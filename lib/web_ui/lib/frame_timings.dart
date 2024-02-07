// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

enum FramePhase {
  vsyncStart,
  buildStart,
  buildFinish,
  rasterStart,
  rasterFinish,
  rasterFinishWallTime,
}

enum _FrameTimingInfo {
  layerCacheCount,
  layerCacheBytes,
  pictureCacheCount,
  pictureCacheBytes,
  frameNumber,
}

class FrameTiming {
  factory FrameTiming({
    required int vsyncStart,
    required int buildStart,
    required int buildFinish,
    required int rasterStart,
    required int rasterFinish,
    required int rasterFinishWallTime,
    int layerCacheCount = 0,
    int layerCacheBytes = 0,
    int pictureCacheCount = 0,
    int pictureCacheBytes = 0,
    int frameNumber = 1,
  }) {
    return FrameTiming._(<int>[
      vsyncStart,
      buildStart,
      buildFinish,
      rasterStart,
      rasterFinish,
      rasterFinishWallTime,
      layerCacheCount,
      layerCacheBytes,
      pictureCacheCount,
      pictureCacheBytes,
      frameNumber,
    ]);
  }

  FrameTiming._(this._data)
      : assert(_data.length == _dataLength);

  static final int _dataLength = FramePhase.values.length + _FrameTimingInfo.values.length;

  int timestampInMicroseconds(FramePhase phase) => _data[phase.index];

  Duration _rawDuration(FramePhase phase) => Duration(microseconds: _data[phase.index]);

  int _rawInfo(_FrameTimingInfo info) => _data[FramePhase.values.length + info.index];

  Duration get buildDuration =>
      _rawDuration(FramePhase.buildFinish) - _rawDuration(FramePhase.buildStart);

  Duration get rasterDuration =>
      _rawDuration(FramePhase.rasterFinish) - _rawDuration(FramePhase.rasterStart);

  Duration get vsyncOverhead => _rawDuration(FramePhase.buildStart) - _rawDuration(FramePhase.vsyncStart);

  Duration get totalSpan =>
      _rawDuration(FramePhase.rasterFinish) - _rawDuration(FramePhase.vsyncStart);

  int get layerCacheCount => _rawInfo(_FrameTimingInfo.layerCacheCount);

  int get layerCacheBytes => _rawInfo(_FrameTimingInfo.layerCacheBytes);

  double get layerCacheMegabytes => layerCacheBytes / 1024.0 / 1024.0;

  int get pictureCacheCount => _rawInfo(_FrameTimingInfo.pictureCacheCount);

  int get pictureCacheBytes => _rawInfo(_FrameTimingInfo.pictureCacheBytes);

  double get pictureCacheMegabytes => pictureCacheBytes / 1024.0 / 1024.0;

  int get frameNumber => _data.last;

  final List<int> _data;  // some elements in microseconds, some in bytes, some are counts

  String _formatMS(Duration duration) => '${duration.inMicroseconds * 0.001}ms';

  @override
  String toString() {
    return '$runtimeType(buildDuration: ${_formatMS(buildDuration)}, '
        'rasterDuration: ${_formatMS(rasterDuration)}, '
        'vsyncOverhead: ${_formatMS(vsyncOverhead)}, '
        'totalSpan: ${_formatMS(totalSpan)}, '
        'layerCacheCount: $layerCacheCount, '
        'layerCacheBytes: $layerCacheBytes, '
        'pictureCacheCount: $pictureCacheCount, '
        'pictureCacheBytes: $pictureCacheBytes, '
        'frameNumber: ${_data.last})';
  }
}
