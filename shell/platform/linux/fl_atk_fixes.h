// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_ATK_FIXES_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_ATK_FIXES_H_

#include <atk/atk.h>

G_BEGIN_DECLS

// ATK g_autoptr macros weren't added until 2.37. Add them manually.
// https://gitlab.gnome.org/GNOME/atk/-/issues/10
#if !ATK_CHECK_VERSION(2, 37, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(AtkObject, g_object_unref)
#endif

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_ATK_FIXES_H_
