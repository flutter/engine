// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fl_test_gtk_logs.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

namespace {

bool gtk_initialized = false;
GLogLevelFlags fl_received_log_levels = (GLogLevelFlags)0x0;

GLogWriterOutput log_writer(GLogLevelFlags log_level,
                            const GLogField* fields,
                            gsize n_fields,
                            gpointer user_data) {
  fl_received_log_levels = (GLogLevelFlags)(log_level | fl_received_log_levels);
  return g_log_writer_default(log_level, fields, n_fields, user_data);
}

}  // namespace

void fl_ensure_gtk_init() {
  if (!gtk_initialized) {
    gtk_init(0, nullptr);
    g_log_set_writer_func(log_writer, nullptr, nullptr);
    gtk_initialized = true;
  }
  fl_reset_received_gtk_log_levels();
}

void fl_reset_received_gtk_log_levels() {
  fl_received_log_levels = (GLogLevelFlags)0x0;
}

GLogLevelFlags fl_get_received_gtk_log_levels() {
  return fl_received_log_levels;
}

}  // namespace testing
}  // namespace flutter