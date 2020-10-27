// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.dynamicfeatures;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetManager;
import android.os.Build;
import androidx.annotation.NonNull;
import com.google.android.play.core.splitinstall.SplitInstallException;
import com.google.android.play.core.splitinstall.SplitInstallManager;
import com.google.android.play.core.splitinstall.SplitInstallManagerFactory;
import com.google.android.play.core.splitinstall.SplitInstallRequest;
import com.google.android.play.core.splitinstall.SplitInstallSessionState;
import com.google.android.play.core.splitinstall.SplitInstallStateUpdatedListener;
import com.google.android.play.core.splitinstall.model.SplitInstallErrorCode;
import com.google.android.play.core.splitinstall.model.SplitInstallSessionStatus;
import io.flutter.Log;
import io.flutter.embedding.engine.FlutterJNI;
import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.util.Queue;
import java.util.LinkedList;
import java.io.File;

/**
 * Flutter default implementation of DynamicFeatureManager that downloads dynamic features from the
 * Google Play store.
 */
public class PlayStoreDynamicFeatureManager implements DynamicFeatureManager {
  private static final String TAG = "flutter";

  private @NonNull SplitInstallManager splitInstallManager;
  private @NonNull Map<Integer, String> sessionIdToName;
  private @NonNull Map<Integer, Integer> sessionIdToLoadingUnitId;
  private @NonNull FlutterJNI flutterJNI;
  private @NonNull Context context;

  private FeatureInstallStateUpdatedListener listener;

  private class FeatureInstallStateUpdatedListener implements SplitInstallStateUpdatedListener {
    public void onStateUpdate(SplitInstallSessionState state) {
      if (sessionIdToName.containsKey(state.sessionId())) {
        // TODO(garyq): Add capability to access the state from framework.
        switch (state.status()) {
          case SplitInstallSessionStatus.FAILED: {
            Log.d(TAG, "Module \"" + sessionIdToName.get(state.sessionId()) + "\" (sessionId " + state.sessionId() + ") install failed with " + state.errorCode());
            flutterJNI.dynamicFeatureInstallFailure(sessionIdToName.get(state.sessionId()),  sessionIdToLoadingUnitId.get(state.sessionId()), "Module install failed with " + state.errorCode(), true);
            sessionIdToName.remove(state.sessionId());
            sessionIdToLoadingUnitId.remove(state.sessionId());
            break;
          }
          case SplitInstallSessionStatus.INSTALLED: {
            Log.d(TAG, "Module \"" + sessionIdToName.get(state.sessionId()) + "\" (sessionId " + state.sessionId() + ") installed successfully.");
            extractFeature(sessionIdToName.get(state.sessionId()), sessionIdToLoadingUnitId.get(state.sessionId()));
            sessionIdToName.remove(state.sessionId());
            sessionIdToLoadingUnitId.remove(state.sessionId());
            break;
          }
          case SplitInstallSessionStatus.CANCELED: {
            Log.d(TAG, "Module \"" + sessionIdToName.get(state.sessionId()) + "\" (sessionId " + state.sessionId() + ") cancelled");
            sessionIdToName.remove(state.sessionId());
            break;
          }
          case SplitInstallSessionStatus.CANCELING: {
            Log.d(TAG, "Module \"" + sessionIdToName.get(state.sessionId()) + "\" (sessionId " + state.sessionId() + ") canceling");
            sessionIdToName.remove(state.sessionId());
            break;
          }
          case SplitInstallSessionStatus.PENDING: {
            Log.d(TAG, "Module \"" + sessionIdToName.get(state.sessionId()) + "\" (sessionId " + state.sessionId() + ") pending.");
            break;
          }
          case SplitInstallSessionStatus.REQUIRES_USER_CONFIRMATION: {
            Log.d(TAG, "Module \"" + sessionIdToName.get(state.sessionId()) + "\" (sessionId " + state.sessionId() + ") requires user confirmation.");
            break;
          }
          case SplitInstallSessionStatus.DOWNLOADING: {
            Log.d(TAG, "Module \"" + sessionIdToName.get(state.sessionId()) + "\" (sessionId " + state.sessionId() + ") downloading.");
            break;
          }
          case SplitInstallSessionStatus.DOWNLOADED: {
            Log.d(TAG, "Module \"" + sessionIdToName.get(state.sessionId()) + "\" (sessionId " + state.sessionId() + ") downloaded.");
            break;
          }
          case SplitInstallSessionStatus.INSTALLING: {
            Log.d(TAG, "Module \"" + sessionIdToName.get(state.sessionId()) + "\" (sessionId " + state.sessionId() + ") installing.");
            break;
          }
          default: Log.d(TAG, "Status: " + state.status());
        }
      }
    }
  }

  public PlayStoreDynamicFeatureManager(@NonNull Context context, @NonNull FlutterJNI flutterJNI) {
    this.context = context;
    this.flutterJNI = flutterJNI;
    splitInstallManager = SplitInstallManagerFactory.create(context);
    listener = new FeatureInstallStateUpdatedListener();
    splitInstallManager.registerListener(listener);
    sessionIdToName = new HashMap();
    sessionIdToLoadingUnitId = new HashMap();
  }

