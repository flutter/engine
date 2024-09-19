// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const String _templateKey = 'json_injector:template';

bool _isListOf<T>(List<dynamic> list) {
  for (final item in list) {
    if (item is! T) {
      return false;
    }
  }
  return list.isNotEmpty;
}

dynamic _applyTemplate(dynamic item, Map<dynamic, dynamic>? templates) {
  if (item is Map) {
    if (item.containsKey(_templateKey)) {
      final String templateName = item[_templateKey] as String;
      final Map<dynamic, dynamic>? template =
          templates?[templateName] as Map<dynamic, dynamic>?;

      if (template == null) {
        throw StateError('unknown template: $templateName');
      }

      final Map<dynamic, dynamic> result = {};
      for (final x in item.keys) {
        if (x != _templateKey) {
          result[x] = item[x];
        }
      }
      for (final x in template.keys) {
        result[x] = template[x];
      }
      return result;
    } else {
      return item;
    }
  } else if (item is List) {
  } else {
    return item;
  }
}

Object? inject(Object? json, Object? injector,
    {String? nameKey, Map<dynamic, dynamic>? templates}) {
  Object? recurse(Object? x, Object? y) =>
      inject(x, y, nameKey: nameKey, templates: templates);
  if (json is Map && injector is Map) {
    final result = <dynamic, dynamic>{};
    for (final key in json.keys) {
      if (!injector.containsKey(key)) {
        result[key] = json[key];
      }
    }
    for (final key in injector.keys) {
      if (key == _templateKey) {
        final String templateName = injector[key] as String;
        final Map<dynamic, dynamic>? template =
            templates?[templateName] as Map<dynamic, dynamic>?;
        if (template == null) {
          throw StateError('unknown template: $templateName');
        }
        for (final templateKey in template.keys) {
          result[templateKey] = template[templateKey];
        }
      } else if (json.containsKey(key)) {
        result[key] = recurse(json[key], injector[key]);
      } else {
        result[key] = _applyTemplate(injector[key], templates);
      }
    }
    return result;
  }
  if (json is List<dynamic> &&
      injector is List<dynamic> &&
      nameKey != null &&
      _isListOf<Map<dynamic, dynamic>>(json) &&
      _isListOf<Map<dynamic, dynamic>>(injector)) {
    final Map<String, Object?> jsonList = {};
    final Map<String, Object?> injectorList = {};

    for (final item in json) {
      final Map<dynamic, dynamic> map = item as Map;
      jsonList[map[nameKey] as String] = item;
    }
    for (final item in injector) {
      final Map<dynamic, dynamic> map = item as Map;
      injectorList[map[nameKey] as String] = item;
    }
    final joined = recurse(jsonList, injectorList) as Map?;
    return joined?.values.toList();
  }

  return json;
}
