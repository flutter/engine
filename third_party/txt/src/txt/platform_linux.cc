// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <glib.h>
#include <gio/gio.h>

#include <iostream>
#include <vector>

#include "txt/platform.h"

#ifdef FLUTTER_USE_FONTCONFIG
#include "third_party/skia/include/ports/SkFontMgr_fontconfig.h"
#else
#include "third_party/skia/include/ports/SkFontMgr_directory.h"
#endif

namespace txt {

std::vector<std::string> GetDefaultFontFamilies() {
  return { "Ubuntu", "Cantarell", "DejaVu Sans", "Liberation Sans", "Arial" };
//  std::string result = "DejaVu Sans";
//  g_autoptr(GSettingsSchema) schema = g_settings_schema_source_lookup(g_settings_schema_source_get_default(), "org.gnome.desktop.interface", FALSE);
//  if (schema != NULL) {
//    g_autoptr(GSettings) settings = g_settings_new_full(schema, NULL, NULL);
//    if (g_settings_schema_has_key(schema, "font-name")) {
//      g_autofree gchar *font_name = g_settings_get_string(settings, "font-name");
//      result = std::string(font_name);
//      result = result.substr(0, result.rfind(' '));
//    }
//  }
//  std::cout << "default font: " << result << std::endl;
//  return result;
}

sk_sp<SkFontMgr> GetDefaultFontManager() {
#ifdef FLUTTER_USE_FONTCONFIG
  return SkFontMgr_New_FontConfig(nullptr);
#else
  return SkFontMgr_New_Custom_Directory("/usr/share/fonts/");
#endif
}

}  // namespace txt
