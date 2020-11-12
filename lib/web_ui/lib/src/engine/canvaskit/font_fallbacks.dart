// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.12
part of engine;

void _findFontsForMissingCodeunit(int codeunit) {
  _ensureNotoFontTreeCreated();
  List<String> families = _lookupNotoFontsForCodeunit(codeunit);
  print('Fonts which match $codeunit: $families');
}

void _ensureNotoFontTreeCreated() {
  if (_notoTreeRoot != null) {
    return;
  }

  for (_NotoFont font in _notoFonts) {
    for (_UnicodeRange range in font.unicodeRanges) {
      _insertNotoFontRange(range, font);
    }
  }
}

List<String> _lookupNotoFontsForCodeunit(int codeunit) {
  List<String> lookupHelper(_NotoTreeNode node) {
    if (node.range.contains(codeunit)) {
      return node.fonts.map((_NotoFont f) => f.name).toList();
    }
    if (node.range.start > codeunit) {
      if (node.left != null) {
        return lookupHelper(node.left!);
      } else {
        print('Could not find font to match $codeunit');
        return <String>[];
      }
    } else {
      if (node.right != null) {
        return lookupHelper(node.right!);
      } else {
        print('Could not find font to match $codeunit');
        return <String>[];
      }
    }
  }

  return lookupHelper(_notoTreeRoot!);
}

class _NotoFont {
  final String name;
  final List<_UnicodeRange> unicodeRanges;

  const _NotoFont(this.name, this.unicodeRanges);

  String get googleFontsCssUrl =>
      'https://fonts.googleapis.com/css2?family=${name.replaceAll(' ', '+')}';
}

class _UnicodeRange {
  final int start;
  final int end;

  const _UnicodeRange(this.start, this.end);

  bool contains(int codeUnit) {
    return start <= codeUnit && codeUnit <= end;
  }

  bool operator ==(dynamic other) {
    if (other is! _UnicodeRange) {
      return false;
    }
    _UnicodeRange range = other;
    return range.start == start && range.end == end;
  }
}

