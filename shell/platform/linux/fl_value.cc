// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_value.h"

#include <gmodule.h>

struct _FlValue {
  FlValueType type;
  gatomicrefcount ref_count;
};

typedef struct {
  FlValue parent;
  bool value;
} FlValueBool;

typedef struct {
  FlValue parent;
  int64_t value;
} FlValueInt;

typedef struct {
  FlValue parent;
  double value;
} FlValueDouble;

typedef struct {
  FlValue parent;
  gchar* value;
} FlValueString;

typedef struct {
  FlValue parent;
  uint8_t* values;
  size_t values_length;
} FlValueUint8List;

typedef struct {
  FlValue parent;
  int32_t* values;
  size_t values_length;
} FlValueInt32List;

typedef struct {
  FlValue parent;
  int64_t* values;
  size_t values_length;
} FlValueInt64List;

typedef struct {
  FlValue parent;
  double* values;
  size_t values_length;
} FlValueFloatList;

typedef struct {
  FlValue parent;
  GPtrArray* values;
} FlValueList;

typedef struct {
  FlValue parent;
  GPtrArray* keys;
  GPtrArray* values;
} FlValueMap;

static FlValue* fl_value_new(FlValueType type, size_t size) {
  FlValue* self = static_cast<FlValue*>(g_malloc0(size));
  self->type = type;
  g_atomic_ref_count_init(&self->ref_count);
  return self;
}

// Helper function to match GDestroyNotify type
static void fl_value_destroy(gpointer value) {
  fl_value_unref(static_cast<FlValue*>(value));
}

// Find the index of a key in a FlValueMap
static ssize_t fl_value_map_lookup_index(FlValue* self, FlValue* key) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_MAP, -1);

  for (size_t i = 0; i < fl_value_get_length(self); i++) {
    FlValue* k = fl_value_map_get_key(self, i);
    if (fl_value_equal(k, key))
      return i;
  }
  return -1;
}

G_MODULE_EXPORT FlValue* fl_value_null_new() {
  return fl_value_new(FL_VALUE_TYPE_NULL, sizeof(FlValue));
}

G_MODULE_EXPORT FlValue* fl_value_bool_new(bool value) {
  FlValueBool* self = reinterpret_cast<FlValueBool*>(
      fl_value_new(FL_VALUE_TYPE_BOOL, sizeof(FlValueBool)));
  self->value = value ? true : false;
  return reinterpret_cast<FlValue*>(self);
}

G_MODULE_EXPORT FlValue* fl_value_int_new(int64_t value) {
  FlValueInt* self = reinterpret_cast<FlValueInt*>(
      fl_value_new(FL_VALUE_TYPE_INT, sizeof(FlValueInt)));
  self->value = value;
  return reinterpret_cast<FlValue*>(self);
}

G_MODULE_EXPORT FlValue* fl_value_float_new(double value) {
  FlValueDouble* self = reinterpret_cast<FlValueDouble*>(
      fl_value_new(FL_VALUE_TYPE_FLOAT, sizeof(FlValueDouble)));
  self->value = value;
  return reinterpret_cast<FlValue*>(self);
}

G_MODULE_EXPORT FlValue* fl_value_string_new(const gchar* value) {
  FlValueString* self = reinterpret_cast<FlValueString*>(
      fl_value_new(FL_VALUE_TYPE_STRING, sizeof(FlValueString)));
  self->value = g_strdup(value);
  return reinterpret_cast<FlValue*>(self);
}

G_MODULE_EXPORT FlValue* fl_value_string_new_sized(const gchar* value,
                                                   size_t value_length) {
  FlValueString* self = reinterpret_cast<FlValueString*>(
      fl_value_new(FL_VALUE_TYPE_STRING, sizeof(FlValueString)));
  self->value =
      value_length == 0 ? g_strdup("") : g_strndup(value, value_length);
  return reinterpret_cast<FlValue*>(self);
}

