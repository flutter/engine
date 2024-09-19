Object? inject(Object? json, Object? injector, {String? nameKey}) {
  if (json is Map && injector is Map) {
    final result = <dynamic, dynamic>{};
    for (final key in json.keys) {
      if (!injector.containsKey(key)) {
        result[key] = json[key];
      }
    }
    for (final key in injector.keys) {
      if (json.containsKey(key)) {
        result[key] = inject(json[key], injector[key], nameKey: nameKey);
      } else {
        result[key] = injector[key];
      }
    }
    return result;
  } if (json is List<Map> && injector is List<Map> && nameKey != null) {
    final Map<String, Object?> jsonList = {};
    final Map<String, Object?> injectorList = {};
    for (final item in json) {
      jsonList[nameKey] = item;
    }
    for (final item in injector) {
      injectorList[nameKey] = item;
    }
    final joined = inject(jsonList, injectorList, nameKey: nameKey) as Map?;
    return joined?.values.toList();
  }

  return json;
}
