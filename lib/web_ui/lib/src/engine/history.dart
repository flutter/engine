// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.10
part of engine;

const MethodCall _popRouteMethodCall = MethodCall('popRoute');
const String _kFlutterTag = 'flutter';
const String _kOriginTag = 'origin';

Map<String, dynamic> _wrapOriginState(dynamic state) {
  return <String, dynamic>{_kOriginTag: true, 'state': state};
}
dynamic _unWrapOriginState(dynamic state) {
  assert(_isOriginEntry(state));
  final Map<dynamic, dynamic> originState = state as Map<dynamic, dynamic>;
  return originState['state'];
}
Map<String, bool> _flutterState = <String, bool>{_kFlutterTag: true};

/// The origin entry is the history entry that the Flutter app landed on. It's
/// created by the browser when the user navigates to the url of the app.
bool _isOriginEntry(dynamic state) {
  return state is Map && state[_kOriginTag] == true;
}

/// The flutter entry is a history entry that we maintain on top of the origin
/// entry. It allows us to catch popstate events when the user hits the back
/// button.
bool _isFlutterEntry(dynamic state) {
  return state is Map && state[_kFlutterTag] == true;
}

/// An abstract class that provides the API for [EngineWindow] to delegate its
/// navigating events.
///
/// Subclasses will have access to [BrowserHistory.locationStrategy] to
/// interact with the html browser history and should come up with their own
/// ways to manage the states in the browser history.
///
/// See also:
///
///  * [SingleEntryBrowserHistory]: whic creates a single fake browser history
///    entry and delegates all browser navigating events to the flutter
///    framework.
///  * [MultiEntriesBrowserHistory]: which creates a set of states that records
///    the navigating events happened in the framework.
abstract class BrowserHistory {
  ui.VoidCallback? _unsubscribe;

  /// The strategy to interact with browser history.
  LocationStrategy? get locationStrategy => _locationStrategy;
  LocationStrategy? _locationStrategy;
  /// Updates the strategy.
  ///
  /// This method will also remove any previous modifications to the html
  /// browser history and start anew.
  Future<void> setLocationStrategy(LocationStrategy? strategy) async {
    if (strategy != _locationStrategy) {
      await _tearoffStrategy(_locationStrategy);
      _locationStrategy = strategy;
      await _setupStrategy(_locationStrategy);
    }
  }

  /// Exit this application and return to the previous page.
  Future<void> exit() {
    if (_locationStrategy != null) {
      _tearoffStrategy(_locationStrategy);
      // Now the history should be in the original state, back one more time to
      // exit the application.
      final Future<void> backFuture = _locationStrategy!.back();
      _locationStrategy = null;
      return backFuture;
    }
    return Future<void>.value();
  }

  /// This method does the same thing as the browser back button.
  Future<void> back() {
    if (locationStrategy != null) {
      return locationStrategy!.back();
    }
    return Future<void>.value();
  }

  /// The path of the current location of the user's browser.
  String get currentPath => locationStrategy?.path ?? '/';

  /// Update the url with the given [routeName] and [state].
  void setRouteName(String? routeName, {dynamic? state});

  /// A callback method to handle browser backward or forward button.
  ///
  /// Subclasses should send appropriate system messages to the framework to
  /// update the flutter applications accordingly.
  @protected
  void onPopState(covariant html.PopStateEvent event);

  /// Sets up any prerequisites to use this browser history class.
  @protected
  Future<void> setup() => Future<void>.value();

  /// Restore any modifications to the html browser history during the lifetime
  /// of this class.
  @protected
  Future<void> tearDown() => Future<void>.value();

  Future<void> _setupStrategy(LocationStrategy? strategy) async {
    if (strategy == null) {
      return;
    }
    _unsubscribe = strategy.onPopState(onPopState as dynamic Function(html.Event));
    await setup();
  }

  Future<void> _tearoffStrategy(LocationStrategy? strategy) async {
    if (strategy == null) {
      return;
    }

    assert(_unsubscribe != null);
    _unsubscribe!();
    _unsubscribe = null;

    await tearDown();
  }
}

/// A browser history class that creates a set of browser history entries to
/// support browser backward and forward button natively.
///
/// This class creates a browser history entry every time the framework reports
/// route changes so that users can use the backward and forward buttons to
/// navigate within the application.
class MultiEntriesBrowserHistory extends BrowserHistory {
  int _flutterHistoryCount = 0;
  dynamic get _currentState => html.window.history.state;

  dynamic _tagWithSerialCount(dynamic originialState, int count) {
    return <dynamic, dynamic> {
      'serialCount': count,
      'state': originialState,
    };
  }

  bool _hasSerialCount(dynamic state) {
    return state is Map && state['serialCount'] != null;
  }

  void _updateHistoryCount() {
    if (_hasSerialCount(_currentState)) {
      _flutterHistoryCount = _currentState['serialCount'];
    } else {
      _flutterHistoryCount = 0;
    }
  }

  @override
  void setRouteName(String? routeName, {dynamic? state}) {
    if (locationStrategy != null && routeName != null) {
      _updateHistoryCount();
      _flutterHistoryCount += 1;
      locationStrategy!.pushState(_tagWithSerialCount(state, _flutterHistoryCount), 'flutter', routeName!);
    }
  }

