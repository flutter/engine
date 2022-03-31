// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.editing;

import android.content.Context;
import android.view.textservice.SentenceSuggestionsInfo;
import android.view.textservice.SpellCheckerSession;
import android.view.textservice.SuggestionsInfo;
import android.view.textservice.TextInfo;
import android.view.textservice.TextServicesManager;
import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import io.flutter.embedding.engine.systemchannels.SpellCheckChannel;
import java.util.ArrayList;
import java.util.Locale;

/** Android implementation of the spell check plugin. */
public class SpellCheckPlugin implements SpellCheckerSession.SpellCheckerSessionListener {

  @NonNull private final Context mContext;
  @NonNull private final SpellCheckChannel mSpellCheckChannel;
  @NonNull private final TextServicesManager tsm;
  private SpellCheckerSession mSpellCheckerSession;

  @VisibleForTesting @NonNull
  final SpellCheckChannel.SpellCheckMethodHandler mSpellCheckMethodHandler;

  public SpellCheckPlugin(@NonNull Context context, @NonNull SpellCheckChannel spellCheckChannel) {
    mContext = context;
    mSpellCheckChannel = spellCheckChannel;
    tsm = (TextServicesManager) mContext.getSystemService(Context.TEXT_SERVICES_MANAGER_SERVICE);

    mSpellCheckMethodHandler =
        new SpellCheckChannel.SpellCheckMethodHandler() {
          @Override
          public void initiateSpellCheck(String locale, String text) {
            performSpellCheck(locale, text);
          }
        };

    mSpellCheckChannel.setSpellCheckMethodHandler(mSpellCheckMethodHandler);
  }

  public void destroy() {
    mSpellCheckChannel.setSpellCheckMethodHandler(null);

    if (mSpellCheckerSession != null) {
      mSpellCheckerSession.close();
    }
  }

  /** Calls on the Android spell check API to spell check specified text. */
  public void performSpellCheck(String locale, String text) {
    String[] localeCodes = locale.split("-");
    Locale parsedLocale;

    if (localeCodes.length == 3) {
      parsedLocale = new Locale(localeCodes[0], localeCodes[1], localeCodes[2]);
    } else if (localeCodes.length == 2) {
      parsedLocale = new Locale(localeCodes[0], localeCodes[1]);
    } else {
      parsedLocale = new Locale(localeCodes[0]);
    }

    if (mSpellCheckerSession != null) {
      mSpellCheckerSession.close();
    }
    mSpellCheckerSession = tsm.newSpellCheckerSession(null, parsedLocale, this, true);

    TextInfo[] textInfos = new TextInfo[] {new TextInfo(text)};
    mSpellCheckerSession.getSentenceSuggestions(textInfos, 3);
  }

  /**
   * Callback for Android spell check API that decomposes results and send results through the
   * {@link SpellCheckChannel}.
   */
  @Override
  public void onGetSentenceSuggestions(SentenceSuggestionsInfo[] results) {
    ArrayList<String> spellCheckerSuggestionSpans = new ArrayList<String>();

    for (int i = 0; i < results[0].getSuggestionsCount(); i++) {
      SuggestionsInfo suggestionsInfo = results[0].getSuggestionsInfoAt(i);
      int suggestionsCount = suggestionsInfo.getSuggestionsCount();

      if (suggestionsCount > 0) {
        String spellCheckerSuggestionSpan = "";
        int start = results[0].getOffsetAt(i);
        int length = results[0].getLengthAt(i);

        spellCheckerSuggestionSpan += (String.valueOf(start) + ".");
        spellCheckerSuggestionSpan += (String.valueOf(start + (length - 1)) + ".");

        for (int j = 0; j < suggestionsCount; j++) {
          spellCheckerSuggestionSpan += (suggestionsInfo.getSuggestionAt(j) + ",");
        }

        spellCheckerSuggestionSpans.add(
            spellCheckerSuggestionSpan.substring(0, spellCheckerSuggestionSpan.length() - 1));
      }
    }

    mSpellCheckChannel.updateSpellCheckResults(spellCheckerSuggestionSpans);
  }

  @Override
  @SuppressWarnings("deprecation")
  public void onGetSuggestions(SuggestionsInfo[] results) {
    // Deprecated callback for Android spell check API; will not use.
  }
}