G_MODULE_EXPORT FlValue* fl_value_uint8_list_new(const uint8_t* data,
                                                 size_t data_length) {
  FlValueUint8List* self = reinterpret_cast<FlValueUint8List*>(
      fl_value_new(FL_VALUE_TYPE_UINT8_LIST, sizeof(FlValueUint8List)));
  self->values_length = data_length;
  self->values = static_cast<uint8_t*>(g_malloc(sizeof(uint8_t) * data_length));
  memcpy(self->values, data, sizeof(uint8_t) * data_length);
  return reinterpret_cast<FlValue*>(self);
}

G_MODULE_EXPORT FlValue* fl_value_uint8_list_new_from_bytes(GBytes* data) {
  gsize length;
  const uint8_t* d =
      static_cast<const uint8_t*>(g_bytes_get_data(data, &length));
  return fl_value_uint8_list_new(d, length);
}

G_MODULE_EXPORT FlValue* fl_value_int32_list_new(const int32_t* data,
                                                 size_t data_length) {
  FlValueInt32List* self = reinterpret_cast<FlValueInt32List*>(
      fl_value_new(FL_VALUE_TYPE_INT32_LIST, sizeof(FlValueInt32List)));
  self->values_length = data_length;
  self->values = static_cast<int32_t*>(g_malloc(sizeof(int32_t) * data_length));
  memcpy(self->values, data, sizeof(int32_t) * data_length);
  return reinterpret_cast<FlValue*>(self);
}

G_MODULE_EXPORT FlValue* fl_value_int64_list_new(const int64_t* data,
                                                 size_t data_length) {
  FlValueInt64List* self = reinterpret_cast<FlValueInt64List*>(
      fl_value_new(FL_VALUE_TYPE_INT64_LIST, sizeof(FlValueInt64List)));
  self->values_length = data_length;
  self->values = static_cast<int64_t*>(g_malloc(sizeof(int64_t) * data_length));
  memcpy(self->values, data, sizeof(int64_t) * data_length);
  return reinterpret_cast<FlValue*>(self);
}

G_MODULE_EXPORT FlValue* fl_value_float_list_new(const double* data,
                                                 size_t data_length) {
  FlValueFloatList* self = reinterpret_cast<FlValueFloatList*>(
      fl_value_new(FL_VALUE_TYPE_FLOAT_LIST, sizeof(FlValueFloatList)));
  self->values_length = data_length;
  self->values = static_cast<double*>(g_malloc(sizeof(double) * data_length));
  memcpy(self->values, data, sizeof(double) * data_length);
  return reinterpret_cast<FlValue*>(self);
}

G_MODULE_EXPORT FlValue* fl_value_list_new() {
  FlValueList* self = reinterpret_cast<FlValueList*>(
      fl_value_new(FL_VALUE_TYPE_LIST, sizeof(FlValueList)));
  self->values = g_ptr_array_new_with_free_func(fl_value_destroy);
  return reinterpret_cast<FlValue*>(self);
}

G_MODULE_EXPORT FlValue* fl_value_map_new() {
  FlValueMap* self = reinterpret_cast<FlValueMap*>(
      fl_value_new(FL_VALUE_TYPE_MAP, sizeof(FlValueMap)));
  self->keys = g_ptr_array_new_with_free_func(fl_value_destroy);
  self->values = g_ptr_array_new_with_free_func(fl_value_destroy);
  return reinterpret_cast<FlValue*>(self);
}

G_MODULE_EXPORT FlValue* fl_value_ref(FlValue* self) {
  g_atomic_ref_count_inc(&self->ref_count);
  return self;
}

