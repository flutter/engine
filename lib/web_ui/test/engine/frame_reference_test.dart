// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('FrameScoped', () {
    test('lifecycle', () {
      final v1 = TestFrameScoped();
      final v2 = TestFrameScoped();
      expect(TestFrameScoped.disposedList, isEmpty);

      endFrameScope();

      expect(
        TestFrameScoped.disposedList,
        [v1, v2],
      );
    });
  });

  group('FrameScopedValue', () {
    test('Holds a value until end of frame', () {
      final v1 = FrameScopedValue<int>(42);
      final v2 = FrameScopedValue<int>(123);

      expect(v1.value, 42);
      expect(v2.value, 123);

      endFrameScope();

      expect(v1.value, isNull);
      expect(v2.value, isNull);
    });
  });

  group('CrossFrameCache', () {
    test('Reuse returns no object when cache empty', () {
      final CrossFrameCache<TestItem> cache = CrossFrameCache<TestItem>();
      cache.commitFrame();
      final TestItem? requestedItem = cache.reuse('item1');
      expect(requestedItem, null);
    });

    test('Reuses object across frames', () {
      final CrossFrameCache<TestItem> cache = CrossFrameCache<TestItem>();
      final TestItem testItem1 = TestItem('item1');
      cache.cache(testItem1.label, testItem1);
      cache.commitFrame();
      TestItem? requestedItem = cache.reuse('item1');
      expect(requestedItem, testItem1);
      requestedItem = cache.reuse('item1');
      expect(requestedItem, null);
    });

    test('Reuses objects that have same key across frames', () {
      final CrossFrameCache<TestItem> cache = CrossFrameCache<TestItem>();
      final TestItem testItem1 = TestItem('sameLabel');
      final TestItem testItem2 = TestItem('sameLabel');
      final TestItem testItemX = TestItem('X');
      cache.cache(testItem1.label, testItem1);
      cache.cache(testItemX.label, testItemX);
      cache.cache(testItem2.label, testItem2);
      cache.commitFrame();
      TestItem? requestedItem = cache.reuse('sameLabel');
      expect(requestedItem, testItem1);
      requestedItem = cache.reuse('sameLabel');
      expect(requestedItem, testItem2);
      requestedItem = cache.reuse('sameLabel');
      expect(requestedItem, null);
    });

    test("Values don't survive beyond next frame", () {
      final CrossFrameCache<TestItem> cache = CrossFrameCache<TestItem>();
      final TestItem testItem1 = TestItem('item1');
      cache.cache(testItem1.label, testItem1);
      cache.commitFrame();
      cache.commitFrame();
      final TestItem? requestedItem = cache.reuse('item1');
      expect(requestedItem, null);
    });

    test('Values are evicted when not reused', () {
      final Set<TestItem> evictedItems = <TestItem>{};
      final CrossFrameCache<TestItem> cache = CrossFrameCache<TestItem>();
      final TestItem testItem1 = TestItem('item1');
      final TestItem testItem2 = TestItem('item2');
      cache.cache(testItem1.label, testItem1, (TestItem item) {evictedItems.add(item);});
      cache.cache(testItem2.label, testItem2, (TestItem item) {evictedItems.add(item);});
      cache.commitFrame();
      expect(evictedItems.length, 0);
      cache.reuse('item2');
      cache.commitFrame();
      expect(evictedItems.contains(testItem1), isTrue);
      expect(evictedItems.contains(testItem2), isFalse);
    });
  });
}

class TestItem {
  TestItem(this.label);
  final String label;
}

class TestFrameScoped extends FrameScoped {
  static final disposedList = <FrameScoped>[];

  @override
  void disposeFrameResources() {
    disposedList.add(this);
  }
}
