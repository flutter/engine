// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.12
part of engine;

/// Associates a [T] with one or more [_UnicodeRange]s.
class _IntervalTree<T> {
  final _IntervalTreeNode<T> root;

  _IntervalTree._(this.root);

  factory _IntervalTree.createFromRanges(
      Map<T, List<_UnicodeRange>> rangesMap) {
    // Get a list of all the ranges ordered by start index.
    List<_Interval<T>> intervals = <_Interval<T>>[];
    for (T key in rangesMap.keys) {
      for (_UnicodeRange range in rangesMap[key]!) {
        intervals.add(_Interval<T>(key, range.start, range.end));
      }
    }
    intervals.sort((_Interval<T> a, _Interval<T> b) => a.start - b.start);

    // Make a balanced binary search tree from the sorted intervals.
    _IntervalTreeNode<T>? _makeBalancedTree(List<_Interval<T>> intervals) {
      if (intervals.length == 0) {
        return null;
      }
      if (intervals.length == 1) {
        return _IntervalTreeNode<T>(intervals.single);
      }
      int mid = intervals.length ~/ 2;
      _IntervalTreeNode<T> root = _IntervalTreeNode<T>(intervals[mid]);
      root.left = _makeBalancedTree(intervals.sublist(0, mid));
      root.right = _makeBalancedTree(intervals.sublist(mid + 1));
    }

    void _computeHigh(_IntervalTreeNode<T> root) {
      if (root.left == null && root.right == null) {
        root._cachedHigh = root.interval.end;
      } else if (root.left == null) {
        _computeHigh(root.right!);
        root._cachedHigh = math.max(root.interval.end, root.right!.high);
      } else if (root.right == null) {
        _computeHigh(root.left!);
        root._cachedHigh = math.max(root.interval.end, root.left!.high);
      } else {
        _computeHigh(root.right!);
        _computeHigh(root.left!);
        root._cachedHigh = math.max(
            root.interval.end, math.max(root.left!.high, root.right!.high));
      }
    }

    _IntervalTreeNode<T> root = _makeBalancedTree(intervals)!;
    _computeHigh(root);

    return _IntervalTree._(root);
  }

  /// Returns the list of objects which have been associated with intervals that
  /// intersect with [x].
  List<T> intersections(int x) {
    List<T> results = <T>[];
    root.searchForPoint(x, results);
    return results;
  }
}

class _Interval<T> {
  final T value;
  final int start;
  final int end;

  bool contains(int x) {
    return start <= x && x <= end;
  }

  const _Interval(this.value, this.start, this.end);
}

class _IntervalTreeNode<T> {
  final _Interval<T> interval;

  int get low => interval.start;
  int get high => _cachedHigh ?? interval.end;
  int? _cachedHigh;

  _IntervalTreeNode<T>? left;
  _IntervalTreeNode<T>? right;

  _IntervalTreeNode(this.interval);

  // Searches the tree rooted at this node for all T containing [x].
  void searchForPoint(int x, List<T> result) {
    if (x > high) {
      return;
    }
    left?.searchForPoint(x, result);
    if (interval.contains(x)) {
      result.add(interval.value);
    }
    if (x < low) {
      return;
    }
    right?.searchForPoint(x, result);
  }
}
