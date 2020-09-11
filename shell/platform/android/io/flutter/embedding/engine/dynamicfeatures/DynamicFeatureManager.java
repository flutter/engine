// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.dynamicfeatures;

import androidx.annotation.NonNull;

/**
 * 
 */
public interface DynamicFeatureManager {

  // Requests that the module be downloaded.
  public abstract void downloadModule(@NonNull String moduleName, int loadingUnitId);

  // Extracts any assets and resources from the module for use by Flutter.
  public abstract void extractModule(@NonNull String moduleName, int loadingUnitId);

  // Loads the .so file into the Dart VM.
  public abstract void loadDartModules(@NonNull String moduleName, int loadingUnitId);

}