G_MODULE_EXPORT void fl_value_unref(FlValue* self) {
  if (!g_atomic_ref_count_dec(&self->ref_count))
    return;

  switch (self->type) {
    case FL_VALUE_TYPE_STRING: {
      FlValueString* v = reinterpret_cast<FlValueString*>(self);
      g_free(v->value);
      break;
    }
    case FL_VALUE_TYPE_UINT8_LIST: {
      FlValueUint8List* v = reinterpret_cast<FlValueUint8List*>(self);
      g_free(v->values);
      break;
    }
    case FL_VALUE_TYPE_INT32_LIST: {
      FlValueInt32List* v = reinterpret_cast<FlValueInt32List*>(self);
      g_free(v->values);
      break;
    }
    case FL_VALUE_TYPE_INT64_LIST: {
      FlValueInt64List* v = reinterpret_cast<FlValueInt64List*>(self);
      g_free(v->values);
      break;
    }
    case FL_VALUE_TYPE_FLOAT_LIST: {
      FlValueFloatList* v = reinterpret_cast<FlValueFloatList*>(self);
      g_free(v->values);
      break;
    }
    case FL_VALUE_TYPE_LIST: {
      FlValueList* v = reinterpret_cast<FlValueList*>(self);
      g_ptr_array_unref(v->values);
      break;
    }
    case FL_VALUE_TYPE_MAP: {
      FlValueMap* v = reinterpret_cast<FlValueMap*>(self);
      g_ptr_array_unref(v->keys);
      g_ptr_array_unref(v->values);
      break;
    }
    case FL_VALUE_TYPE_NULL:
    case FL_VALUE_TYPE_BOOL:
    case FL_VALUE_TYPE_INT:
    case FL_VALUE_TYPE_FLOAT:
      break;
  }
  g_free(self);
}

G_MODULE_EXPORT FlValueType fl_value_get_type(FlValue* self) {
  g_return_val_if_fail(self != nullptr, FL_VALUE_TYPE_NULL);
  return self->type;
}

G_MODULE_EXPORT bool fl_value_equal(FlValue* a, FlValue* b) {
  g_return_val_if_fail(a != nullptr, false);
  g_return_val_if_fail(b != nullptr, false);

  if (a->type != b->type)
    return false;

  switch (a->type) {
    case FL_VALUE_TYPE_NULL:
      return true;
    case FL_VALUE_TYPE_BOOL:
      return fl_value_get_bool(a) == fl_value_get_bool(b);
    case FL_VALUE_TYPE_INT:
      return fl_value_get_int(a) == fl_value_get_int(b);
    case FL_VALUE_TYPE_FLOAT:
      return fl_value_get_float(a) == fl_value_get_float(b);
    case FL_VALUE_TYPE_STRING: {
      FlValueString* a_ = reinterpret_cast<FlValueString*>(a);
      FlValueString* b_ = reinterpret_cast<FlValueString*>(b);
      return g_strcmp0(a_->value, b_->value) == 0;
    }
    case FL_VALUE_TYPE_UINT8_LIST: {
      if (fl_value_get_length(a) != fl_value_get_length(b))
        return false;
      const uint8_t* values_a = fl_value_get_uint8_list(a);
      const uint8_t* values_b = fl_value_get_uint8_list(b);
      for (size_t i = 0; i < fl_value_get_length(a); i++) {
        if (values_a[i] != values_b[i])
          return false;
      }
      return true;
    }
    case FL_VALUE_TYPE_INT32_LIST: {
      if (fl_value_get_length(a) != fl_value_get_length(b))
        return false;
      const int32_t* values_a = fl_value_get_int32_list(a);
      const int32_t* values_b = fl_value_get_int32_list(b);
      for (size_t i = 0; i < fl_value_get_length(a); i++) {
        if (values_a[i] != values_b[i])
          return false;
      }
      return true;
    }
    case FL_VALUE_TYPE_INT64_LIST: {
      if (fl_value_get_length(a) != fl_value_get_length(b))
        return false;
      const int64_t* values_a = fl_value_get_int64_list(a);
      const int64_t* values_b = fl_value_get_int64_list(b);
      for (size_t i = 0; i < fl_value_get_length(a); i++) {
        if (values_a[i] != values_b[i])
          return false;
      }
      return true;
    }
    case FL_VALUE_TYPE_FLOAT_LIST: {
      if (fl_value_get_length(a) != fl_value_get_length(b))
        return false;
      const double* values_a = fl_value_get_float_list(a);
      const double* values_b = fl_value_get_float_list(b);
      for (size_t i = 0; i < fl_value_get_length(a); i++) {
        if (values_a[i] != values_b[i])
          return false;
      }
      return true;
    }
    case FL_VALUE_TYPE_LIST: {
      if (fl_value_get_length(a) != fl_value_get_length(b))
        return false;
      for (size_t i = 0; i < fl_value_get_length(a); i++) {
        if (!fl_value_equal(fl_value_list_get_value(a, i),
                            fl_value_list_get_value(b, i)))
          return false;
      }
      return true;
    }
    case FL_VALUE_TYPE_MAP: {
      if (fl_value_get_length(a) != fl_value_get_length(b))
        return false;
      for (size_t i = 0; i < fl_value_get_length(a); i++) {
        FlValue* key = fl_value_map_get_key(a, i);
        FlValue* value_b = fl_value_map_lookup(b, key);
        if (value_b == nullptr)
          return false;
        FlValue* value_a = fl_value_map_get_value(a, i);
        if (!fl_value_equal(value_a, value_b))
          return false;
      }
      return true;
    }
  }
}

