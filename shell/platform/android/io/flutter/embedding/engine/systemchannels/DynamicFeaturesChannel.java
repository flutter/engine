// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.os.Build;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.FlutterInjector;
import io.flutter.Log;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.dynamicfeatures.DynamicFeatureManager;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import org.json.JSONException;
import org.json.JSONObject;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class DynamicFeaturesChannel {
  private static final String TAG = "DynamicFeaturesChannel";

  @NonNull public final MethodChannel channel;
  @Nullable DynamicFeatureManager dynamicFeatureManager;
  @NonNull Map<String, List<MethodChannel.Result>> moduleNameToResults;


  private final MethodChannel.MethodCallHandler parsingMethodHandler =
      new MethodChannel.MethodCallHandler() {
        @Override
        public void onMethodCall(@NonNull MethodCall call, @NonNull MethodChannel.Result result) {
          if (dynamicFeatureManager == null) {
            // If no DynamicFeatureManager has been injected, then this channel is a no-op.
            return;
          }
          String method = call.method;
          Object args = call.arguments;
          Log.v(TAG, "Received '" + method + "' message.");
          try {
            final JSONObject arguments = (JSONObject) args;
            final int loadingUnitId = arguments.getInt("loadingUnitId");
            final String moduleName = arguments.getString("moduleName");
            switch (method) {
              case "DynamicFeatures.installDynamicFeature":
                dynamicFeatureManager.installDynamicFeature(loadingUnitId, moduleName);
                if (!moduleNameToResults.containsKey(moduleName)) {
                  moduleNameToResults.put(moduleName, new ArrayList<>());
                } else {
                  moduleNameToResults.get(moduleName).add(result);
                }
                break;
              case "DynamicFeatures.getDynamicFeatureInstallState":
                result.success(dynamicFeatureManager.getDynamicFeatureInstallState(loadingUnitId, moduleName));
                break;
              default:
                result.notImplemented();
                break;
            }
          } catch (JSONException exception) {
            result.error("error", exception.getMessage(), null);
          }
        }
      };

  /**
   * Constructs a {@code DynamicFeaturesChannel} that connects Android to the Dart code running in {@code
   * dartExecutor}.
   *
   * <p>The given {@code dartExecutor} is permitted to be idle or executing code.
   *
   * <p>See {@link DartExecutor}.
   */
  public DynamicFeaturesChannel(@NonNull DartExecutor dartExecutor) {
    this.channel =
        new MethodChannel(dartExecutor, "flutter/splitaot", JSONMethodCodec.INSTANCE);
    channel.setMethodCallHandler(parsingMethodHandler);
    dynamicFeatureManager = FlutterInjector.instance().dynamicFeatureManager();
    moduleNameToResults = new HashMap<>();
  }

  public void completeInstallSuccess(String moduleName) {
    if (moduleNameToResults.containsKey(moduleName)) {
      for (MethodChannel.Result result : moduleNameToResults.get(moduleName)) {
        result.success(null);
      }
      moduleNameToResults.get(moduleName).clear();
    }
    return;
  }

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
