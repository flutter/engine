// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

abstract class TextFragmenter {
  const TextFragmenter();

  List<TextFragment> fragment();
}

abstract class TextFragment {
  const TextFragment(this.start, this.end);

  final int start;
  final int end;

  /// Whether this fragment's range overlaps with the range from [start] to [end].
  bool overlapsWith(int start, int end) {
    return start < this.end && this.start < end;
  }
}