  public void downloadFeature(String moduleName, int loadingUnitId) {
    if (moduleName == null) {
      Log.e(TAG, "Dynamic feature module name was null.");
      return;
    }

    SplitInstallRequest request =
        SplitInstallRequest
            .newBuilder()
            .addModule(moduleName)
            .build();

    splitInstallManager
        // Submits the request to install the module through the
        // asynchronous startInstall() task. Your app needs to be
        // in the foreground to submit the request.
        .startInstall(request)
        // Called when the install request is sent successfully. This is different than a successful install
        // which is handled in FeatureInstallStateUpdatedListener.
        .addOnSuccessListener(sessionId -> {
          this.sessionIdToName.put(sessionId, moduleName);
          this.sessionIdToLoadingUnitId.put(sessionId, loadingUnitId);
          Log.d(TAG, "Request to install module \"" + moduleName + "\" sent with session id " + sessionId + ".");
        })
        .addOnFailureListener(exception -> {
          switch(((SplitInstallException) exception).getErrorCode()) {
            case SplitInstallErrorCode.NETWORK_ERROR:
              Log.d(TAG, "Install of dynamic feature module \"" + moduleName + "\" failed with a network error");
              flutterJNI.dynamicFeatureInstallFailure(moduleName, loadingUnitId, "Install of dynamic feature module \"" + moduleName + "\" failed with a network error", true);
              break;
            case SplitInstallErrorCode.MODULE_UNAVAILABLE:
              Log.d(TAG, "Install of dynamic feature module \"" + moduleName + "\" failed as is unavailable.");
              flutterJNI.dynamicFeatureInstallFailure(moduleName, loadingUnitId,  "Install of dynamic feature module \"" + moduleName + "\" failed as is unavailable.", false);
              break;
            default:
              Log.d(TAG, "Install of dynamic feature module \"" + moduleName + "\" failed with error: \"" + ((SplitInstallException) exception).getErrorCode() + "\": " + ((SplitInstallException) exception).getMessage());
              flutterJNI.dynamicFeatureInstallFailure(moduleName, loadingUnitId,  "Install of dynamic feature module \"" + moduleName + "\" failed with error: \"" + ((SplitInstallException) exception).getErrorCode() + "\": " + ((SplitInstallException) exception).getMessage(), false);
              break;
          }
        });
  }

  public void extractFeature(@NonNull String moduleName, int loadingUnitId) {
    try {
      context = context.createPackageContext(context.getPackageName(), 0);

      AssetManager assetManager = context.getAssets();

      flutterJNI.updateAssetManager(
          assetManager,
          // TODO(garyq): Made the "flutter_assets" directory dynamic based off of DartEntryPoint.
          "flutter_assets");

      // We only load dart shared lib for the loading unit id requested. Other loading units (if present)
      // in the dynamic feature module are not loaded, but can be loaded by calling again with their
      // loading unit id.
      loadDartLibrary(moduleName, loadingUnitId);
    } catch (NameNotFoundException e) {
      Log.d(TAG, "NameNotFoundException creating context for " + moduleName);
      throw new RuntimeException(e);
    }
    // TODO: Handle assets here.
  }

  public void loadDartLibrary(String moduleName, int loadingUnitId) {
    // This matches/depends on dart's loading unit naming convention, which we use unchanged.
    String aotSharedLibraryName = "app.so-" + loadingUnitId + ".part.so";

    // Possible values: armeabi, armeabi-v7a, arm64-v8a, x86, x86_64, mips, mips64
    String abi;
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
        abi = Build.SUPPORTED_ABIS[0];
    } else {
        abi = Build.CPU_ABI;
    }
    String pathAbi = abi.replace("-", "_"); // abis are represented with underscores in paths.

    // TODO(garyq): Optimize this apk/file discovery process to use less i/o and be more
    // performant.

    // Search directly in APKs first
    List<String> apkPaths = new ArrayList();
    // If not found in APKs, we check in extracted native libs for the lib directly.
    String soPath = "";
    Queue<File> searchFiles = new LinkedList();
    searchFiles.add(context.getFilesDir());
    while (!searchFiles.isEmpty()) {
      File file = searchFiles.remove();
      if (file != null && file.isDirectory()) {
        for (File f : file.listFiles()) {
          searchFiles.add(f);
        }
        continue;
      }
      String name = file.getName();
      if (name.substring(name.length() - 4).equals(".apk") && name.substring(0, moduleName.length()).equals(moduleName) && name.contains(pathAbi)) {
        apkPaths.add(file.getAbsolutePath());
        continue;
      }
      if (name.equals(aotSharedLibraryName)) {
        soPath = file.getAbsolutePath();
      }
    }
  }

  public void uninstallFeature(String moduleName, int loadingUnitId) {
    // TODO(garyq): support uninstalling.
  }

  void destroy() {
    splitInstallManager.unregisterListener(listener);
  }

}