  @override
  void onPopState(covariant html.PopStateEvent event) {
    if ((_hasSerialCount(event.state)) &&
        window._onPlatformMessage != null) {
      _flutterHistoryCount = event.state['serialCount'];
      window.invokeOnPlatformMessage(
        'flutter/navigation',
        const JSONMethodCodec().encodeMethodCall(
          MethodCall('pushRouteInformation', <dynamic, dynamic>{
            'location': currentPath,
            'state': event.state,
          })
        ),
        (_) {},
      );
    }
  }

  @override
  Future<void> setup() {
    _flutterHistoryCount = 0;
    if (locationStrategy != null) {
      locationStrategy!.replaceState(
        _tagWithSerialCount(_currentState, _flutterHistoryCount),
        'flutter',
        currentPath
      );
    }
    return Future<void>.value();
  }

  @override
  Future<void> tearDown() async {
    if (_hasSerialCount(_currentState)) {
      if (_currentState['serialCount'] > 0) {
        final int currentFlutterStateSerialCount = _currentState['serialCount'] as int;
        await locationStrategy!.back(count: currentFlutterStateSerialCount);
      }
      // Unwrap state.
      locationStrategy!.replaceState(
        _currentState['state'],
        'flutter',
        currentPath,
      );
    }
  }
}

/// The [SingleEntryBrowserHistory] class is responsible for integrating Flutter
/// Web apps with the browser history so that the back button works as expected.
///
/// It does that by always keeping a single entry (conventionally called the
/// "flutter" entry) at the top of the browser history. That way, the browser's
/// back button always triggers a `popstate` event and never closes the app (we
/// close the app programmatically by calling [SystemNavigator.pop] when there
/// are no more app routes to be popped).
///
/// There should only be one global instance of this class.
class SingleEntryBrowserHistory extends BrowserHistory {
  dynamic get _currentState => html.window.history.state;

  @override
  void setRouteName(String? routeName, {dynamic? state}) {
    if (locationStrategy != null) {
      _setupFlutterEntry(locationStrategy!, replace: true, path: routeName);
    }
  }

  String? _userProvidedRouteName;
  @override
  void onPopState(covariant html.PopStateEvent event) {
    if (_isOriginEntry(event.state)) {
      _setupFlutterEntry(_locationStrategy!);

      // 2. Send a 'popRoute' platform message so the app can handle it accordingly.
      if (window._onPlatformMessage != null) {
        window.invokeOnPlatformMessage(
          'flutter/navigation',
          const JSONMethodCodec().encodeMethodCall(_popRouteMethodCall),
          (_) {},
        );
      }
    } else if (_isFlutterEntry(event.state)) {
      // We get into this scenario when the user changes the url manually. It
      // causes a new entry to be pushed on top of our "flutter" one. When this
      // happens it first goes to the "else" section below where we capture the
      // path into `_userProvidedRouteName` then trigger a history back which
      // brings us here.
      assert(_userProvidedRouteName != null);

      final String newRouteName = _userProvidedRouteName!;
      _userProvidedRouteName = null;

      // Send a 'pushRoute' platform message so the app handles it accordingly.
      if (window._onPlatformMessage != null) {
        window.invokeOnPlatformMessage(
          'flutter/navigation',
          const JSONMethodCodec().encodeMethodCall(
            MethodCall('pushRoute', newRouteName),
          ),
          (_) {},
        );
      }
    } else {
      // The user has pushed a new entry on top of our flutter entry. This could
      // happen when the user modifies the hash part of the url directly, for
      // example.

      // 1. We first capture the user's desired path.
      _userProvidedRouteName = currentPath;

      // 2. Then we remove the new entry.
      // This will take us back to our "flutter" entry and it causes a new
      // popstate event that will be handled in the "else if" section above.
      _locationStrategy!.back();
    }
  }

  /// This method should be called when the Origin Entry is active. It just
  /// replaces the state of the entry so that we can recognize it later using
  /// [_isOriginEntry] inside [_popStateListener].
  void _setupOriginEntry(LocationStrategy strategy) {
    assert(strategy != null); // ignore: unnecessary_null_comparison
    strategy.replaceState(_wrapOriginState(_currentState), 'origin', '');
  }

  /// This method is used manipulate the Flutter Entry which is always the
  /// active entry while the Flutter app is running.
  void _setupFlutterEntry(
    LocationStrategy strategy, {
    bool replace = false,
    String? path,
  }) {
    assert(strategy != null); // ignore: unnecessary_null_comparison
    path ??= currentPath;
    if (replace) {
      strategy.replaceState(_flutterState, 'flutter', path);
    } else {
      strategy.pushState(_flutterState, 'flutter', path);
    }
  }

  @override
  Future<void> setup() {
    if (_isFlutterEntry(html.window.history.state)) {
      // This could happen if the user, for example, refreshes the page. They
      // will land directly on the "flutter" entry, so there's no need to setup
      // the "origin" and "flutter" entries, we can safely assume they are
      // already setup.
    } else {
      _setupOriginEntry(locationStrategy!);
      _setupFlutterEntry(locationStrategy!, replace: false, path: currentPath);
    }
    return Future<void>.value();
  }

  @override
  Future<void> tearDown() async {
    if (locationStrategy != null) {
      // We need to remove the flutter entry that we pushed in setup.
      await locationStrategy!.back();
      // Restores original state.
      print('_currentState ${_currentState}, currentPath ${currentPath}');
      locationStrategy!.replaceState(_unWrapOriginState(_currentState), 'flutter', currentPath);
    }
  }
}
