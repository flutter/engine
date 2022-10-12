// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' show base64, utf8;
import 'dart:io' hide BytesBuilder;
import 'dart:typed_data';

import 'package:meta/meta.dart' show immutable;

enum LayoutPlatform {
  win,
  linux,
  darwin,
}

const int _kPrintableLength = 4;

@immutable
class LayoutEntry {
  LayoutEntry(this.printables, this.deadMasks)
    : assert(printables.length == _kPrintableLength);

  final List<String> printables;
  final int deadMasks;

  static final LayoutEntry empty = LayoutEntry(
    const <String>['', '', '', ''], 0xf);
}

@immutable
class Layout {
  const Layout(this.language, this.platform, this.entries);

  final String language;
  final LayoutPlatform platform;
  final Map<String, LayoutEntry> entries;
}

@immutable
class LayoutStore {
  const LayoutStore(this.goals, this.layouts);

  final Map<String, String?> goals;
  final List<Layout> layouts;
}

class _ByteStream {
  _ByteStream(this.buffer)
    : _data = buffer.asByteData(), _offset = 0;

  final ByteBuffer buffer;
  final ByteData _data;

  int get offest => _offset;
  int _offset;

  int readUint8() {
    final int result = _data.getUint8(_offset);
    _offset += 1;
    return result;
  }

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

  String? readNullableString() {
    final int length = _data.getUint8(_offset);
    if (length == 0) {
      _offset += 1;
      return null;
    }
    return readString();
  }
}

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

void verifyLayoutStoreEqual(LayoutStore store1, LayoutStore store2) {
  void expectEqual<T>(T a, T b, String path) {
    if (a != b) {
      throw Exception('Error verifying unmarshalled layout on $path: $a != $b');
    }
  }
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
