// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_ACCESSIBLE_NODE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_ACCESSIBLE_NODE_H_

#include <gtk/gtk.h>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_engine.h"

G_BEGIN_DECLS

#define FL_TYPE_ACCESSIBLE_NODE fl_accessible_node_get_type()
G_DECLARE_FINAL_TYPE(FlAccessibleNode,
                     fl_accessible_node,
                     FL,
                     ACCESSIBLE_NODE,
                     GObject);

/**
 * fl_accessible_node_new:
 * @display: the display this node is on.
 * @engine: the #FlEngine this node came from.
 *
 * Creates a new accessibility object that exposes Flutter accessibility
 * information to GTK.
 *
 * Returns: a new #FlAccessibleNode.
 */
FlAccessibleNode* fl_accessible_node_new(GdkDisplay* display, FlEngine* engine);

void fl_accessible_node_set_parent(FlAccessibleNode* self,
                                   GtkAccessible* parent);

void fl_accessible_node_set_first_child(FlAccessibleNode* self,
                                        FlAccessibleNode* child);

void fl_accessible_node_set_sibling(FlAccessibleNode* self,
                                    FlAccessibleNode* sibling);

/**
 * fl_accessible_node_set_bounds:
 * @node: an #FlAccessibleNode.
 * @x: x co-ordinate of this node relative to its parent.
 * @y: y co-ordinate of this node relative to its parent.
 * @width: width of this node in pixels.
 * @height: height of this node in pixels.
 *
 * Sets the position and size of this node.
 */
void fl_accessible_node_set_bounds(FlAccessibleNode* node,
                                   gint x,
                                   gint y,
                                   gint width,
                                   gint height);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_ACCESSIBLE_NODE_H_
