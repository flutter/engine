// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_menu_plugin.h"

#include <gdk/gdk.h>
#include <gio/gmenu.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>

#include "flutter/shell/platform/common/platform_provided_menu.h"
#include "flutter/shell/platform/linux/fl_gtk_keymaps.h"
#include "flutter/shell/platform/linux/key_mapping.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_method_channel.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_method_codec.h"

namespace {
// See menu_channel.dart for documentation.
constexpr char kChannelName[] = "flutter/menu";
constexpr char kBadArgumentsError[] = "Bad Arguments";
constexpr char kNoScreenError[] = "No Screen";
constexpr char kMenuSetMethod[] = "Menu.SetMenu";
constexpr char kMenuItemOpenedMethod[] = "Menu.Opened";
constexpr char kMenuItemClosedMethod[] = "Menu.Closed";
constexpr char kMenuItemSelectedCallbackMethod[] = "Menu.SelectedCallback";
constexpr char kMenuActionPrefix[] = "flutter-menu-";
constexpr char kIdKey[] = "id";
constexpr char kLabelKey[] = "label";
constexpr char kEnabledKey[] = "enabled";
constexpr char kChildrenKey[] = "children";
constexpr char kIsDividerKey[] = "isDivider";
constexpr char kShortcutTrigger[] = "shortcutTrigger";
constexpr char kShortcutModifiers[] = "shortcutModifiers";
constexpr char kPlatformDefaultMenuKey[] = "platformDefaultMenu";
constexpr int kFlutterShortcutModifierMeta = 1 << 0;
constexpr int kFlutterShortcutModifierShift = 1 << 1;
constexpr int kFlutterShortcutModifierAlt = 1 << 2;
constexpr int kFlutterShortcutModifierControl = 1 << 3;

namespace {
void quit_app() {
  exit(0);
}

void about_app() {
  std::cerr << "about: test app" << std::endl;
}
}  // namespace

static const std::map<int, std::function<void()>> kLinuxDefaultMenus = {
    {PlatformProvidedMenu::about, about_app},
    {PlatformProvidedMenu::quit, quit_app},
};

static const std::map<int, std::string> kLinuxDefaultMenuLabels = {
    {PlatformProvidedMenu::about, "About"},
    {PlatformProvidedMenu::quit, "Quit"},
};

std::map<uint64_t, uint64_t> logical_key_map_to_gtk_keyval;
std::map<uint64_t, uint64_t> unicode_to_gtk_keyval;

}  // namespace

struct _FlMenuPlugin {
  GObject parent_instance;

  FlView* view;

  // Connection to Flutter engine.
  FlMethodChannel* channel;

  // Special handle used to indicate a divider.
  GMenuItem* divider_item;

  // Menu being shown to the user.
  GMenu* menu;
};

G_DEFINE_TYPE(FlMenuPlugin, fl_menu_plugin, g_object_get_type())

static GtkApplication* get_gtk_app(FlMenuPlugin* self) {
  GtkApplication* gtk_app = gtk_window_get_application(
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(self->view))));
  if (gtk_app == nullptr) {
    g_warning("Failed to get top level GTK application.");
  }
  return gtk_app;
}