const List<_NotoFont> _notoFonts = <_NotoFont>[
  _NotoFont('Noto Naskh Arabic UI', <_UnicodeRange>[
    _UnicodeRange(1536, 1791),
    _UnicodeRange(8204, 8206),
    _UnicodeRange(8208, 8209),
    _UnicodeRange(8271, 8271),
    _UnicodeRange(11841, 11841),
    _UnicodeRange(64336, 65023),
    _UnicodeRange(65132, 65276),
  ]),
  _NotoFont('Noto Sans Armenian', <_UnicodeRange>[
    _UnicodeRange(1328, 1424),
    _UnicodeRange(64275, 64279),
  ]),
  _NotoFont('Noto Sans Bengali UI', <_UnicodeRange>[
    _UnicodeRange(2404, 2405),
    _UnicodeRange(2433, 2555),
    _UnicodeRange(8204, 8205),
    _UnicodeRange(8377, 8377),
    _UnicodeRange(9676, 9676),
  ]),
  _NotoFont('Noto Sans Myanmar UI', <_UnicodeRange>[
    _UnicodeRange(4096, 4255),
    _UnicodeRange(8204, 8205),
    _UnicodeRange(9676, 9676),
  ]),
  _NotoFont('Noto Sans HK', <_UnicodeRange>[
    _UnicodeRange(12288, 12351),
    _UnicodeRange(12549, 12585),
    _UnicodeRange(19968, 40959),
  ]),
  _NotoFont('Noto Sans SC', <_UnicodeRange>[
    _UnicodeRange(12288, 12591),
    _UnicodeRange(12800, 13311),
    _UnicodeRange(19968, 40959),
    _UnicodeRange(65072, 65135),
    _UnicodeRange(65280, 65519),
  ]),
  _NotoFont('Noto Sans TC', <_UnicodeRange>[
    _UnicodeRange(12288, 12351),
    _UnicodeRange(12549, 12585),
    _UnicodeRange(19968, 40959),
  ]),
  _NotoFont('Noto Sans Egyptian Hieroglyphs', <_UnicodeRange>[
    _UnicodeRange(77824, 78894),
  ]),
  _NotoFont('Noto Sans Ethiopic', <_UnicodeRange>[
    _UnicodeRange(4608, 5017),
    _UnicodeRange(11648, 11742),
    _UnicodeRange(43777, 43822),
  ]),
  _NotoFont('Noto Sans Georgian', <_UnicodeRange>[
    _UnicodeRange(1417, 1417),
    _UnicodeRange(4256, 4351),
    _UnicodeRange(11520, 11567),
  ]),
  _NotoFont('Noto Sans Gujarati UI', <_UnicodeRange>[
    _UnicodeRange(2404, 2405),
    _UnicodeRange(2688, 2815),
    _UnicodeRange(8204, 8205),
    _UnicodeRange(8377, 8377),
    _UnicodeRange(9676, 9676),
    _UnicodeRange(43056, 43065),
  ]),
  _NotoFont('Noto Sans Gurmukhi UI', <_UnicodeRange>[
    _UnicodeRange(2404, 2405),
    _UnicodeRange(2561, 2677),
    _UnicodeRange(8204, 8205),
    _UnicodeRange(8377, 8377),
    _UnicodeRange(9676, 9676),
    _UnicodeRange(9772, 9772),
    _UnicodeRange(43056, 43065),
  ]),
  _NotoFont('Noto Sans Hebrew', <_UnicodeRange>[
    _UnicodeRange(1424, 1535),
    _UnicodeRange(8362, 8362),
    _UnicodeRange(9676, 9676),
    _UnicodeRange(64285, 64335),
  ]),
  _NotoFont('Noto Sans Devanagari UI', <_UnicodeRange>[
    _UnicodeRange(2304, 2431),
    _UnicodeRange(7376, 7414),
    _UnicodeRange(7416, 7417),
    _UnicodeRange(8204, 9205),
    _UnicodeRange(8360, 8360),
    _UnicodeRange(8377, 8377),
    _UnicodeRange(9676, 9676),
    _UnicodeRange(43056, 43065),
    _UnicodeRange(43232, 43259),
  ]),
  _NotoFont('Noto Sans JP', <_UnicodeRange>[
    _UnicodeRange(12288, 12543),
    _UnicodeRange(19968, 40959),
    _UnicodeRange(65280, 65519),
  ]),
  _NotoFont('Noto Sans Kannada UI', <_UnicodeRange>[
    _UnicodeRange(2404, 2405),
    _UnicodeRange(3202, 3314),
    _UnicodeRange(8204, 8205),
    _UnicodeRange(8377, 8377),
    _UnicodeRange(9676, 9676),
  ]),
  _NotoFont('Noto Sans Khmer UI', <_UnicodeRange>[
    _UnicodeRange(6016, 6143),
    _UnicodeRange(8204, 8204),
    _UnicodeRange(9676, 9676),
  ]),
  _NotoFont('Noto Sans KR', <_UnicodeRange>[
    _UnicodeRange(12593, 12686),
    _UnicodeRange(12800, 12828),
    _UnicodeRange(12896, 12923),
    _UnicodeRange(44032, 55215),
  ]),
  _NotoFont('Noto Sans Lao UI', <_UnicodeRange>[
    _UnicodeRange(3713, 3807),
    _UnicodeRange(9676, 9676),
  ]),
  _NotoFont('Noto Sans Malayalam UI', <_UnicodeRange>[
    _UnicodeRange(775, 775),
    _UnicodeRange(803, 803),
    _UnicodeRange(2404, 2405),
    _UnicodeRange(3330, 3455),
    _UnicodeRange(8204, 8205),
    _UnicodeRange(8377, 8377),
    _UnicodeRange(9676, 9676),
  ]),
  _NotoFont('Noto Sans Sinhala', <_UnicodeRange>[
    _UnicodeRange(2404, 2405),
    _UnicodeRange(3458, 3572),
    _UnicodeRange(8204, 8205),
    _UnicodeRange(9676, 9676),
  ]),
  _NotoFont('Noto Sans Tamil UI', <_UnicodeRange>[
    _UnicodeRange(2404, 2405),
    _UnicodeRange(2946, 3066),
    _UnicodeRange(8204, 8205),
    _UnicodeRange(8377, 8377),
    _UnicodeRange(9676, 9676),
  ]),
  _NotoFont('Noto Sans Telugu UI', <_UnicodeRange>[
    _UnicodeRange(2385, 2386),
    _UnicodeRange(2404, 2405),
    _UnicodeRange(3072, 3199),
    _UnicodeRange(7386, 7386),
    _UnicodeRange(8204, 8205),
    _UnicodeRange(9676, 9676),
  ]),
  _NotoFont('Noto Sans Thai UI', <_UnicodeRange>[
    _UnicodeRange(3585, 3675),
    _UnicodeRange(8204, 8205),
    _UnicodeRange(9676, 9676),
  ]),
  _NotoFont('Noto Sans', <_UnicodeRange>[
    _UnicodeRange(0, 255),
    _UnicodeRange(305, 305),
    _UnicodeRange(338, 339),
    _UnicodeRange(699, 700),
    _UnicodeRange(710, 710),
    _UnicodeRange(730, 730),
    _UnicodeRange(732, 732),
    _UnicodeRange(8192, 8303),
    _UnicodeRange(8308, 8308),
    _UnicodeRange(8364, 8364),
    _UnicodeRange(8482, 8482),
    _UnicodeRange(8593, 8593),
    _UnicodeRange(8595, 8595),
    _UnicodeRange(8722, 8722),
    _UnicodeRange(8725, 8725),
    _UnicodeRange(65279, 65279),
    _UnicodeRange(65533, 65533),
    _UnicodeRange(1024, 1119),
    _UnicodeRange(1168, 1169),
    _UnicodeRange(1200, 1201),
    _UnicodeRange(8470, 8470),
    _UnicodeRange(1120, 1327),
    _UnicodeRange(7296, 7304),
    _UnicodeRange(8372, 8372),
    _UnicodeRange(11744, 11775),
    _UnicodeRange(42560, 42655),
    _UnicodeRange(65070, 65071),
    _UnicodeRange(880, 1023),
    _UnicodeRange(7936, 8191),
    _UnicodeRange(256, 591),
    _UnicodeRange(601, 601),
    _UnicodeRange(7680, 7935),
    _UnicodeRange(8224, 8224),
    _UnicodeRange(8352, 8363),
    _UnicodeRange(8365, 8399),
    _UnicodeRange(8467, 8467),
    _UnicodeRange(11360, 11391),
    _UnicodeRange(42784, 43007),
    _UnicodeRange(258, 259),
    _UnicodeRange(272, 273),
    _UnicodeRange(296, 297),
    _UnicodeRange(360, 361),
    _UnicodeRange(416, 417),
    _UnicodeRange(431, 432),
    _UnicodeRange(7840, 7929),
    _UnicodeRange(8363, 8363),
  ]),
//  Noto Sans Symbols
//  Noto Color Emoji Compat
];

