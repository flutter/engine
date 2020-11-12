// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.dynamicfeatures;

import static junit.framework.TestCase.assertFalse;
import static junit.framework.TestCase.assertTrue;
import static org.mockito.Mockito.mock;

import android.content.Context;
import android.content.res.AssetManager;
import androidx.annotation.NonNull;
import io.flutter.embedding.engine.FlutterJNI;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class PlayStoreDynamicFeatureManagerTest {
  private class TestFlutterJNI extends FlutterJNI {
    public boolean loadDartDeferredLibraryCalled = false;
    public boolean updateAssetManagerCalled = false;
    public boolean dynamicFeatureInstallFailureCalled = false;

    public TestFlutterJNI() {}

    @Override
    public void loadDartDeferredLibrary(int loadingUnitId, @NonNull String[] searchPaths) {
      loadDartDeferredLibraryCalled = true;
    }

    @Override
    public void updateAssetManager(
        @NonNull AssetManager assetManager, @NonNull String assetBundlePath) {
      updateAssetManagerCalled = true;
    }

    @Override
    public void dynamicFeatureInstallFailure(
        int loadingUnitId, @NonNull String error, boolean isTransient) {
      dynamicFeatureInstallFailureCalled = true;
    }
  }

  // Skips the download process to directly call the loadAssets and loadDartLibrary methods.
  private class TestPlayStoreDynamicFeatureManager extends PlayStoreDynamicFeatureManager {
    public TestPlayStoreDynamicFeatureManager(Context context, FlutterJNI jni) {
      super(context, jni);
    }

    @Override
    public void downloadDynamicFeature(int loadingUnitId, String moduleName) {
      loadAssets(loadingUnitId, moduleName);
      loadDartLibrary(loadingUnitId, moduleName);
    }
  }

  TestFlutterJNI jni;
  PlayStoreDynamicFeatureManager playStoreManager;

  @Test
  public void downloadCallsJNIFunctions() {
    jni = new TestFlutterJNI();
    Context context = mock(Context.class);
    playStoreManager = new TestPlayStoreDynamicFeatureManager(context, jni);
    jni.setDynamicFeatureManager(playStoreManager);

    playStoreManager.downloadDynamicFeature(0, "TestModuleName");
    assertTrue(jni.loadDartDeferredLibraryCalled);
    assertTrue(jni.updateAssetManagerCalled);
    assertFalse(jni.dynamicFeatureInstallFailureCalled);
  }
}
