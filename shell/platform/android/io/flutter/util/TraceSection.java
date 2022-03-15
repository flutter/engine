// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.util;

import androidx.annotation.NonNull;
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
    Trace.beginSection(sectionName);
  }

  /**
   * Wraps Trace.endSection.
   *
   * <p>This is intended to be called from a try-with-resources block.
   */
  public void close() throws RuntimeException {
    Trace.endSection();
  }
}
