// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:sky' as sky;
import 'dart:sky' show Point, Offset, Size, Rect, Color, Paint, Path;

import 'package:sky/base/hit_test.dart';
import 'package:sky/rendering/sky_binding.dart';

class DragStartEvent extends sky.Event {
  DragStartEvent() : super(type: 'dragstart');
}

class DragEnterEvent extends sky.Event {
  DragEnterEvent() : super(type: 'dragenter');
}

class DragLeaveEvent extends sky.Event {
  DragLeaveEvent() : super(type: 'dragleave');
}

class DragEvent extends sky.Event {
  DragEvent({ this.data }) : super(type: 'drag');

  final dynamic data;
}

class DropEvent extends sky.Event {
  DropEvent({ this.data }) : super(type: 'drop');

  final dynamic data;
}

class DragController {
  final SkyBinding binding_ = SkyBinding.instance;
  HitTestResult _originalResult;
  HitTestResult _previousResult;

  void start(Point globalPosition) {
    _originalResult = binding_.hitTest(globalPosition);
    binding_.dispatchEvent(new DragStartEvent(), _originalResult);
  }

  void update(Point globalPosition) {
    HitTestResult currentResult = binding_.hitTest(globalPosition);
    List<HitTestEntry> currentPath = currentResult.path;
    List<HitTestEntry> previousPath = _previousResult != null ? _previousResult.path : [];

    Set<HitTestEntry> previous = new Set<HitTestEntry>.from(previousPath);
    Set<HitTestEntry> current = new Set<HitTestEntry>.from(currentPath);

    Iterable<HitTestEntry> left = previousPath.where(
      (HitTestEntry entry) => !current.contains(entry));
    Iterable<HitTestEntry> entered = currentPath.where(
      (HitTestEntry entry) => !previous.contains(entry));

    HitTestResult leftResult = new HitTestResult()..path.addAll(left);
    HitTestResult enteredResult = new HitTestResult()..path.addAll(entered);

    _previousResult = currentResult;
    binding_.dispatchEvent(new DragLeaveEvent(), leftResult);
    binding_.dispatchEvent(new DragEnterEvent(), enteredResult);
  }

  void drop(Point globalPosition, dynamic data) {
    binding_.dispatchEvent(new DragLeaveEvent(), _previousResult);
    binding_.dispatchEvent(new DragEvent(data: data), _originalResult);
    HitTestResult dropResult = binding_.hitTest(globalPosition);
    binding_.dispatchEvent(new DropEvent(data: data), dropResult);
  }
}
