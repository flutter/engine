import 'dart:convert' show jsonDecode;

Object? inject(Object? json, Object? injector) {
  if (json is Map && injector is Map) {
    final result = <dynamic, dynamic>{};
    for (final key in json.keys) {
      if (!injector.containsKey(key)) {
        result[key] = json[key];
      }
    }
    for (final key in injector.keys) {
      if (json.containsKey(key)) {
        result[key] = inject(json[key], injector[key]);
      } else {
        result[key] = injector[key];
      }
    }
    return result;
  }

  return json;
}
