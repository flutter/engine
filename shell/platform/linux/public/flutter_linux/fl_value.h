// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_VALUE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_VALUE_H_

#include <glib.h>
#include <stdbool.h>
#include <stdint.h>

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

G_BEGIN_DECLS

/**
 * FlValue:
 *
 * #FlValue is an object that contains data using Flutter platform channels.
 * Values are encoded and decoded into a binary form using #FlCodec.
 * Values are used in method calls with a #FlMethodChannel.
 */
typedef struct _FlValue FlValue;

/**
 * FlValueType:
 * @FL_VALUE_TYPE_NULL: The null value.
 * @FL_VALUE_TYPE_BOOL: A boolean.
 * @FL_VALUE_TYPE_INT: An integer.
 * @FL_VALUE_TYPE_FLOAT: A floating point number.
 * @FL_VALUE_TYPE_STRING: UTF-8 text.
 * @FL_VALUE_TYPE_UINT8_LIST: An ordered list of unsigned 8 bit integers.
 * @FL_VALUE_TYPE_INT32_LIST: An ordered list of 32 bit integers.
 * @FL_VALUE_TYPE_INT64_LIST: An ordered list of 64 bit integers.
 * @FL_VALUE_TYPE_FLOAT_LIST: An ordered list of floating point numbers.
 * @FL_VALUE_TYPE_LIST: An ordered list of #FlValue objects.
 * @FL_VALUE_TYPE_MAP: A map of #FlValue objects keyed by #FlValue object.
 *
 * Types of #FlValue.
 */
typedef enum {
  FL_VALUE_TYPE_NULL,
  FL_VALUE_TYPE_BOOL,
  FL_VALUE_TYPE_INT,
  FL_VALUE_TYPE_FLOAT,
  FL_VALUE_TYPE_STRING,
  FL_VALUE_TYPE_UINT8_LIST,
  FL_VALUE_TYPE_INT32_LIST,
  FL_VALUE_TYPE_INT64_LIST,
  FL_VALUE_TYPE_FLOAT_LIST,
  FL_VALUE_TYPE_LIST,
  FL_VALUE_TYPE_MAP,
} FlValueType;

/**
 * fl_value_null_new:
 *
 * Create a new #FlValue that contains a null value.
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_null_new();

/**
 * fl_value_bool_new:
 * @value: the value.
 *
 * Create a new #FlValue that contains a boolean value.
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_bool_new(bool value);

/**
 * fl_value_int_new:
 * @value: the value.
 *
 * Create a new #FlValue that contains an integer number.
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_int_new(int64_t value);

/**
 * fl_value_float_new:
 * @value: the value.
 *
 * Create a new #FlValue that contains a floating point number.
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_float_new(double value);

/**
 * fl_value_string_new:
 * @value: the value.
 *
 * Create a new #FlValue that contains UTF-8 text.
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_string_new(const gchar* value);

/**
 * fl_value_string_new:
 * @value: the value.
 * @value_length: the number of bytes to use from @value.
 *
 * Create a new #FlValue that contains UTF-8 text.
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_string_new_sized(const gchar* value, size_t value_length);

/**
 * fl_value_uint8_list_new:
 * @data: Data to copy.
 * @data_length: number of elements in @data.
 *
 * Create a new ordered list containing 8 bit unsigned values.
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_uint8_list_new(const uint8_t* data, size_t data_length);

/**
 * fl_value_uint8_list_new:
 * @data: Data to copy
 *
 * Create a new ordered list containing 8 bit unsigned integers.
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_uint8_list_new_from_bytes(GBytes* data);

/**
 * fl_value_int32_list_new:
 *
 * Create a new ordered list containing 32 bit integers.
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_int32_list_new(const int32_t* data, size_t data_length);

/**
 * fl_value_int64_list_new:
 *
 * Create a new ordered list containing 64 bit integers.
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_int64_list_new(const int64_t* data, size_t data_length);

/**
 * fl_value_float_list_new:
 *
 * Create a new ordered list containing floating point numbers.
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_float_list_new(const double* data, size_t data_length);

/**
 * fl_value_list_new:
 *
 * Create a new ordered list. Children can be added to the list using
 * fl_value_list_add(). The children are accessed using fl_value_get_length()
 * and fl_value_list_get_value().
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_list_new();

/**
 * fl_value_map_new:
 *
 * Create a new map. Children can be added to the map using fl_value_map_set().
 * The children are accessed using fl_value_get_length(),
 * fl_value_map_get_key(), fl_value_map_get_value() and fl_value_map_lookup().
 *
 * Returns: a new #FlValue.
 */
FlValue* fl_value_map_new();

/**
 * fl_value_ref:
 * @value: an #FlValue
 *
 * Increase the reference count of an #FlValue.
 *
 * Returns: the value that was referenced.
 */
FlValue* fl_value_ref(FlValue* value);