static void add_menu_accelerator(FlMenuPlugin* self,
                                 GMenuItem* menu_item,
                                 FlValue* value,
                                 const gchar* action_name) {
  FlValue* shortcut_trigger_value =
      fl_value_lookup_string(value, kShortcutTrigger);
  FlValue* shortcut_modifiers_value =
      fl_value_lookup_string(value, kShortcutModifiers);
  if (shortcut_trigger_value == nullptr ||
      shortcut_modifiers_value == nullptr) {
    return;
  }
  if (fl_value_get_type(shortcut_trigger_value) != FL_VALUE_TYPE_INT) {
    g_warning("Invalid type for trigger: %d",
              fl_value_get_type(shortcut_trigger_value));
    return;
  }
  if (fl_value_get_type(shortcut_modifiers_value) != FL_VALUE_TYPE_INT) {
    g_warning("Invalid type for modifier: %d",
              fl_value_get_type(shortcut_modifiers_value));
    return;
  }

  gint gtk_keyval = -1;

  // Attempt to look up a non-printable key
  int64_t key_id = fl_value_get_int(shortcut_trigger_value);
  gtk_keyval = logical_key_map_to_gtk_keyval.find(key_id) !=
                       logical_key_map_to_gtk_keyval.end()
                   ? logical_key_map_to_gtk_keyval[key_id]
                   : -1;

  // Attempt to look up a Unicode key
  if (gtk_keyval == -1) {
    constexpr int64_t valueMask = 0x000ffffffff;
    int64_t unicode_value = key_id & valueMask;
    gtk_keyval =
        unicode_to_gtk_keyval.find(unicode_value) != unicode_to_gtk_keyval.end()
            ? unicode_to_gtk_keyval[unicode_value]
            : -1;
  }

  // Unable to find a matching keycode.
  if (gtk_keyval == -1) {
    g_warning("Unable to find a keyval for logical key %ld", key_id);
    return;
  }

  gint modifiers = 0;
  int64_t modifier_value = fl_value_get_int(shortcut_modifiers_value);
  if (modifier_value & kFlutterShortcutModifierShift) {
    modifiers |= GdkModifierType::GDK_SHIFT_MASK;
  }
  if (modifier_value & kFlutterShortcutModifierControl) {
    modifiers |= GdkModifierType::GDK_CONTROL_MASK;
  }
  if (modifier_value & kFlutterShortcutModifierAlt) {
    modifiers |= GdkModifierType::GDK_MOD1_MASK;
  }
  if (modifier_value & kFlutterShortcutModifierMeta) {
    modifiers |= GdkModifierType::GDK_META_MASK;
  }

  const char* accelerator_name =
      gtk_accelerator_name(gtk_keyval, static_cast<GdkModifierType>(modifiers));
  g_menu_item_set_attribute(G_MENU_ITEM(menu_item), "accel", "s",
                            accelerator_name, nullptr);
  const char* accelerators[] = {
      accelerator_name,
      NULL,
  };
  GtkApplication* gtk_app = get_gtk_app(self);
  gtk_application_set_accels_for_action(gtk_app, action_name, accelerators);
}

static GMenu* value_to_menu(FlMenuPlugin* self, FlValue* value, GError** error);

// Called when a menu item is selected.
static void menu_select_cb(GSimpleAction* action,
                           GVariant* parameter,
                           gpointer user_data) {
  FlMenuPlugin* self = FL_MENU_PLUGIN(user_data);

  // Parse the action ID out of the action name, since accelerators
  // can't have targets to pass as the parameter.
  std::string action_name = g_action_get_name(G_ACTION(action));
  assert(action_name.substr(0, strlen(kMenuActionPrefix)) ==
             kMenuActionPrefix &&
         "Menu action has the wrong prefix.");
  int last_dash = action_name.find_last_of('-');
  if (last_dash < 0) {
    g_warning("Unable to determine action ID for action %s",
              action_name.c_str());
    return;
  }
  action_name = action_name.substr(last_dash + 1);
  // It's potentially a 64-bit number, so parse accordingly.
  std::istringstream atoi_convert_stream(action_name);
  int64_t id = 0;
  atoi_convert_stream >> id;
  g_autoptr(FlValue) result = fl_value_new_int(id);
  fl_method_channel_invoke_method(self->channel,
                                  kMenuItemSelectedCallbackMethod, result,
                                  nullptr, nullptr, nullptr);
}

