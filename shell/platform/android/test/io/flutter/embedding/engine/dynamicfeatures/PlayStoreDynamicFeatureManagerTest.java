// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.dynamicfeatures;

import static junit.framework.TestCase.assertEquals;
import static junit.framework.TestCase.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetManager;
import androidx.annotation.NonNull;
import io.flutter.embedding.engine.FlutterJNI;
import java.io.File;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class PlayStoreDynamicFeatureManagerTest {
  private class TestFlutterJNI extends FlutterJNI {
    public int loadDartDeferredLibraryCalled = 0;
    public int updateAssetManagerCalled = 0;
    public int dynamicFeatureInstallFailureCalled = 0;
    public String[] searchPaths;
    public int loadingUnitId;

    public TestFlutterJNI() {}

    @Override
    public void loadDartDeferredLibrary(int loadingUnitId, @NonNull String[] searchPaths) {
      loadDartDeferredLibraryCalled++;
      this.searchPaths = searchPaths;
      this.loadingUnitId = loadingUnitId;
    }

    @Override
    public void updateAssetManager(
        @NonNull AssetManager assetManager, @NonNull String assetBundlePath) {
      updateAssetManagerCalled++;
      this.loadingUnitId = loadingUnitId;
    }

    @Override
    public void dynamicFeatureInstallFailure(
        int loadingUnitId, @NonNull String error, boolean isTransient) {
      dynamicFeatureInstallFailureCalled++;
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
  TestPlayStoreDynamicFeatureManager playStoreManager;

  @Test
  public void downloadCallsJNIFunctions() throws NameNotFoundException {
    TestFlutterJNI jni = new TestFlutterJNI();
    Context context = mock(Context.class);
    when(context.createPackageContext(any(), anyInt())).thenReturn(context);
    when(context.getAssets()).thenReturn(null);
    String soTestPath = "test/path/app.so-123.part.so";
    when(context.getFilesDir()).thenReturn(new File(soTestPath));
    TestPlayStoreDynamicFeatureManager playStoreManager = new TestPlayStoreDynamicFeatureManager(context, jni);
    jni.setDynamicFeatureManager(playStoreManager);

    assertEquals(jni.loadingUnitId, 0);

    playStoreManager.downloadDynamicFeature(123, "TestModuleName");
    assertEquals(jni.loadDartDeferredLibraryCalled, 1);
    assertEquals(jni.updateAssetManagerCalled, 1);
    assertEquals(jni.dynamicFeatureInstallFailureCalled, 0);

    assertTrue(jni.searchPaths[0].endsWith(soTestPath));
    assertEquals(jni.searchPaths.length, 1);
    assertEquals(jni.loadingUnitId, 123);
  }

  @Test
  public void searchPathsAddsApks() throws NameNotFoundException {
    TestFlutterJNI jni = new TestFlutterJNI();
    Context context = mock(Context.class);
    when(context.createPackageContext(any(), anyInt())).thenReturn(context);
    when(context.getAssets()).thenReturn(null);
    String apkTestPath = "test/path/TestModuleName_armeabi_v7a.apk";
    when(context.getFilesDir()).thenReturn(new File(apkTestPath));
    TestPlayStoreDynamicFeatureManager playStoreManager = new TestPlayStoreDynamicFeatureManager(context, jni);
    jni.setDynamicFeatureManager(playStoreManager);

    assertEquals(jni.loadingUnitId, 0);

    playStoreManager.downloadDynamicFeature(123, "TestModuleName");
    assertEquals(jni.loadDartDeferredLibraryCalled, 1);
    assertEquals(jni.updateAssetManagerCalled, 1);
    assertEquals(jni.dynamicFeatureInstallFailureCalled, 0);

    assertTrue(jni.searchPaths[0].endsWith(apkTestPath + "!lib/armeabi-v7a/app.so-123.part.so"));
    assertEquals(jni.searchPaths.length, 1);
    assertEquals(jni.loadingUnitId, 123);
  }

  @Test
  public void invalidSearchPathsAreIgnored() throws NameNotFoundException {
    TestFlutterJNI jni = new TestFlutterJNI();
    Context context = mock(Context.class);
    when(context.createPackageContext(any(), anyInt())).thenReturn(context);
    when(context.getAssets()).thenReturn(null);
    String apkTestPath = "test/path/invalidpath.apk";
    when(context.getFilesDir()).thenReturn(new File(apkTestPath));
    TestPlayStoreDynamicFeatureManager playStoreManager = new TestPlayStoreDynamicFeatureManager(context, jni);
    jni.setDynamicFeatureManager(playStoreManager);

    assertEquals(jni.loadingUnitId, 0);

    playStoreManager.downloadDynamicFeature(123, "TestModuleName");
    assertEquals(jni.loadDartDeferredLibraryCalled, 1);
    assertEquals(jni.updateAssetManagerCalled, 1);
    assertEquals(jni.dynamicFeatureInstallFailureCalled, 0);

    assertEquals(jni.searchPaths.length, 0);
    assertEquals(jni.loadingUnitId, 123);
  }
}
