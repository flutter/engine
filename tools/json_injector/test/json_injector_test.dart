import 'package:json_injector/json_injector.dart';
import 'package:test/test.dart';

bool _deepEquals(dynamic x, dynamic y) {
  if (x is Map && y is Map) {
    if (x.length != y.length) {
      return false;
    }

    for (final key in x.keys) {
      if (!_deepEquals(x[key], y[key])) {
        return false;
      }
    }
    return true;
  } else if (x is List && y is List) {
    if (x.length != y.length) {
      return false;
    }

    for (int i = 0; i < x.length; i++) {
      if (_deepEquals(x[i], y[i])) {
        return false;
      }
    }
    return true;
  } else {
    return x == y;
  }
}

class _DeepMatcher extends Matcher {
  _DeepMatcher(this._target);

  final dynamic _target;

  @override
  Description describe(Description description) {
    description.add('equals $_target');
    return description;
  }

  @override
  bool matches(dynamic item, Map<dynamic, dynamic> matchState) =>
      _deepEquals(item, _target);
}

Matcher _isDeepEquals(dynamic x) => _DeepMatcher(x);

void main() {
  test('noop', () {
    const json = {
      'configurations': <Object?>[],
    };
    final result = inject(json, {});

    expect(result, _isDeepEquals(json));
  });

  test('simple inject', () {
    const json = {
      'configurations': <Object?>[],
    };
    const injector = {
      'bar': <Object?>[],
    };

    expect(
        inject(json, injector),
        _isDeepEquals({
          'configurations': <Object?>[],
          'bar': <Object?>[],
        }));
  });

  test('simple recurse', () {
    const json = {
      'configurations': {
        'foo': 1,
      },
    };
    const injector = {
      'configurations': {
        'bar': 2,
      },
    };

    expect(
        inject(json, injector),
        _isDeepEquals({
          'configurations': {
            'foo': 1,
            'bar': 2,
          },
        }));
  });
}