// Called when a submenu is opened or closed.
static void submenu_click_cb(GSimpleAction* action,
                             GVariant* parameter,
                             gpointer user_data) {
  FlMenuPlugin* self = FL_MENU_PLUGIN(user_data);

  // Parse the action ID out of the action name, since accelerators
  // can't have targets to pass as the parameter.
  std::string action_name = g_action_get_name(G_ACTION(action));
  assert(action_name.substr(0, strlen(kMenuActionPrefix)) ==
             kMenuActionPrefix &&
         "Menu action has the wrong prefix.");
  int last_dash = action_name.find_last_of('-');
  if (last_dash < 0) {
    g_warning("Unable to determine action ID for action %s",
              action_name.c_str());
    return;
  }
  action_name = action_name.substr(last_dash + 1);
  // It's potentially a 64-bit number, so parse accordingly.
  std::istringstream atoi_convert_stream(action_name);
  int64_t id = 0;
  atoi_convert_stream >> id;
  g_autoptr(FlValue) result = fl_value_new_int(id);
  bool is_open = g_variant_get_boolean(parameter);
  fl_method_channel_invoke_method(
      self->channel, is_open ? kMenuItemOpenedMethod : kMenuItemClosedMethod,
      result, nullptr, nullptr, nullptr);
}

// Called when a default menu item is selected.
static void default_menu_select_cb(GSimpleAction* action,
                                   GVariant* parameter,
                                   gpointer user_data) {
  // Parse the default menu ID out of the action name, since accelerators
  // can't have targets to pass as the parameter.
  std::string action_name = g_action_get_name(G_ACTION(action));
  assert(action_name.substr(0, strlen(kMenuActionPrefix)) ==
             kMenuActionPrefix &&
         "Menu action has the wrong prefix.");
  int last_dash = action_name.find_last_of('-');
  if (last_dash < 0) {
    g_warning("Unable to determine action ID for action %s",
              action_name.c_str());
    return;
  }
  action_name = action_name.substr(last_dash + 1);
  // It's potentially a 64-bit number, so parse accordingly.
  std::istringstream atoi_convert_stream(action_name);
  int64_t id = 0;
  atoi_convert_stream >> id;
  auto action_callback = kLinuxDefaultMenus.find(id);
  if (action_callback != kLinuxDefaultMenus.end()) {
    action_callback->second();
  }
}

static void add_default_menu(FlMenuPlugin* self,
                             GMenuItem* item,
                             FlValue* value,
                             bool is_enabled) {
  FlValue* default_menu_value =
      fl_value_lookup_string(value, kPlatformDefaultMenuKey);
  g_autoptr(GString) default_activate_action_name =
      g_string_new(kMenuActionPrefix);
  int64_t default_menu = fl_value_get_int(default_menu_value);
  auto label_iter = kLinuxDefaultMenuLabels.find(default_menu);
  std::string label = label_iter == kLinuxDefaultMenuLabels.end()
                          ? "<unknown>"
                          : label_iter->second;
  g_menu_item_set_label(item, label.c_str());
  g_string_append_printf(default_activate_action_name, "default-%ld",
                         default_menu);
  g_autoptr(GSimpleAction) action = g_simple_action_new(
      default_activate_action_name->str, G_VARIANT_TYPE_INT64);
  g_simple_action_set_enabled(action, is_enabled);
  g_signal_connect_object(action, "activate",
                          G_CALLBACK(default_menu_select_cb), self,
                          G_CONNECT_SWAPPED);
  g_autoptr(GString) default_menu_id = g_string_new("");
  g_string_append_printf(default_menu_id, "%ld", default_menu);
  const GActionEntry entries[] = {
      {
          .name = default_activate_action_name->str,
          .activate = default_menu_select_cb,
          .parameter_type = nullptr,
      },
  };
  g_action_map_add_action_entries(G_ACTION_MAP(g_application_get_default()),
                                  entries, G_N_ELEMENTS(entries), self);
  g_string_prepend(default_activate_action_name, "app.");
  g_menu_item_set_detailed_action(item, default_activate_action_name->str);
  add_menu_accelerator(self, item, value, default_activate_action_name->str);
}

