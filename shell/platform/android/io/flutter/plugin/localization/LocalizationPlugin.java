// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.localization;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.Configuration;
import android.os.Build;
import android.os.LocaleList;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import io.flutter.embedding.engine.systemchannels.LocalizationChannel;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

/** Android implementation of the localization plugin. */
public class LocalizationPlugin {
  @NonNull private final LocalizationChannel localizationChannel;
  @NonNull private final Context context;

  // This should always be initialized before being used by resolveNativeLocale
  // since the embedder starts the engine which in turn calls resolveNativeLocale.
  private static LocalizationPlugin instance;

  public LocalizationPlugin(
      @NonNull Context context,
      @NonNull LocalizationChannel localizationChannel) {

    this.context = context;
    this.localizationChannel = localizationChannel;
    instance = this;
  }

  public static Locale resolveNativeLocale(List<Locale> supportedLocales) {
    Locale platformResolvedLocale = null;
    if (Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
      List<Locale.LanguageRange> languageRanges = new ArrayList<>();
      LocaleList localeList = instance.context.getResources().getConfiguration().getLocales();
      int localeCount = localeList.size();
      for (int index = 0; index < localeCount; ++index) {
        Locale locale = localeList.get(index);
        String localeString = locale.toString();
        languageRanges.add(new Locale.LanguageRange(localeString.replace("_", "-")));
      }

      // TODO(garyq): This should be modified to achieve Android's full
      // locale resolution: https://developer.android.com/guide/topics/resources/multilingual-support
      platformResolvedLocale =
          Locale.lookup(languageRanges, supportedLocales);
    }
    return platformResolvedLocale;
  }

  /**
   * Send the current {@link Locale} configuration to Flutter.
   *
   * <p>FlutterEngine must be non-null when this method is invoked.
   */
  @SuppressWarnings("deprecation")
  public void sendLocalesToFlutter(@NonNull Configuration config) {
    List<Locale> locales = new ArrayList<>();
    if (Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
      LocaleList localeList = config.getLocales();
      int localeCount = localeList.size();
      for (int index = 0; index < localeCount; ++index) {
        Locale locale = localeList.get(index);
        locales.add(locale);
      }
    } else {
      locales.add(config.locale);
    }

    Locale platformResolvedLocale = null;
    if (Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
      List<Locale.LanguageRange> languageRanges = new ArrayList<>();
      LocaleList localeList = config.getLocales();
      int localeCount = localeList.size();
      for (int index = 0; index < localeCount; ++index) {
        Locale locale = localeList.get(index);
        languageRanges.add(new Locale.LanguageRange(locale.toLanguageTag()));
      }
      // TODO(garyq) implement a real locale resolution.
      platformResolvedLocale =
          Locale.lookup(languageRanges, Arrays.asList(Locale.getAvailableLocales()));
    }

    localizationChannel.sendLocales(locales);
    // Do not initialize platform resolved locale. We will do
    // this later via the method channel call "resolveLocale".
    // localizationChannel.sendPlatformResolvedLocales(platformResolvedLocale);
  }
}
