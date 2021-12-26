// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Key of the flutter application id.
const Symbol kApplicationId = #flutter.io.application_id;

/// The default id of the flutter application.
const Object kDefaultApplicationId = 0;

/// The 'Application' represents a flutter application, each flutter application will have its own 'ui.window', 'ui.PlatformDispatcher' and so on. Each application will have its own element tree, render object tree, and layer tree. In the engine side, each application will have a corresponding 'flutte::Shell'. There can be multiple flutter applications in the root isolate.
class Application {

  /// Get the current 'Application' according to the application id in the zone.
  static Application get current {
    return _application;
  }

  /// Get the 'Application' according to the specified application id.
  static Application fromId(Object applicationId) {
    assert(applicationId == kDefaultApplicationId);
    return _application;
  }

  Application._(
    PlatformDispatcher platformDispatcher,
    SingletonFlutterWindow window,
    ChannelBuffers channelBuffers):
    _zone = Zone.current.fork(zoneValues: <Object, Object>{kApplicationId:kDefaultApplicationId}),
    _platformDispatcher = platformDispatcher,
    _window = window,
    _channelBuffers = channelBuffers;

  /// The id of the flutter application.
  Object get id => kDefaultApplicationId;

  /// The id of the flutter application. The logic of the 'Application' should run in there.
  Zone get zone => _zone;
  Zone _zone;

  ///
  PlatformDispatcher get platformDispatcher => _platformDispatcher;
  PlatformDispatcher _platformDispatcher;

  ///
  SingletonFlutterWindow get window => _window;
  SingletonFlutterWindow _window;

  ///
  ChannelBuffers get channelBuffers => _channelBuffers;
  ChannelBuffers _channelBuffers;

  Map<Object, Object?> _values = <Object, Object?>{};

  /// Whether the default flutter application.
  bool isDefaultApplication() => true;

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
}

Application _application = Application._(PlatformDispatcher.instance, window, channelBuffers);
