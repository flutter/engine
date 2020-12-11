// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.os.Build;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import io.flutter.FlutterInjector;
import io.flutter.Log;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.dynamicfeatures.DynamicFeatureManager;
import io.flutter.plugin.common.StandardMethodCodec;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class DynamicFeatureChannel {
  private static final String TAG = "DynamicFeatureChannel";

  @NonNull public final MethodChannel channel;
  @Nullable DynamicFeatureManager dynamicFeatureManager;
  @NonNull Map<String, List<MethodChannel.Result>> moduleNameToResults;

  @NonNull @VisibleForTesting
  final MethodChannel.MethodCallHandler parsingMethodHandler =
      new MethodChannel.MethodCallHandler() {
        @Override
        public void onMethodCall(@NonNull MethodCall call, @NonNull MethodChannel.Result result) {
          if (dynamicFeatureManager == null) {
            // If no DynamicFeatureManager has been injected, then this channel is a no-op.
            return;
          }
          String method = call.method;
          Map<String, Object> args = call.arguments();
          Log.v(TAG, "Received '" + method + "' message.");
          final int loadingUnitId = (int) args.get("loadingUnitId");
          final String moduleName = (String) args.get("moduleName");
          switch (method) {
            case "installDynamicFeature":
              dynamicFeatureManager.installDynamicFeature(loadingUnitId, moduleName);
              if (!moduleNameToResults.containsKey(moduleName)) {
                moduleNameToResults.put(moduleName, new ArrayList<>());
              }
              moduleNameToResults.get(moduleName).add(result);
              break;
            case "getDynamicFeatureInstallState":
              result.success(dynamicFeatureManager.getDynamicFeatureInstallState(loadingUnitId, moduleName));
              break;
            default:
              result.notImplemented();
              break;
          }
        }
      };

  /**
   * Constructs a {@code DynamicFeatureChannel} that connects Android to the Dart code running in {@code
   * dartExecutor}.
   *
   * <p>The given {@code dartExecutor} is permitted to be idle or executing code.
   *
   * <p>See {@link DartExecutor}.
   */
  public DynamicFeatureChannel(@NonNull DartExecutor dartExecutor, @Nullable DynamicFeatureManager featureManager) {
    this.channel =
        new MethodChannel(dartExecutor, "flutter/dynamicfeature", StandardMethodCodec.INSTANCE);
    channel.setMethodCallHandler(parsingMethodHandler);
    dynamicFeatureManager = featureManager != null ? featureManager : FlutterInjector.instance().dynamicFeatureManager();
    moduleNameToResults = new HashMap<>();
  }

  /**
   * Finishes the `installDynamicFeature` method channel call for the specified moduleName
   * with a success.
   *
   * @param moduleName The name of the android dynamic feature module install request to complete.
   */
  public void completeInstallSuccess(String moduleName) {
    if (moduleNameToResults.containsKey(moduleName)) {
      for (MethodChannel.Result result : moduleNameToResults.get(moduleName)) {
        result.success(null);
      }
      moduleNameToResults.get(moduleName).clear();
    }
    return;
  }

  /**
   * Finishes the `installDynamicFeature` method channel call for the specified moduleName
   * with an error/failure.
   *
   * @param moduleName The name of the android dynamic feature module install request to complete.
   * @param errorMessage The error message to display to complete the future with.
   */
  public void completeInstallError(String moduleName, String errorMessage) {
    if (moduleNameToResults.containsKey(moduleName)) {
      for (MethodChannel.Result result : moduleNameToResults.get(moduleName)) {
        result.error("DynamicFeature Install failure", errorMessage, null);
      }
      moduleNameToResults.get(moduleName).clear();
    }
    return;
  }
}