static void add_menu_item(FlMenuPlugin* self,
                          GMenuItem* item,
                          FlValue* value,
                          bool is_enabled,
                          GError** error) {
  FlValue* label_value = fl_value_lookup_string(value, kLabelKey);
  if (label_value != nullptr &&
      fl_value_get_type(label_value) == FL_VALUE_TYPE_STRING) {
    g_menu_item_set_label(item, fl_value_get_string(label_value));
  }
  g_autoptr(GString) activate_action_name = g_string_new(kMenuActionPrefix);
  FlValue* id_value = fl_value_lookup_string(value, kIdKey);
  if (id_value == nullptr || fl_value_get_type(id_value) != FL_VALUE_TYPE_INT) {
    g_set_error(error, 0, 0, "Menu item missing a menu ID");
  }
  int64_t id = fl_value_get_int(id_value);
  g_string_append_printf(activate_action_name, "activate-%ld", id);
  g_autoptr(GSimpleAction) action =
      g_simple_action_new(activate_action_name->str, G_VARIANT_TYPE_INT64);
  g_simple_action_set_enabled(action, is_enabled);
  g_signal_connect_object(action, "activate", G_CALLBACK(menu_select_cb), self,
                          G_CONNECT_SWAPPED);
  g_autoptr(GString) menu_id = g_string_new("");
  g_string_append_printf(menu_id, "%ld", id);
  const GActionEntry entries[] = {
      {
          .name = activate_action_name->str,
          .activate = menu_select_cb,
          .parameter_type = nullptr,
      },
  };
  g_action_map_add_action_entries(G_ACTION_MAP(g_application_get_default()),
                                  entries, G_N_ELEMENTS(entries), self);
  g_string_prepend(activate_action_name, "app.");
  if (is_enabled) {
    g_menu_item_set_detailed_action(item, activate_action_name->str);
  }
  add_menu_accelerator(self, item, value, activate_action_name->str);
}

// Convert a value received from Flutter to a GtkMenuItem.
static GMenuItem* value_to_menu_item(FlMenuPlugin* self,
                                     FlValue* value,
                                     GError** error) {
  if (fl_value_get_type(value) != FL_VALUE_TYPE_MAP) {
    g_set_error(error, 0, 0, "Menu item map missing or malformed");
    return nullptr;
  }
  FlValue* is_divider_value = fl_value_lookup_string(value, kIsDividerKey);
  if (is_divider_value != nullptr &&
      fl_value_get_type(is_divider_value) == FL_VALUE_TYPE_BOOL &&
      fl_value_get_bool(is_divider_value)) {
    return G_MENU_ITEM(g_object_ref(self->divider_item));
  }

  g_autoptr(GMenuItem) item = g_menu_item_new("<unknown>", "randomAction");

  FlValue* enabled_value = fl_value_lookup_string(value, kEnabledKey);
  bool is_enabled = enabled_value != nullptr &&
                    fl_value_get_type(enabled_value) == FL_VALUE_TYPE_BOOL &&
                    fl_value_get_bool(enabled_value);

  FlValue* children = fl_value_lookup_string(value, kChildrenKey);
  bool has_children = children != nullptr && fl_value_get_length(children) > 0;

  FlValue* default_menu_value =
      fl_value_lookup_string(value, kPlatformDefaultMenuKey);
  if (default_menu_value != nullptr) {
    add_default_menu(self, item, value, is_enabled);
  } else {
    add_menu_item(self, item, value, is_enabled, error);
  }
  if (has_children) {
    g_autoptr(GMenu) sub_menu = value_to_menu(self, children, error);
    if (sub_menu == nullptr) {
      return nullptr;
    }

    // Add an action so we know when the submenu opens and closes.
    // The open/close state is sent in the boolean GVariant parameter.
    FlValue* id_value = fl_value_lookup_string(value, kIdKey);
    if (id_value == nullptr ||
        fl_value_get_type(id_value) != FL_VALUE_TYPE_INT) {
      g_set_error(error, 0, 0, "Menu item missing a menu ID");
    }
    g_autoptr(GString) submenu_action_name = g_string_new(kMenuActionPrefix);
    g_string_append_printf(submenu_action_name, "submenu-%ld",
                           fl_value_get_int(id_value));
    g_autoptr(GString) app_submenu_action_name = g_string_new("app.");
    g_string_append(app_submenu_action_name, submenu_action_name->str);
    g_menu_item_set_attribute(item, "submenu-action", "&s",
                              app_submenu_action_name->str, NULL);
    const GActionEntry entries[] = {
        {
            .name = submenu_action_name->str,
            .parameter_type = "b",  // G_VARIANT_TYPE_BOOLEAN
            .state = "false",
            .change_state = submenu_click_cb,
        },
    };
    g_action_map_add_action_entries(G_ACTION_MAP(g_application_get_default()),
                                    entries, G_N_ELEMENTS(entries), self);

    g_menu_item_set_submenu(item, G_MENU_MODEL(sub_menu));
  }

  return G_MENU_ITEM(g_object_ref(item));
}