/// A node in a red-black tree for Noto Fonts.
class _NotoTreeNode {
  _NotoTreeNode? parent;
  _NotoTreeNode? left;
  _NotoTreeNode? right;

  /// If `true`, then this node is black. Otherwise it is red.
  bool isBlack = false;
  bool get isRed => !isBlack;

  final _UnicodeRange range;
  final List<_NotoFont> fonts;

  _NotoTreeNode(this.range) : this.fonts = <_NotoFont>[];
}

/// Associates [range] with [font] in the Noto Font tree.
void _insertNotoFontRange(_UnicodeRange range, _NotoFont font) {
  _NotoTreeNode? newNode =
      _insertNotoFontRangeHelper(_notoTreeRoot, range, font);
  if (newNode != null) {
    _repairNotoFontTree(newNode);

    // Make sure the root node is correctly set.
    _NotoTreeNode newRoot = newNode;
    while (newRoot.parent != null) {
      newRoot = newRoot.parent!;
    }

    _notoTreeRoot = newRoot;
  }
}

/// Recurses the font tree and associates [range] with [font].
///
/// If a new node is created, it is returned so we can repair the tree.
_NotoTreeNode? _insertNotoFontRangeHelper(
    _NotoTreeNode? root, _UnicodeRange range, _NotoFont font) {
  if (root != null) {
    if (root.range == range) {
      // The root node range is the same as the range we're inserting.
      root.fonts.add(font);
      return null;
    }
    if (range.start < root.range.start) {
      if (root.left != null) {
        return _insertNotoFontRangeHelper(root.left, range, font);
      } else {
        _NotoTreeNode newNode = _NotoTreeNode(range);
        newNode.fonts.add(font);
        newNode.parent = root;
        root.left = newNode;
        return newNode;
      }
    } else {
      if (root.right != null) {
        return _insertNotoFontRangeHelper(root.right, range, font);
      } else {
        _NotoTreeNode newNode = _NotoTreeNode(range);
        newNode.fonts.add(font);
        newNode.parent = root;
        root.right = newNode;
        return newNode;
      }
    }
  } else {
    // If [root] is null, then the root of the entire Noto Font tree is null.
    assert(_notoTreeRoot == null);
    _NotoTreeNode newRoot = _NotoTreeNode(range);
    newRoot.fonts.add(font);
    _notoTreeRoot = newRoot;
    return newRoot;
  }
}

