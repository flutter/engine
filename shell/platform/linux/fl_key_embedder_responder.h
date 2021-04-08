// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_EMBEDDER_RESPONDER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_EMBEDDER_RESPONDER_H_

#include <gdk/gdk.h>

#include "flutter/shell/platform/linux/fl_key_responder.h"
#include "flutter/shell/platform/linux/fl_keyboard_manager.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_binary_messenger.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_value.h"

constexpr int kMaxConvertedKeyData = 3;

G_BEGIN_DECLS

#define FL_TYPE_KEY_EMBEDDER_RESPONDER fl_key_embedder_responder_get_type ()
G_DECLARE_FINAL_TYPE(FlKeyEmbedderResponder,
                     fl_key_embedder_responder,
                     FL,
                     KEY_EMBEDDER_RESPONDER,
                     GObject);

/**
 * FlKeyEmbedderResponder:
 *
 * #FlKeyEmbedderResponder is a plugin that implements the shell side
 * of SystemEmbedders.keyEvent from the Flutter services library.
 */

/**
 * fl_key_embedder_responder_new:
 * @messenger: an #FlBinaryMessenger.
 * @response_callback: the callback to call when a response is received.  If not
 *                     given (nullptr), then the default response callback is
 *                     used. Typically used for tests to receive event
 *                     information. If specified, unhandled events will not be
 *                     re-dispatched.
 * @text_input_plugin: The #FlTextInputPlugin to send key events to if the
 *                     framework doesn't handle them.
 * @embedder_name: the name of the embedder to send key events to the framework
 *                on. If not given (nullptr), then the standard key event
 *                embedder name is used. Typically used for tests to send on a
 *                test embedder.
 *
 * Creates a new plugin that implements SystemEmbedders.keyEvent from the
 * Flutter services library.
 *
 * Returns: a new #FlKeyEmbedderResponder.
 */
FlKeyEmbedderResponder* fl_key_embedder_responder_new(FlEngine* engine);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_EMBEDDER_RESPONDER_H_
