// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' show base64, utf8;
import 'dart:io' hide BytesBuilder;
import 'dart:typed_data';

// Changing the structure of layouting classes must change the following places
// as well:
//
//  * Marshalling,
//  * Unmarshalling,
//  * Verification.

// The following segment is not only used in the generating script, but also
// copied to the generated package.
/*@@@ SHARED SEGMENT START @@@*/

/// The platform that the browser is running on.
enum LayoutPlatform {
  /// Windows.
  win,
  /// Linux.
  linux,
  /// MacOS or iOS.
  darwin,
}

// The length of [LayoutEntry.printable].
const int _kPrintableLength = 4;

/// Describes the characters that a physical keyboard key will be mapped to
/// under different modifier states, for a given language on a given
/// platform.
class LayoutEntry {
  /// Create a layout entry.
  LayoutEntry(this.printables, this.deadMasks)
    : assert(printables.length == _kPrintableLength);

  /// The printable characters that a key should be mapped to under different
  /// modifier states.
  ///
  /// The [printables] always have a length of 4, corresponding to "without any
  /// modifiers", "with Shift", "with AltGr", and "with Shift and AltGr"
  /// respectively. Some values might be empty, or be dead keys that are
  /// indiecated by [deadMasks].
  final List<String> printables;

  /// Whether the outcome of a key is a dead key under different modifier
  /// states.
  ///
  /// The four LSB [deadMasks] correspond to the four conditions of
  /// [printables]: 0x1 for "without any modifiers", 0x2 for "with Shift",
  /// 0x4 for "with AltGr", and 0x8 for "with Shift and AltGr". A set bit means
  /// the character is a dead key.
  final int deadMasks;

  /// An empty [LayoutEntry] that produces dead keys under all conditions.
  static final LayoutEntry empty = LayoutEntry(
    const <String>['', '', '', ''], 0xf);
}

/// Describes the characters that all goal keys will be mapped to for a given
/// language on a given platform.
class Layout {
  /// Create a [Layout].
  const Layout(this.language, this.platform, this.entries);

  /// The language being used.
  final String language;

  /// The platform that the browser is running on.
  final LayoutPlatform platform;

  /// Maps from DOM `KeyboardKey.key`s to the characters they produce.
  final Map<String, LayoutEntry> entries;
}

/// Describes all information needed to detect keyboard layout for any languages
/// on any platforms.
class LayoutStore {
  /// Create a [LayoutStore].
  const LayoutStore(this.goals, this.layouts);

  /// The list of goals, mapping from DOM `KeyboardKey.key` to their mandatory
  /// goal characters, or null if this goal is optional.
  ///
  /// Mandatory goals are characters that must be fulfilled during keyboard
  /// layout detection. If the character of a mandatory goal is not assigned in
  /// earlier stages, this character (the value of this map) will be assigned
  /// to its corresponding key (the key of this map).
  ///
  /// Optional goals are keys that will be tested to see if they can be mapped
  /// to mandatory goal characters.
  final Map<String, String?> goals;

  /// The layout information for different languages on different platforms.
  final List<Layout> layouts;
}

// A [ByteBuffer] that records a offset for the convenience of reading
// sequentially.
class _ByteStream {
  _ByteStream(this.buffer)
    : _data = buffer.asByteData(), _offset = 0;

  final ByteBuffer buffer;
  final ByteData _data;

  // The current offset.
  //
  // The next read will start from this byte (inclusive).
  int get offest => _offset;
  int _offset;

  // Read the next byte as an 8-bit unsigned integer, and increase [offset] by
  // 1.
  int readUint8() {
    final int result = _data.getUint8(_offset);
    _offset += 1;
    return result;
  }

  // Read the next few bytes as a UTF-8 string, and increase [offset]
  // accordingly.
  //
  // The first byte will be a uint8, `length`, the number of bytes of the UTF-8
  // sequence. Following that is the UTF-8 sequence. Therefore, the total
  // increment for [offset] is `length + 1`.
  //
  // If the `length` is 0, then an empty string is returned.
  String readString() {
    final int length = _data.getUint8(_offset);
    if (length == 0) {
      _offset += 1;
      return '';
    }
    final Uint8List bytes = buffer.asUint8List(_offset + 1, length);
    final String result = utf8.decode(bytes);
    _offset += 1 + length;
    return result;
  }