// Convert a value received from Flutter to a GtkMenuItem.
static GMenu* value_to_menu(FlMenuPlugin* self,
                            FlValue* value,
                            GError** error) {
  if (fl_value_get_type(value) != FL_VALUE_TYPE_LIST) {
    g_set_error(error, 0, 0, "Menu list missing or malformed");
    return nullptr;
  }

  g_autoptr(GMenu) menu = g_menu_new();
  g_autoptr(GMenu) section = nullptr;
  for (size_t i = 0; i < fl_value_get_length(value); i++) {
    g_autoptr(GMenuItem) item =
        value_to_menu_item(self, fl_value_get_list_value(value, i), error);
    if (item == nullptr) {
      return nullptr;
    }

    if (item == self->divider_item) {
      if (section != nullptr) {
        g_menu_append_section(menu, nullptr, G_MENU_MODEL(section));
      }
      g_clear_object(&section);
    } else {
      if (section == nullptr) {
        section = g_menu_new();
      }
      g_menu_append_item(section, item);
    }
  }
  if (section != nullptr) {
    g_menu_append_section(menu, nullptr, G_MENU_MODEL(section));
  }

  return G_MENU(g_object_ref(menu));
}

static void dump_accels(GtkApplication* app) {
  char** actions;
  int i;

  actions = gtk_application_list_action_descriptions(app);
  for (i = 0; actions[i]; i++) {
    char** accels;
    char* str;

    accels = gtk_application_get_accels_for_action(app, actions[i]);

    str = g_strjoinv(",", accels);
    g_print("%s -> %s\n", actions[i], str);
    g_strfreev(accels);
    g_free(str);
  }
  g_strfreev(actions);
}

// Clears all the accelerators for all actions.
static void clear_actions(GtkApplication* app) {
  char** actions;
  int i;

  GActionMap* map = G_ACTION_MAP(app);
  const char* empty_accelerators[] = {NULL};
  actions = gtk_application_list_action_descriptions(app);
  for (i = 0; actions[i]; i++) {
    gtk_application_set_accels_for_action(app, actions[i], empty_accelerators);
    g_action_map_remove_action(map, actions[i]);
  }
  g_strfreev(actions);
}