/**
 * fl_value_ref:
 * @value: an #FlValue
 *
 * Derease the reference count of an #FlValue. When the refernece count hits
 * zero the value is destroyed and no longer valid.
 */
void fl_value_unref(FlValue* value);

/**
 * fl_value_get_type:
 * @value: an #FlValue
 *
 * Get the type of this value.
 *
 * Returns: an #FlValueType.
 */
FlValueType fl_value_get_type(FlValue* value);

/**
 * fl_value_equal:
 * @a: an #FlValue
 * @b: an #FlValue
 *
 * Compare two #FlValue to see if they are equivalent. Two values are considered
 * equivalent if they are of the same type and their data is the same including
 * any child values.
 *
 * Returns: %TRUE if both values are equivalent.
 */
bool fl_value_equal(FlValue* a, FlValue* b);

/**
 * fl_value_list_add:
 * @value: an #FlValue of type #FL_VALUE_TYPE_LIST
 * @child: an #FlValue
 *
 * Add a value to a list. Calling this with an #FlValue that is not of type
 * #FL_VALUE_TYPE_LIST is a programming error.
 */
void fl_value_list_add(FlValue* value, FlValue* child);

/**
 * fl_value_list_add:
 * @value: an #FlValue of type #FL_VALUE_TYPE_LIST
 * @child: (transfer full): an #FlValue
 *
 * Add a value to a list taking ownership of that value. Calling this with an
 * #FlValue that is not of type #FL_VALUE_TYPE_LIST is a programming error.
 */
void fl_value_list_add_take(FlValue* value, FlValue* child);

/**
 * fl_value_map_set:
 * @value: an #FlValue of type #FL_VALUE_TYPE_MAP
 * @key: an #FlValue
 * @child_value: an #FlValue
 *
 * Set a value in the map. If an existing value was in the map with the same key
 * it is replaced. Calling this with an #FlValue that is not of type
 * #FL_VALUE_TYPE_MAP is a programming error.
 */
void fl_value_map_set(FlValue* value, FlValue* key, FlValue* child_value);

/**
 * fl_value_map_set_take:
 * @value: an #FlValue of type #FL_VALUE_TYPE_MAP
 * @key: (transfer full): an #FlValue
 * @child_value: (transfer full): an #FlValue
 *
 * Set a value in the map, taking ownership of the keys and values. If an
 * existing value was in the map with the same key it is replaced. Calling this
 * with an #FlValue that is not of type #FL_VALUE_TYPE_MAP is a programming
 * error.
 */
void fl_value_map_set_take(FlValue* value, FlValue* key, FlValue* child_value);

/**
 * fl_value_map_set_string:
 * @value: an #FlValue of type #FL_VALUE_TYPE_MAP
 * @key: a UTF-8 text key
 * @child_value: an #FlValue
 *
 * Set a value in the map with a text key. If an existing value was in the map
 * with the same key it is replaced. Calling this with an #FlValue that is not
 * of type #FL_VALUE_TYPE_MAP is a programming error.
 */
void fl_value_map_set_string(FlValue* value,
                             const gchar* key,
                             FlValue* child_value);

/**
 * fl_value_map_set_string_take:
 * @value: an #FlValue of type #FL_VALUE_TYPE_MAP
 * @key: a UTF-8 text key
 * @child_value: (transfer full): an #FlValue
 *
 * Set a value in the map with a text key, taking ownership of the value. If an
 * existing value was in the map with the same key it is replaced. Calling this
 * with an #FlValue that is not of type #FL_VALUE_TYPE_MAP is a programming
 * error.
 */
void fl_value_map_set_string_take(FlValue* value,
                                  const gchar* key,
                                  FlValue* child_value);

/**
 * fl_value_get_bool:
 * @value: an #FlValue of type #FL_VALUE_TYPE_BOOL
 *
 * Get the boolean value of this value. Calling this with an #FlValue that is
 * not of type #FL_VALUE_TYPE_BOOL is a programming error.
 *
 * Returns: a boolean value.
 */
bool fl_value_get_bool(FlValue* value);

/**
 * fl_value_get_int:
 * @value: an #FlValue of type #FL_VALUE_TYPE_INT
 *
 * Get the integer number of this value. Calling this with an #FlValue that is
 * not of type #FL_VALUE_TYPE_INT is a programming error.
 *
 * Returns: an integer number.
 */
int64_t fl_value_get_int(FlValue* value);

/**
 * fl_value_get_double:
 * @value: an #FlValue of type #FL_VALUE_TYPE_FLOAT
 *
 * Get the floating point number of this value. Calling this with an #FlValue
 * that is not of type #FL_VALUE_TYPE_FLOAT is a programming error.
 *
 * Returns: a UTF-8 encoded string.
 */
double fl_value_get_float(FlValue* value);

