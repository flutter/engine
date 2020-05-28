// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.os.Build;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.Log;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/** Sends the platform's locales to Dart. */
public class LocalizationChannel {
  private static final String TAG = "LocalizationChannel";

  @NonNull public final MethodChannel channel;
  @Nullable private LocalizationMethodHandler localizationMethodHandler;

  private MethodChannel.MethodCallHandler parsingMethodHandler =
      new MethodChannel.MethodCallHandler() {
        @Override
        public void onMethodCall(@NonNull MethodCall call, @NonNull MethodChannel.Result result) {
          if (localizationMethodHandler == null) {
            // If no explicit LocalizationMethodHandler has been registered then we don't
            // need to forward this call to an API. Return.
            return;
          }

          String method = call.method;
          Object args = call.arguments;
          Log.v(TAG, "Received '" + method + "' message.");
          switch (method) {
            case "Localization.resolveLocale":
              try {
                final JSONObject arguments = (JSONObject) args;
                final int length = arguments.getInt("count");
                final JSONArray supportedLocalesData = arguments.getJSONArray("localeData");
                List<Locale> supportedLocales = new ArrayList<Locale>();
                for (int i = 0; i < length; i++) {
                  // Java locales only support language, country, and variant.
                  final String language = supportedLocalesData.getString(i * 2 + 0);
                  final String country = supportedLocalesData.getString(i * 2 + 1);
                  supportedLocales.add(new Locale(language, country));
                }
                Locale resolvedLocale = localizationMethodHandler.resolveLocale(supportedLocales);
                result.success(resolvedLocale.toString());
              } catch (JSONException exception) {
                result.error("error", exception.getMessage(), null);
              }
              break;
            default:
              result.notImplemented();
              break;
          }
        }
      };

  public LocalizationChannel(@NonNull DartExecutor dartExecutor) {
    this.channel =
        new MethodChannel(dartExecutor, "flutter/localization", JSONMethodCodec.INSTANCE);
    channel.setMethodCallHandler(parsingMethodHandler);
  }

  /** Send the given {@code locales} to Dart. */
  public void sendLocales(@NonNull List<Locale> locales) {
    Log.v(TAG, "Sending Locales to Flutter.");
    // Send the user's preferred locales.
    List<String> data = new ArrayList<>();
    for (Locale locale : locales) {
      Log.v(
          TAG,
          "Locale (Language: "
              + locale.getLanguage()
              + ", Country: "
              + locale.getCountry()
              + ", Variant: "
              + locale.getVariant()
              + ")");
      data.add(locale.getLanguage());
      data.add(locale.getCountry());
      // locale.getScript() was added in API 21.
      data.add(Build.VERSION.SDK_INT >= 21 ? locale.getScript() : "");
      data.add(locale.getVariant());
    }
    channel.invokeMethod("setLocale", data);
  }

    /** Send the given {@code locales} to Dart. */
  public void sendPlatformResolvedLocales(Locale platformResolvedLocale) {
    Log.v(TAG, "Sending Locales to Flutter.");
    // Send platformResolvedLocale first as it may be used in the callback
    // triggered by the user supported locales being updated/set.
    if (platformResolvedLocale != null) {
      List<String> platformResolvedLocaleData = new ArrayList<>();
      platformResolvedLocaleData.add(platformResolvedLocale.getLanguage());
      platformResolvedLocaleData.add(platformResolvedLocale.getCountry());
      platformResolvedLocaleData.add(
          Build.VERSION.SDK_INT >= 21 ? platformResolvedLocale.getScript() : "");
      platformResolvedLocaleData.add(platformResolvedLocale.getVariant());
      channel.invokeMethod("setPlatformResolvedLocale", platformResolvedLocaleData);
    }
  }

  /**
   * Sets the {@link TextInputMethodHandler} which receives all events and requests that are parsed
   * from the underlying platform channel.
   */
  public void setLocalizationMethodHandler(@Nullable LocalizationMethodHandler localizationMethodHandler) {
    this.localizationMethodHandler = localizationMethodHandler;
  }

  public interface LocalizationMethodHandler {
    /** Performs the android native locale resolution.  */
    Locale resolveLocale(List<Locale> supportedLocales);
  }
}
