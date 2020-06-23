// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:js';

import 'package:mockito/mockito.dart';
import 'package:test/test.dart';

import 'package:ui/src/engine.dart';

void main() {
  SkiaObjects.maximumCacheSize = 4;
  group(ResurrectableSkiaObject, () {
    test('implements create, cache, delete, resurrect, delete lifecycle', () {
      int addPostFrameCallbackCount = 0;

      MockRasterizer mockRasterizer = MockRasterizer();
      when(mockRasterizer.addPostFrameCallback(any)).thenAnswer((_) {
        addPostFrameCallbackCount++;
      });
      window.rasterizer = mockRasterizer;

      // Trigger first create
      final TestSkiaObject testObject = TestSkiaObject();
      expect(SkiaObjects.resurrectableObjects.single, testObject);
      expect(testObject.createDefaultCount, 1);
      expect(testObject.resurrectCount, 0);
      expect(testObject.deleteCount, 0);

      // Check that the getter does not have side-effects
      final JsObject skiaObject1 = testObject.skiaObject;
      expect(skiaObject1, isNotNull);
      expect(SkiaObjects.resurrectableObjects.single, testObject);
      expect(testObject.createDefaultCount, 1);
      expect(testObject.resurrectCount, 0);
      expect(testObject.deleteCount, 0);

      // Trigger first delete
      SkiaObjects.postFrameCleanUp();
      expect(SkiaObjects.resurrectableObjects, isEmpty);
      expect(addPostFrameCallbackCount, 1);
      expect(testObject.createDefaultCount, 1);
      expect(testObject.resurrectCount, 0);
      expect(testObject.deleteCount, 1);

      // Trigger resurrect
      final JsObject skiaObject2 = testObject.skiaObject;
      expect(skiaObject2, isNotNull);
      expect(skiaObject2, isNot(same(skiaObject1)));
      expect(SkiaObjects.resurrectableObjects.single, testObject);
      expect(addPostFrameCallbackCount, 1);
      expect(testObject.createDefaultCount, 1);
      expect(testObject.resurrectCount, 1);
      expect(testObject.deleteCount, 1);

      // Trigger final delete
      SkiaObjects.postFrameCleanUp();
      expect(SkiaObjects.resurrectableObjects, isEmpty);
      expect(addPostFrameCallbackCount, 1);
      expect(testObject.createDefaultCount, 1);
      expect(testObject.resurrectCount, 1);
      expect(testObject.deleteCount, 2);
    });
  });

  group(OneShotSkiaObject, () {
    test('is added to SkiaObjects cache', () {
      int deleteCount = 0;
      JsObject _makeJsObject() {
        return JsObject.jsify({
          'delete': allowInterop(() {
            deleteCount++;
          }),
        });
      }

      OneShotSkiaObject object1 = OneShotSkiaObject(_makeJsObject());
      expect(SkiaObjects.objectCache.length, 1);
      expect(SkiaObjects.objectCache.contains(object1), isTrue);

      OneShotSkiaObject object2 = OneShotSkiaObject(_makeJsObject());
      expect(SkiaObjects.objectCache.length, 2);
      expect(SkiaObjects.objectCache.contains(object2), isTrue);

      SkiaObjects.postFrameCleanUp();
      expect(SkiaObjects.objectCache.length, 2);
      expect(SkiaObjects.objectCache.contains(object1), isTrue);
      expect(SkiaObjects.objectCache.contains(object2), isTrue);

      OneShotSkiaObject object3 = OneShotSkiaObject(_makeJsObject());
      OneShotSkiaObject object4 = OneShotSkiaObject(_makeJsObject());
      OneShotSkiaObject object5 = OneShotSkiaObject(_makeJsObject());
      expect(SkiaObjects.objectCache.length, 5);
      expect(SkiaObjects.cachesToResize.length, 1);

      SkiaObjects.postFrameCleanUp();
      expect(deleteCount, 2);
      expect(SkiaObjects.objectCache.length, 3);
      expect(SkiaObjects.objectCache.contains(object1), isFalse);
      expect(SkiaObjects.objectCache.contains(object2), isFalse);
    });
  });
}

class TestSkiaObject extends ResurrectableSkiaObject {
  int createDefaultCount = 0;
  int resurrectCount = 0;
  int deleteCount = 0;

  JsObject _makeJsObject() {
    return JsObject.jsify({
      'delete': allowInterop(() {
        deleteCount++;
      }),
    });
  }

  @override
  JsObject createDefault() {
    createDefaultCount++;
    return _makeJsObject();
  }

  @override
  JsObject resurrect() {
    resurrectCount++;
    return _makeJsObject();
  }
}

class MockRasterizer extends Mock implements Rasterizer {}
