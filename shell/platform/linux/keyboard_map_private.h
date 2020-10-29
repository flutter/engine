// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef KEYBOARD_MAP_H_
#define KEYBOARD_MAP_H_

#include <gdk/gdk.h>

// Initialize a hashtable that maps XKB specific key code values to
// Flutter's physical key code values.
void initialize_xkb_to_physical_key(GHashTable* table);

// Initialize a hashtable that maps GDK keyval values to
// Flutter's logical key code values.
void initialize_gtk_keyval_to_logical_key(GHashTable* table);

#endif  // KEYBOARD_MAP_H_
