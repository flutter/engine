// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.12
part of dart.ui;

///
const Symbol kApplicationId = #flutter.io.application_id;

///
const Object kDefaultApplicationId = 0;

///
Map<Object, Application> _applications = <Object, Application>{};

///
Set<Object> _exitedApplicationIds = <Object>{};

///
class Application {

  ///
  static Application get current {
    final Object applicationId = Zone.current[kApplicationId] ?? kDefaultApplicationId;
    assert(applicationId != null);
    return Application.fromId(applicationId);
  }

  ///
  static Application fromId(Object applicationId) {
    assert(applicationId != null);
    assert(!_exitedApplicationIds.contains(applicationId));
    final Application result = _applications[applicationId] ??= Application._(applicationId);
    return result;
  }



  Application._(Object id):
               _id = id,
               _zone = Zone.current.fork(zoneValues:{kApplicationId:id});

  ///
  Object get id => _id;
  Object _id;

  ///
  Zone get zone => _zone;
  Zone _zone;

  Map<Object, Object?> _values = <Object, Object?>{};

  ///
  bool isDefaultApplication() => _id == kDefaultApplicationId;

  ///
  void put(Object key, Object? value) => _values[key] = value;

  ///
  T? find<T>(Object key) => _values[key] as T?;

  ///
  T get<T extends Object>(Object key, T Function() ifAbsent) {
    _values[key] ??= ifAbsent();
    return _values[key] as T;
  }

  ///
  bool exit() {
    assert(!_exitedApplicationIds.contains(_id));
    _values.clear();
    _applications.remove(_id);
    _exitedApplicationIds.add(_id);
    return true;
  }
}