/**
 * fl_value_get_string:
 * @value: an #FlValue of type #FL_VALUE_TYPE_STRING
 *
 * Get the UTF-8 text contained in this value.
 *
 * Returns: a UTF-8 encoded string.
 */
const gchar* fl_value_get_string(FlValue* value);

/**
 * fl_value_get_length:
 * @value: an #FlValue of type #FL_VALUE_TYPE_UINT8_LIST,
 * #FL_VALUE_TYPE_INT32_LIST, #FL_VALUE_TYPE_INT64_LIST,
 * #FL_VALUE_TYPE_FLOAT_LIST, #FL_VALUE_TYPE_LIST or #FL_VALUE_TYPE_MAP.
 *
 * Gets the number of elements this value contains. This is only valid for list
 * and map types. Calling this with other types is a programming error.
 *
 * Returns: the number of elements inside this value.
 */
size_t fl_value_get_length(FlValue* value);

/**
 * fl_value_get_uint8_list:
 * @value: an #FlValue of type #FL_VALUE_TYPE_UINT8_LIST
 *
 * Get the array of unisigned 8 bit integers this value contains. The data
 * contains fl_list_get_length() elements. Calling this with an #FlValue that is
 * not of type #FL_VALUE_TYPE_UINT8_LIST is a programming error.
 *
 * Returns: an array of unsigned 8 bit integers.
 */
const uint8_t* fl_value_get_uint8_list(FlValue* value);

/**
 * fl_value_get_int32_list:
 * @value: an #FlValue of type #FL_VALUE_TYPE_INT32_LIST
 *
 * Get the array of 32 bit integers this value contains. The data contains
 * fl_list_get_length() elements. Calling this with an #FlValue that is not of
 * type #FL_VALUE_TYPE_INT32_LIST is a programming error.
 *
 * Returns: an array of 32 bit integers.
 */
const int32_t* fl_value_get_int32_list(FlValue* value);

/**
 * fl_value_get_int64_list:
 * @value: an #FlValue of type #FL_VALUE_TYPE_INT64_LIST
 *
 * Get the array of 64 bit integers this value contains. The data contains
 * fl_list_get_length() elements. Calling this with an #FlValue that is not of
 * type #FL_VALUE_TYPE_INT64_LIST is a programming error.
 *
 * Returns: an array of 64 bit integers.
 */
const int64_t* fl_value_get_int64_list(FlValue* value);

/**
 * fl_value_get_float_list:
 * @value: an #FlValue of type #FL_VALUE_TYPE_FLOAT_LIST
 *
 * Get the array of floating point numbers this value contains. The data
 * contains fl_list_get_length() elements. Calling this with an #FlValue that is
 * not of type #FL_VALUE_TYPE_FLOAT_LIST is a programming error.
 *
 * Returns: an array of floating point numbers.
 */
const double* fl_value_get_float_list(FlValue* value);

/**
 * fl_value_list_get_value:
 * @value: an #FlValue of type #FL_VALUE_TYPE_LIST.
 * @index: an index in the list.
 *
 * Get a child element of the list. It is a programming error to request an
 * index that is outside the size of the list as returned from
 * fl_value_get_length().
 *
 * Returns: a #FlValue
 */
FlValue* fl_value_list_get_value(FlValue* value, size_t index);

/**
 * fl_value_map_get_key:
 * @value: an #FlValue of type #FL_VALUE_TYPE_MAP.
 * @index: an index in the map.
 *
 * Get an key from the map. It is a programming error to request an index that
 * is outside the size of the list as returned from fl_value_get_length().
 *
 * Returns: a #FlValue
 */
FlValue* fl_value_map_get_key(FlValue* value, size_t index);

/**
 * fl_value_map_get_key:
 * @value: an #FlValue of type #FL_VALUE_TYPE_MAP.
 * @index: an index in the map.
 *
 * Get a value from the map. It is a programming error to request an index that
 * is outside the size of the list as returned from fl_value_get_length().
 *
 * Returns: a #FlValue
 */
FlValue* fl_value_map_get_value(FlValue* value, size_t index);

/**
 * fl_value_map_lookup:
 * @value: an #FlValue of type FL_VALUE_TYPE_MAP
 * @key: a key value
 *
 * Get the map entry that matches the given key.
 *
 * Returns: (allow-none): the value with this key or %NULL if not one present.
 */
FlValue* fl_value_map_lookup(FlValue* value, FlValue* key);

/**
 * fl_value_map_lookup_string:
 * @value: an #FlValue of type FL_VALUE_TYPE_MAP
 * @key: a key value
 *
 * Get the map entry that matches the given text key.
 *
 * Returns: (allow-none): the value with this key or %NULL if not one present.
 */
FlValue* fl_value_map_lookup_string(FlValue* value, const gchar* key);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(FlValue, fl_value_unref)

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_VALUE_H_