G_MODULE_EXPORT void fl_value_list_add(FlValue* self, FlValue* value) {
  g_return_if_fail(self->type == FL_VALUE_TYPE_LIST);
  g_return_if_fail(value != nullptr);

  fl_value_list_add_take(self, fl_value_ref(value));
}

G_MODULE_EXPORT void fl_value_list_add_take(FlValue* self, FlValue* value) {
  g_return_if_fail(self->type == FL_VALUE_TYPE_LIST);
  g_return_if_fail(value != nullptr);

  FlValueList* v = reinterpret_cast<FlValueList*>(self);
  g_ptr_array_add(v->values, value);
}

G_MODULE_EXPORT void fl_value_map_set(FlValue* self,
                                      FlValue* key,
                                      FlValue* value) {
  g_return_if_fail(self->type == FL_VALUE_TYPE_MAP);
  g_return_if_fail(key != nullptr);
  g_return_if_fail(value != nullptr);

  fl_value_map_set_take(self, fl_value_ref(key), fl_value_ref(value));
}

G_MODULE_EXPORT void fl_value_map_set_take(FlValue* self,
                                           FlValue* key,
                                           FlValue* value) {
  g_return_if_fail(self->type == FL_VALUE_TYPE_MAP);
  g_return_if_fail(key != nullptr);
  g_return_if_fail(value != nullptr);

  FlValueMap* v = reinterpret_cast<FlValueMap*>(self);
  ssize_t index = fl_value_map_lookup_index(self, key);
  if (index < 0) {
    g_ptr_array_add(v->keys, key);
    g_ptr_array_add(v->values, value);
  } else {
    fl_value_destroy(v->keys->pdata[index]);
    v->keys->pdata[index] = key;
    fl_value_destroy(v->values->pdata[index]);
    v->values->pdata[index] = value;
  }
}

G_MODULE_EXPORT void fl_value_map_set_string(FlValue* self,
                                             const gchar* key,
                                             FlValue* value) {
  g_return_if_fail(self->type == FL_VALUE_TYPE_MAP);
  g_return_if_fail(key != nullptr);
  g_return_if_fail(value != nullptr);

  fl_value_map_set_take(self, fl_value_string_new(key), fl_value_ref(value));
}

G_MODULE_EXPORT void fl_value_map_set_string_take(FlValue* self,
                                                  const gchar* key,
                                                  FlValue* value) {
  g_return_if_fail(self->type == FL_VALUE_TYPE_MAP);
  g_return_if_fail(key != nullptr);
  g_return_if_fail(value != nullptr);

  fl_value_map_set_take(self, fl_value_string_new(key), value);
}

G_MODULE_EXPORT bool fl_value_get_bool(FlValue* self) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_BOOL, FALSE);
  FlValueBool* v = reinterpret_cast<FlValueBool*>(self);
  return v->value;
}

G_MODULE_EXPORT int64_t fl_value_get_int(FlValue* self) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_INT, 0);
  FlValueInt* v = reinterpret_cast<FlValueInt*>(self);
  return v->value;
}

G_MODULE_EXPORT double fl_value_get_float(FlValue* self) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_FLOAT, 0.0);
  FlValueDouble* v = reinterpret_cast<FlValueDouble*>(self);
  return v->value;
}

G_MODULE_EXPORT const gchar* fl_value_get_string(FlValue* self) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_STRING, nullptr);
  FlValueString* v = reinterpret_cast<FlValueString*>(self);
  return v->value;
}

