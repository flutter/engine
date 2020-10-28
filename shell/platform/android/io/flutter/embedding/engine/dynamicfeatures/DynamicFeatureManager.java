// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.dynamicfeatures;

// Flutter dynamic feature support is still in early developer preview, and should
// not be used in production apps yet.

/**
 * Basic interface that may be implemented to provide custom handling of dynamic features.
 *
 * The Flutter default implementation is PlayStoreDynamicFeatureManager.
 *
 * The methods here may be called independently or in a sequence one after the other to perform
 * a full install cycle of download, extract, and load dart libs.
 *
 * A dynamic feature module is uniquely identified by a module name. Each feature module may
 * also contain one or more loading units, uniquely identified by the loading unit ID.
 */
public interface DynamicFeatureManager {
  /**
   * Request that the feature module be downloaded and installed.
   * 
   * This method is called when loadLibrary() is called on a dart library.
   * Upon completion of download, loadAssets and loadDartLibrary should
   * be called to complete the dynamic feature load process.
   */
  public abstract void downloadFeature(String moduleName, int loadingUnitId);

  /**
   * Extract and load any assets and resources from the module for use by Flutter.
   *
   * Assets shoud be loaded before the dart library is loaded, as successful loading
   * of the dart loading unit indicates the dynamic feature is fully loaded.
   *
   * Depending on the structure of the feature module, there may or may not be assets
   * to extract.
   */
  public abstract void loadAssets(String moduleName, int loadingUnitId);

  /**
   * Load the .so shared library file into the Dart VM.
   * 
   * Upon successful load of the dart library, the feature corresponding to the
   * loadingUnitId is considered finished loading, and the dart future completes.
   * Developers are expected to begin using symbols and assets from the feature
   * module after completion.
   */
  public abstract void loadDartLibrary(String moduleName, int loadingUnitId);

  /**
   * Uninstall the specified feature module.
   */
  public abstract void uninstallFeature(String moduleName, int loadingUnitId);
}