void _repairNotoFontTree(_NotoTreeNode node) {
  if (node.parent == null) {
    // This is the root node. The root node must be black.
    node.isBlack = true;
    return;
  } else if (node.parent!.isBlack) {
    // Do nothing.
    return;
  }

  // If we've reached here, then (1) node's parent is non-null and (2) node's
  // parent is red, which means that node's parent is not the root node and
  // therefore (3) node's grandparent is not null;

  _NotoTreeNode parent = node.parent!;
  _NotoTreeNode grandparent = parent.parent!;

  _NotoTreeNode? uncle;
  if (parent == grandparent.left) {
    uncle = grandparent.right;
  } else {
    uncle = grandparent.left;
  }
  if (uncle != null && uncle.isRed) {
    parent.isBlack = true;
    uncle.isBlack = true;
    _repairNotoFontTree(grandparent);
    return;
  }

  // If we've reached here, then the parent is red and the uncle is black
  // (note: null leaves are considered black). We must re-balance the tree
  // by rotating such that the node is in the grandparent position.

  void rotateLeft(_NotoTreeNode node) {
    // We will only ever call this on nodes which have a right child.
    _NotoTreeNode newNode = node.right!;
    _NotoTreeNode? parent = node.parent;

    node.right = newNode.left;
    newNode.left = node;
    node.parent = newNode;
    if (node.right != null) {
      node.right!.parent = node;
    }

    if (parent != null) {
      if (node == parent.left) {
        parent.left = newNode;
      } else {
        parent.right = newNode;
      }
    }

    newNode.parent = parent;
  }

  void rotateRight(_NotoTreeNode node) {
    // We will only ever call this on nodes which have a left child.
    _NotoTreeNode newNode = node.left!;
    _NotoTreeNode? parent = node.parent;

    node.left = newNode.right;
    newNode.right = node;
    node.parent = newNode;

    if (node.left != null) {
      node.left!.parent = node;
    }

    if (parent != null) {
      if (node == parent.left) {
        parent.left = newNode;
      } else {
        parent.right = newNode;
      }
    }

    newNode.parent = parent;
  }

  if (node == parent.right && parent == grandparent.left) {
    rotateLeft(parent);
    node = node.left!;
  } else if (node == parent.left && parent == grandparent.right) {
    rotateRight(parent);
    node = node.right!;
  }

  parent = node.parent!;
  grandparent = parent.parent!;

  if (node == parent.left) {
    rotateRight(grandparent);
  } else {
    rotateLeft(grandparent);
  }

  parent.isBlack = true;
  grandparent.isBlack = false;
}

_NotoTreeNode? _notoTreeRoot;
