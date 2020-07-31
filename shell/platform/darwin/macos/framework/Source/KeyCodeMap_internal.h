// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Maps macOS-specific key code values representing [PhysicalKeyboardKey].
 *
 * MacOS doesn't provide a scan code, but a virtual keycode to represent a physical key.
 */
static NSDictionary* keyCodeToPhysicalKey;

/**
 * A map of macOS key codes which have printable representations, but appear
 * on the number pad. Used to provide different key objects for keys like
 * KEY_EQUALS and NUMPAD_EQUALS.
 */
static NSDictionary* keyCodeToNumpad;

/**
 * A map of macOS key codes which are numbered function keys, so that they
 * can be excluded when asking "is the Fn modifier down?".
 */
static NSDictionary* keyCodeToFunctionKey;
