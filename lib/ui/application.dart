// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.12
part of dart.ui;

/// Key of the flutter application id.
const Symbol kApplicationId = #flutter.io.application_id;

/// The default id of the flutter application.
const Object kDefaultApplicationId = 0;

Map<Object, Application> _applications = <Object, Application>{};

Set<Object> _exitedApplicationIds = <Object>{};

/// The 'Application' represents a flutter application, each flutter application will have its own 'ui.window', 'ui.PlatformDispatcher' and so on. Each application will have its own element tree, render object tree, and layer tree. In the engine side, each application will have a corresponding 'flutte::Shell'. There can be multiple flutter applications in the root isolate.
class Application {

  /// Get the current 'Application' according to the application id in the zone.
  static Application get current {
    final Object? applicationId = Zone.current[kApplicationId] ?? kDefaultApplicationId;
    return Application.fromId(applicationId!);
  }

  /// Get the 'Application' according to the specified application id.
  static Application fromId(Object applicationId) {
    assert(!_exitedApplicationIds.contains(applicationId));
    final Application result = _applications[applicationId] ??= Application._(applicationId);
    return result;
  }

  Application._(Object id) :
    _id = id,
    _zone = Zone.current.fork(zoneValues: <Object, Object>{kApplicationId:id}),
    _platformDispatcher = PlatformDispatcher._(applicationId: id),
    _channelBuffers = ChannelBuffers() {
    _window = SingletonFlutterWindow._(0, _platformDispatcher);
  }

  /// The id of the flutter application.
  Object get id => _id;
  Object _id;

  /// The id of the flutter application. The logic of the 'Application' should run in there.
  Zone get zone => _zone;
  Zone _zone;

  ///
  PlatformDispatcher get platformDispatcher => _platformDispatcher;
  PlatformDispatcher _platformDispatcher;

  ///
  SingletonFlutterWindow get window => _window;
  late SingletonFlutterWindow _window;

  ///
  ChannelBuffers get channelBuffers => _channelBuffers;
  ChannelBuffers _channelBuffers;

  Map<Object, Object?> _values = <Object, Object?>{};

  /// Whether the default flutter application.
  bool isDefaultApplication() => _id == kDefaultApplicationId;

  /// Put the object for the key.
  void put(Object key, Object? value) => _values[key] = value;

  /// find the object for the key, return null if not found.
  T? find<T>(Object key) => _values[key] as T?;

  /// Get the object according to the key. If the object does not exist, it will be created by 'ifAbsent'.
  T get<T extends Object>(Object key, T Function() ifAbsent) {
    T? result = find(key);
    if (result == null) {
      result = ifAbsent();
      put(key, result);
    }
    return result;
  }

  // called when the application exits.
  bool _exit() {
    assert(!_exitedApplicationIds.contains(_id));
    _values.clear();
    _applications.remove(_id);
    _exitedApplicationIds.add(_id);
    return true;
  }
}
