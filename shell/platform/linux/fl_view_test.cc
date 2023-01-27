// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Included first as it collides with the X11 headers.
#include "gtest/gtest.h"

#include "flutter/shell/platform/linux/fl_view_private.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_engine.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_view.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"

struct FlViewTest : ::testing::Test {
  static void SetUpTestSuite() { gtk_init(nullptr, nullptr); }
};

TEST_F(FlViewTest, SizeAllocate) {
  g_autoptr(FlDartProject) project = fl_dart_project_new();
  FlView* view = fl_view_new(project);
  gtk_widget_show(GTK_WIDGET(view));

  GtkAllocation allocation = {0, 0, 320, 240};
  gtk_widget_size_allocate(GTK_WIDGET(view), &allocation);

  fl_view_begin_frame(view);

  GtkWidget* child = gtk_label_new(nullptr);
  fl_view_add_widget(view, child, nullptr);

  fl_view_end_frame(view);

  GtkAllocation child_allocation;
  gtk_widget_get_allocation(child, &child_allocation);
  EXPECT_EQ(child_allocation.x, 0);
  EXPECT_EQ(child_allocation.y, 0);
  EXPECT_EQ(child_allocation.width, 320);
  EXPECT_EQ(child_allocation.height, 240);

  allocation = {0, 0, 321, 239};
  gtk_widget_size_allocate(GTK_WIDGET(view), &allocation);

  gtk_widget_get_allocation(child, &child_allocation);
  EXPECT_EQ(child_allocation.x, 0);
  EXPECT_EQ(child_allocation.y, 0);
  EXPECT_EQ(child_allocation.width, 321);
  EXPECT_EQ(child_allocation.height, 239);

  gtk_widget_destroy(GTK_WIDGET(view));
}
