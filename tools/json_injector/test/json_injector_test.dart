import 'package:json_injector/json_injector.dart';
import 'package:test/test.dart';

bool _deepEquals(Object? x, Object? y) {
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

void main() {
  test('noop', () {
    const json = {
      'configurations': <Object?>[],
    };
    final result = inject(json, {});

    expect(_deepEquals(json, result), isTrue);
  });

  test('simple inject', () {
    const json = {
      'configurations': <Object?>[],
    };
    const injector = {
      'bar': <Object?>[],
    };

    expect(
        _deepEquals(inject(json, injector), {
          'configurations': <Object?>[],
          'bar': <Object?>[],
        }),
        isTrue);
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
        _deepEquals(inject(json, injector), {
          'configurations': {
            'foo': 1,
            'bar': 2,
          },
        }),
        isTrue);
  });
}
