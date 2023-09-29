// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.text;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Build;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.embedding.engine.plugins.FlutterPlugin;
import io.flutter.embedding.engine.plugins.activity.ActivityAware;
import io.flutter.embedding.engine.plugins.activity.ActivityPluginBinding;
import io.flutter.embedding.engine.systemchannels.ProcessTextChannel;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.PluginRegistry.ActivityResultListener;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ProcessTextPlugin implements FlutterPlugin, ActivityAware, ActivityResultListener {
  private static final String TAG = "ProcessTextPlugin";

  @NonNull private final ProcessTextChannel processTextChannel;
  @NonNull private final PackageManager packageManager;
  @Nullable private ActivityPluginBinding activityBinding;
  private Map<Integer, ResolveInfo> resolveInfosById;

  @NonNull
  private Map<Integer, MethodChannel.Result> requestsByCode =
      new HashMap<Integer, MethodChannel.Result>();

  public ProcessTextPlugin(@NonNull ProcessTextChannel processTextChannel) {
    this.processTextChannel = processTextChannel;
    this.packageManager = processTextChannel.packageManager;

    processTextChannel.setMethodHandler(
        new ProcessTextChannel.ProcessTextMethodHandler() {
          @Override
          public Map<Integer, String> queryTextActions() {
            if (resolveInfosById == null) {
              resolveInfosById = new HashMap<Integer, ResolveInfo>();
              cacheResolveInfos();
            }
            Map<Integer, String> result = new HashMap<Integer, String>();
            for (Integer id : resolveInfosById.keySet()) {
              final ResolveInfo info = resolveInfosById.get(id);
              result.put(id, info.loadLabel(packageManager).toString());
            }
            return result;
          }

          @Override
          public void processTextAction(
              @NonNull int id,
              @NonNull String text,
              @NonNull boolean readOnly,
              @NonNull MethodChannel.Result result) {
            if (activityBinding == null) {
              result.error("error", "Plugin not bound to an Activity", null);
              return;
            }

            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
              result.error("error", "Android version not supported", null);
              return;
            }

            if (resolveInfosById == null) {
              result.error("error", "Can not process text actions before calling queryTextActions", null);
              return;
            }

            final ResolveInfo info = resolveInfosById.get(id);
            if (info == null) {
              result.error("error", "Text processing activity not found", null);
              return;
            }

            Integer requestCode = result.hashCode();
            requestsByCode.put(requestCode, result);

            Intent intent =
                new Intent()
                    .setClassName(info.activityInfo.packageName, info.activityInfo.name)
                    .setAction(Intent.ACTION_PROCESS_TEXT)
                    .setType("text/plain")
                    .putExtra(Intent.EXTRA_PROCESS_TEXT, text)
                    .putExtra(Intent.EXTRA_PROCESS_TEXT_READONLY, readOnly);

            // Start the text processing activity. onActivityResult callback is called
            // when the activity completes.
            activityBinding.getActivity().startActivityForResult(intent, requestCode);
          }
        });
  }

  private void cacheResolveInfos() {
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
      return;
    }

    Intent intent = new Intent().setAction(Intent.ACTION_PROCESS_TEXT).setType("text/plain");

    List<ResolveInfo> infos;
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
      infos = packageManager.queryIntentActivities(intent, PackageManager.ResolveInfoFlags.of(0));
    } else {
      infos = packageManager.queryIntentActivities(intent, 0);
    }

    // Assign an internal id for communication between the engine and the framework.
    int index = 0;
    resolveInfosById.clear();
    for (ResolveInfo info : infos) {
      final String label = info.loadLabel(packageManager).toString();
      resolveInfosById.put(index++, info);
    }
  }

  /**
   * Executed when a text processing activity terminates.
   *
   * <p>The result is null when an activity does not return an updated text.
   */
  public boolean onActivityResult(int requestCode, int resultCode, @Nullable Intent intent) {
    String result = null;
    if (resultCode == Activity.RESULT_OK) {
      result = intent.getStringExtra(Intent.EXTRA_PROCESS_TEXT);
    }
    requestsByCode.remove(requestCode).success(result);
    return true;
  }

  /**
   * Unregisters this {@code ProcessTextPlugin} as the {@code
   * ProcessTextChannel.ProcessTextMethodHandler}, for the {@link
   * io.flutter.embedding.engine.systemchannels.ProcessTextChannel}.
   *
   * <p>Do not invoke any methods on a {@code ProcessTextPlugin} after invoking this method.
   */
  public void destroy() {
    processTextChannel.setMethodHandler(null);
  }

  // FlutterPlugin interface implementation.

  public void onAttachedToEngine(@NonNull FlutterPluginBinding binding) {
    // Nothing to do because this plugin is instantiated by the engine.
  }

  public void onDetachedFromEngine(@NonNull FlutterPluginBinding binding) {
    // Nothing to do because this plugin is instantiated by the engine.
  }

  // ActivityAware interface implementation.
  //
  // Store the binding and manage the activity result listerner.

  public void onAttachedToActivity(@NonNull ActivityPluginBinding binding) {
    this.activityBinding = binding;
    this.activityBinding.addActivityResultListener(this);
  };

  public void onDetachedFromActivityForConfigChanges() {
    this.activityBinding.removeActivityResultListener(this);
    this.activityBinding = null;
  }

  public void onReattachedToActivityForConfigChanges(@NonNull ActivityPluginBinding binding) {
    this.activityBinding = binding;
    this.activityBinding.addActivityResultListener(this);
  }

  public void onDetachedFromActivity() {
    this.activityBinding.removeActivityResultListener(this);
    this.activityBinding = null;
  }
}
