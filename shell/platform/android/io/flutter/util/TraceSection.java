// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.util;

import androidx.tracing.Trace;

public final class TraceSection implements AutoCloseable {
  /**
   * Wraps Trace.beginSection to ensure that the line length stays below 127 code units, and
   * automatically closes the section in a try-with-resources block.
   *
   * @param sectionName The string to display as the section name in the trace.
   */
  public TraceSection(@NonNull String sectionName) {
    sectionName = sectionName.length() < 127 ? sectionName : sectionName.substring(0, 127);
    Trace.beginSection(name);
  }

  /**
   * Wraps Trace.endSection.
   *
   * <p>This method exists solely to avoid problematic usage of Trace.beginSection.
   */
  public void close() throws Exception {
    Trace.endSection();
  }
}
