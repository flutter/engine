// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.dynamicfeatures;

/**
 * Basic interface that may be implemented to provide custom handling of dynamic features.
 *
 * The Flutter default implementation is PlayStoreDynamicFeatureManager.
 *
 * The methods here may be called independently or in a sequence one after the other to perform
 * a full install cycle of download, extract, and load dart libs.
 */
public interface DynamicFeatureManager {
  // Request that the feature module be downloaded and installed.
  public abstract void downloadFeature(String moduleName, int loadingUnitId);

  // Extract and load any assets and resources from the module for use by Flutter.
  // Depending on the structure of the feature module, there may or may not be assets
  // to extract.
  public abstract void extractFeature(String moduleName, int loadingUnitId);

  // Load the .so shared library file into the Dart VM. Asset only features may skip this
  // step.
  public abstract void loadDartLibrary(String moduleName, int loadingUnitId);

  // Uninstall the specified feature module.
  public abstract void uninstallFeature(String moduleName, int loadingUnitId);
}