// Sets the menu.
static FlMethodResponse* menu_set(FlMenuPlugin* self, FlValue* args) {
  g_autoptr(GError) error = nullptr;
  GtkApplication* app = get_gtk_app(self);
  clear_actions(app);
  g_autoptr(GMenu) menu = value_to_menu(self, args, &error);
  if (menu == nullptr) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, error ? error->message : nullptr, nullptr));
  }

  if (self->view == nullptr) {
    return FL_METHOD_RESPONSE(
        fl_method_error_response_new(kNoScreenError, nullptr, nullptr));
  }

  dump_accels(app);

  // Replace existing menu with this one
  g_menu_remove_all(self->menu);
  g_menu_append_section(self->menu, nullptr, G_MENU_MODEL(menu));

  return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

// Called when a method call is received from Flutter.
static void method_call_cb(FlMethodChannel* channel,
                           FlMethodCall* method_call,
                           gpointer user_data) {
  FlMenuPlugin* self = FL_MENU_PLUGIN(user_data);

  const gchar* method = fl_method_call_get_name(method_call);
  FlValue* args = fl_method_call_get_args(method_call);

  g_autoptr(FlMethodResponse) response = nullptr;
  if (strcmp(method, kMenuSetMethod) == 0) {
    response = menu_set(self, args);
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  g_autoptr(GError) error = nullptr;
  if (!fl_method_call_respond(method_call, response, &error)) {
    g_warning("Failed to send method call response: %s", error->message);
  }
}

static void fl_menu_plugin_dispose(GObject* object) {
  FlMenuPlugin* self = FL_MENU_PLUGIN(object);

  g_clear_object(&self->view);
  g_clear_object(&self->channel);
  g_clear_object(&self->menu);
  g_clear_object(&self->divider_item);

  G_OBJECT_CLASS(fl_menu_plugin_parent_class)->dispose(object);
}

static void fl_menu_plugin_class_init(FlMenuPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_menu_plugin_dispose;
}

static void fl_menu_plugin_init(FlMenuPlugin* self) {
  self->divider_item = g_menu_item_new(nullptr, nullptr);
}

FlMenuPlugin* fl_menu_plugin_new(FlBinaryMessenger* messenger, FlView* view) {
  FlMenuPlugin* self =
      FL_MENU_PLUGIN(g_object_new(fl_menu_plugin_get_type(), nullptr));

  self->view = view;
  if (view != nullptr) {
    g_object_add_weak_pointer(G_OBJECT(view),
                              reinterpret_cast<gpointer*>(&(self->view)));
  }

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  self->channel =
      fl_method_channel_new(messenger, kChannelName, FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(self->channel, method_call_cb, self,
                                            nullptr);

  fl_method_channel_set_method_call_handler(self->channel, method_call_cb,
                                            g_object_ref(self), g_object_unref);

  // Add a GAction for the menu bar to trigger.
  GApplication* app = nullptr;
  if (self->view != nullptr) {
    app = g_application_get_default();
  }
  if (app != nullptr) {
    // Set an empty menu bar now, as GTK doesn't detect it being changed later
    // on. https://gitlab.gnome.org/GNOME/gtk/-/issues/2834
    self->menu = g_menu_new();
    gtk_application_set_menubar(GTK_APPLICATION(app), G_MENU_MODEL(self->menu));
    g_object_notify(G_OBJECT(gtk_settings_get_default()),
                    "gtk-shell-shows-menubar");
  } else {
    g_warning("Unable to get default GTK application.");
  }

  // Set up the reverse lookup map for finding keyvals from logical key ids. The
  // map only contains non-printable keys (Unicode capped keys are handled
  // separately).
  logical_key_map_to_gtk_keyval.clear();
  for (const auto& [key, value] : gtk_keyval_to_logical_key_map) {
    logical_key_map_to_gtk_keyval[value] = key;
  }

  // TODO(gspencergoog): register to listen for changes in the keymap so that
  // this mapping can be updated.
  unicode_to_gtk_keyval.clear();
  for (const auto& keyval : gdk_keyvals) {
    guint32 unicode = gdk_keyval_to_unicode(keyval);
    if (unicode != 0) {
      unicode_to_gtk_keyval[unicode] = keyval;
    }
  }

  return self;
}
