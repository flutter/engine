// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.editing;

import android.content.Context;
import android.view.View;
import android.view.textservice.SentenceSuggestionsInfo;
import android.view.textservice.SpellCheckerInfo;
import android.view.textservice.SpellCheckerSession;
import android.view.textservice.SuggestionsInfo;
import android.view.textservice.TextInfo;
import android.view.textservice.TextServicesManager;
import androidx.annotation.NonNull;
import io.flutter.embedding.engine.systemchannels.SpellCheckChannel;
import java.util.ArrayList;
import java.util.Locale;

public class SpellCheckPlugin implements SpellCheckerSession.SpellCheckerSessionListener {

  @NonNull private final View mView;
  @NonNull private final SpellCheckChannel mSpellCheckChannel;
  @NonNull private final TextServicesManager tsm;
  private SpellCheckerSession mSpellCheckerSession;

  // String of spell checked text for testing
  private String currentSpellCheckedText;

  public SpellCheckPlugin(@NonNull View view, @NonNull SpellCheckChannel spellCheckChannel) {
    mView = view;
    mSpellCheckChannel = spellCheckChannel;
    tsm =
        (TextServicesManager)
            view.getContext().getSystemService(Context.TEXT_SERVICES_MANAGER_SERVICE);

    mSpellCheckChannel.setSpellCheckMethodHandler(
        new SpellCheckChannel.SpellCheckMethodHandler() {
          @Override
          public void initiateSpellChecking(String locale, String text) {
            currentSpellCheckedText = text;
            performSpellCheck(locale, text);
          }
        });
  }

  // Responsible for calling the Android spell checker API to retrieve spell
  // checking results.
  public void performSpellCheck(String locale, String text) {
    String[] localeCodes = locale.split("-");
    Locale localeToUse;

    if (localeCodes.length == 3) {
      localeToUse = new Locale(localeCodes[0], localeCodes[1], localeCodes[2]);
    } else if (localeCodes.length == 2) {
      localeToUse = new Locale(localeCodes[0], localeCodes[1]);
    } else {
      localeToUse = new Locale(localeCodes[0]);
    }

    // Open a new spell checker session when a new text input client is set.
    // Closes spell checker session if one previously in use.
    // TODO(camillesimon): Figure out proper session management.
    if (mSpellCheckerSession != null) {
      mSpellCheckerSession.close();
      mSpellCheckerSession = tsm.newSpellCheckerSession(null, Locale.ENGLISH, this, true);
    } else {
      mSpellCheckerSession = tsm.newSpellCheckerSession(null, Locale.ENGLISH, this, true);
    }
    SpellCheckerInfo infoChecker = mSpellCheckerSession.getSpellChecker();

    // Define TextInfo[] object (textInfos) based on the current input to be
    // spell checked.
    TextInfo[] textInfos = new TextInfo[] {new TextInfo(text)};

    // Make API call. Maximum suggestions requested set to 3 for now.
    mSpellCheckerSession.getSentenceSuggestions(textInfos, 3);
  }

  // Responsible for decomposing spell checker results into an object that can
  // then be sent to the framework.
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
          String key = "suggestion_" + String.valueOf(j);
          spellCheckerSuggestionSpan += (suggestionsInfo.getSuggestionAt(j) + ",");
        }

        spellCheckerSuggestionSpans.add(
            spellCheckerSuggestionSpan.substring(0, spellCheckerSuggestionSpan.length() - 1));
      }
    }

    // Make call to update the spell checker results in the framework.
    // Current text being passed for testing purposes.
    // TODO(camillesimon): Don't pass text back to framework.
    mSpellCheckChannel.updateSpellCheckerResults(
        spellCheckerSuggestionSpans, currentSpellCheckedText);
  }

  @Override
  public void onGetSuggestions(SuggestionsInfo[] results) {
    // Callback for a deprecated method, so will not use.
  }
}
