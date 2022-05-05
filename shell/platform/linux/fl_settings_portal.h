// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_SETTINGS_PORTAL_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_SETTINGS_PORTAL_H_

#include "flutter/shell/platform/linux/fl_settings.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlSettingsPortal,
                     fl_settings_portal,
                     FL,
                     SETTINGS_PORTAL,
                     GObject);

/**
 * FlSettingsPortal:
 * #FlSettingsPortal reads settings from the XDG desktop portal.
 */

/**
 * fl_settings_portal_new:
 * @values: (nullable): a #GVariantDict with values for testing.
 *
 * Creates a new settings portal instance.
 *
 * Returns: a new #FlSettingsPortal.
 */
FlSettingsPortal* fl_settings_portal_new(GVariantDict* values);

/**
 * fl_settings_portal_start:
 * @portal: an #FlSettingsPortal.
 * @error: (allow-none): #GError location to store the error occurring, or %NULL
 *
 * Reads the current settings and starts monitoring for changes in the desktop
 * portal settings.
 *
 * Returns: %TRUE on success, or %FALSE if the portal is not available.
 */
gboolean fl_settings_portal_start(FlSettingsPortal* portal, GError** error);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_SETTINGS_PORTAL_H_