  // Read the next few bytes as a nullable UTF-8 string, and increase [offset]
  // accordingly.
  //
  // It is the same as [readString], except that if the `length` is 0, a null is
  // returned.
  String? readNullableString() {
    final int length = _data.getUint8(_offset);
    if (length == 0) {
      _offset += 1;
      return null;
    }
    return readString();
  }
}

/// Decode a [LayoutStore] out of the compressed binary data.
LayoutStore unmarshallStoreCompressed(String compressed) {
  final Uint8List bytes = Uint8List.fromList(gzip.decode(base64.decode(compressed)));
  return _unmarshallStore(bytes.buffer);
}

LayoutStore _unmarshallStore(ByteBuffer buffer) {
  final _ByteStream stream = _ByteStream(buffer);
  final Map<String, String?> goals = _unmarshallGoals(stream);
  final List<String> goalKeys = goals.keys.toList();
  final int layoutNum = stream.readUint8();
  final List<Layout> layouts = List<Layout>.generate(layoutNum, (_) {
    return _unmarshallLayout(stream, goalKeys);
  });
  return LayoutStore(goals, layouts);
}

Map<String, String?> _unmarshallGoals(_ByteStream stream) {
  final int goalsLength = stream.readUint8();
  return Map<String, String?>.fromEntries((() sync* {
    for (int goalIndex = 0; goalIndex < goalsLength; goalIndex += 1) {
      yield MapEntry<String, String?>(stream.readString(), stream.readNullableString());
    }
  })());
}

Layout _unmarshallLayout(_ByteStream stream, List<String> goalKeys) {
  final String language = stream.readString();
  final LayoutPlatform platform = LayoutPlatform.values[stream.readUint8()];
  final Map<String, LayoutEntry> entries = Map<String, LayoutEntry>.fromIterables(
    goalKeys,
    goalKeys.map((_) => _unmarshallLayoutEntry(stream)),
  );
  return Layout(language, platform, entries);
}

LayoutEntry _unmarshallLayoutEntry(_ByteStream stream) {
  final List<String> printables = List<String>.generate(_kPrintableLength,
    (_) => stream.readString());
  final int deadMasks = stream.readUint8();
  return LayoutEntry(printables, deadMasks);
}

/*@@@ SHARED SEGMENT END @@@*/

/// Compress a [LayoutStore] into the compressed binary data.
///
/// See [unmarshallStoreCompressed] for decompression.
///
/// The corretness of compression and decompression is checked by
/// [verifyLayoutStoreEqual], which is run at the end of every generation.
String marshallStoreCompressed(LayoutStore store) {
  final BytesBuilder bodyBuilder = BytesBuilder();
  _marshallStore(bodyBuilder, store);
  final Uint8List bytes = bodyBuilder.takeBytes();
  return base64.encode(gzip.encode(bytes));
}

void _marshallStore(BytesBuilder builder, LayoutStore store) {
  final List<String> goalKeys = store.goals.keys.toList();
  // Sanity check: All layouts should have the same list of keys.
  for (final Layout layout in store.layouts) {
    if (layout.entries.length != goalKeys.length) {
      throw Exception('Unmatched key list for ${layout.language}.${layout.platform}: '
        'Expect length ${goalKeys.length}, found length ${layout.entries.length}');
    }
    final Set<String> unfoundKeys = goalKeys.toSet().difference(layout.entries.keys.toSet());
    if (unfoundKeys.isNotEmpty) {
      throw Exception('Unmatched key list for ${layout.language}.${layout.platform}: '
        'The following keys are not found: ${unfoundKeys.join(', ')}.');
    }
  }

  _marshallGoals(builder, store.goals);
  _marshallUint8(builder, store.layouts.length);
  for (final Layout layout in store.layouts) {
    _marshallLayout(builder, layout, goalKeys);
  }
}

void _marshallUint8(BytesBuilder builder, int value) {
  if (value < 0 || value >= 256) {
    throw Exception('Out of range uint8: $value');
  }
  builder.addByte(value);
}

void _marshallString(BytesBuilder builder, String? string) {
  if (string == null) {
    _marshallUint8(builder, 0);
    return;
  }
  final List<int> encoded = utf8.encode(string);
  _marshallUint8(builder, encoded.length);
  builder.add(encoded);
}

void _marshallGoals(BytesBuilder builder, Map<String, String?> goals) {
  _marshallUint8(builder, goals.length);
  goals.forEach((String key, String? value) {
    _marshallString(builder, key);
    _marshallString(builder, value);
  });
}