G_MODULE_EXPORT const uint8_t* fl_value_get_uint8_list(FlValue* self) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_UINT8_LIST, nullptr);
  FlValueUint8List* v = reinterpret_cast<FlValueUint8List*>(self);
  return v->values;
}

G_MODULE_EXPORT const int32_t* fl_value_get_int32_list(FlValue* self) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_INT32_LIST, nullptr);
  FlValueInt32List* v = reinterpret_cast<FlValueInt32List*>(self);
  return v->values;
}

G_MODULE_EXPORT const int64_t* fl_value_get_int64_list(FlValue* self) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_INT64_LIST, nullptr);
  FlValueInt64List* v = reinterpret_cast<FlValueInt64List*>(self);
  return v->values;
}

G_MODULE_EXPORT const double* fl_value_get_float_list(FlValue* self) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_FLOAT_LIST, nullptr);
  FlValueFloatList* v = reinterpret_cast<FlValueFloatList*>(self);
  return v->values;
}

G_MODULE_EXPORT size_t fl_value_get_length(FlValue* self) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_UINT8_LIST ||
                           self->type == FL_VALUE_TYPE_INT32_LIST ||
                           self->type == FL_VALUE_TYPE_INT64_LIST ||
                           self->type == FL_VALUE_TYPE_FLOAT_LIST ||
                           self->type == FL_VALUE_TYPE_LIST ||
                           self->type == FL_VALUE_TYPE_MAP,
                       0);

  switch (self->type) {
    case FL_VALUE_TYPE_UINT8_LIST: {
      FlValueUint8List* v = reinterpret_cast<FlValueUint8List*>(self);
      return v->values_length;
    }
    case FL_VALUE_TYPE_INT32_LIST: {
      FlValueInt32List* v = reinterpret_cast<FlValueInt32List*>(self);
      return v->values_length;
    }
    case FL_VALUE_TYPE_INT64_LIST: {
      FlValueInt64List* v = reinterpret_cast<FlValueInt64List*>(self);
      return v->values_length;
    }
    case FL_VALUE_TYPE_FLOAT_LIST: {
      FlValueFloatList* v = reinterpret_cast<FlValueFloatList*>(self);
      return v->values_length;
    }
    case FL_VALUE_TYPE_LIST: {
      FlValueList* v = reinterpret_cast<FlValueList*>(self);
      return v->values->len;
    }
    case FL_VALUE_TYPE_MAP: {
      FlValueMap* v = reinterpret_cast<FlValueMap*>(self);
      return v->keys->len;
    }
    case FL_VALUE_TYPE_NULL:
    case FL_VALUE_TYPE_BOOL:
    case FL_VALUE_TYPE_INT:
    case FL_VALUE_TYPE_FLOAT:
    case FL_VALUE_TYPE_STRING:
      return 0;
  }

  return 0;
}

G_MODULE_EXPORT FlValue* fl_value_list_get_value(FlValue* self, size_t index) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_LIST, nullptr);

  FlValueList* v = reinterpret_cast<FlValueList*>(self);
  return static_cast<FlValue*>(g_ptr_array_index(v->values, index));
}

G_MODULE_EXPORT FlValue* fl_value_map_get_key(FlValue* self, size_t index) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_MAP, nullptr);

  FlValueMap* v = reinterpret_cast<FlValueMap*>(self);
  return static_cast<FlValue*>(g_ptr_array_index(v->keys, index));
}

G_MODULE_EXPORT FlValue* fl_value_map_get_value(FlValue* self, size_t index) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_MAP, nullptr);

  FlValueMap* v = reinterpret_cast<FlValueMap*>(self);
  return static_cast<FlValue*>(g_ptr_array_index(v->values, index));
}

G_MODULE_EXPORT FlValue* fl_value_map_lookup(FlValue* self, FlValue* key) {
  g_return_val_if_fail(self->type == FL_VALUE_TYPE_MAP, nullptr);

  ssize_t index = fl_value_map_lookup_index(self, key);
  if (index < 0)
    return nullptr;
  return fl_value_map_get_value(self, index);
}

FlValue* fl_value_map_lookup_string(FlValue* self, const gchar* key) {
  g_autoptr(FlValue) string_key = fl_value_string_new(key);
  return fl_value_map_lookup(self, string_key);
}