void _marshallLayout(BytesBuilder builder, Layout layout, List<String> goalKeys) {
  _marshallString(builder, layout.language);
  _marshallUint8(builder, layout.platform.index);
  for (final String key in goalKeys) {
    _marshallLayoutEntry(builder, layout.entries[key]!);
  }
}

void _marshallLayoutEntry(BytesBuilder builder, LayoutEntry entry) {
  if (entry.printables.length != _kPrintableLength) {
    throw Exception('Malshaped entry printables: ${entry.printables}');
  }
  for (final String printable in entry.printables) {
    _marshallString(builder, printable);
  }
  builder.addByte(entry.deadMasks);
}

typedef _VerifyCallback<T> = void Function(T value1, T value2, String path);

/// Verify that two [LayoutStore]s are equal.
///
/// It verifies all fields of all children objects of the store. For maps,
/// it also verifies that the two stores have the same order of entries.
///
/// Inconsistencies will lead to throwing [Exception].
void verifyLayoutStoreEqual(LayoutStore store1, LayoutStore store2) {
  // Test if two values are equal by `==`, or throw an exception.
  void expectEqual<T>(T a, T b, String path) {
    if (a != b) {
      throw Exception('Error verifying unmarshalled layout on $path: $a != $b');
    }
  }
  // Test that two iterables are of the same length, and each of their elements
  // are verifies by `body`.
  void verifyEach<T>(Iterable<T> a, Iterable<T> b, String path, _VerifyCallback<T> body) {
    expectEqual(a.length, b.length, '$path.length');
    final Iterator<T> aIter = a.iterator;
    final Iterator<T> bIter = b.iterator;
    int index = 0;
    while (aIter.moveNext()) {
      assert(bIter.moveNext()); // Guaranteed true since they're of the same length.
      body(aIter.current, bIter.current, '$path[$index]');
      index += 1;
    }
  }

  // Verify Store.goals
  verifyEach(store1.goals.entries, store2.goals.entries, 'Store.goals',
    (MapEntry<String, String?> entry1, MapEntry<String, String?> entry2, String path) {
      expectEqual(entry1.key, entry2.key, '$path.key');
      expectEqual(entry1.value, entry2.value, '$path.value');
    });

  // Verify Layout
  verifyEach(store1.layouts, store2.layouts, 'Store.layouts',
    (Layout layout1, Layout layout2, String path) {
      expectEqual(layout1.language, layout2.language, '$path.language');
      expectEqual(layout1.platform, layout2.platform, '$path.platform');
      // Verify LayoutEntry
      verifyEach(layout1.entries.entries, layout2.entries.entries, '$path.entries',
        (MapEntry<String, LayoutEntry> entry1, MapEntry<String, LayoutEntry> entry2, String path) {
          expectEqual(entry1.key, entry2.key, '$path.key');
          verifyEach(entry1.value.printables, entry2.value.printables, '$path.printables',
            (String printable1, String printable2, String path) {
              expectEqual(printable1, printable2, path);
            });
          expectEqual(entry1.value.deadMasks, entry2.value.deadMasks, '$path.deadMasks');
        });
    });
}

String _layoutPlatformToString(LayoutPlatform platform) {
  switch (platform) {
    case LayoutPlatform.win:
      return 'win';
    case LayoutPlatform.linux:
      return 'linux';
    case LayoutPlatform.darwin:
      return 'darwin';
  }
}

/// Marshall a store into a JSON.
///
/// The JSON is not to be parsed by scripts, but read by human.
Map<String, dynamic> jsonifyStore(LayoutStore store) {
  final Map<String, dynamic> storeJson = <String, dynamic>{};
  storeJson['goals'] = Map<String, dynamic>.from(store.goals);
  storeJson['layouts'] = store.layouts.map((Layout layout) {
    final Map<String, dynamic> layoutJson = <String, dynamic>{};
    layoutJson['language'] = layout.language;
    layoutJson['platform'] = _layoutPlatformToString(layout.platform);
    layoutJson['entries'] = Map<String, dynamic>.fromIterables(
      layout.entries.keys,
      layout.entries.values.map((LayoutEntry entry) =>
        <dynamic>[...entry.printables, entry.deadMasks]
      ),
    );
    return layoutJson;
  }).toList();
  return storeJson;
}
