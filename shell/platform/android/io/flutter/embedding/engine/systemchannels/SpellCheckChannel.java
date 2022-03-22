package io.flutter.embedding.engine.systemchannels;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import io.flutter.Log;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import java.util.ArrayList;
import java.util.Arrays;
import org.json.JSONArray;
import org.json.JSONException;

public class SpellCheckChannel {
  private static final String TAG = "SpellCheckChannel";

  @NonNull public final MethodChannel channel;
  @Nullable private SpellCheckMethodHandler spellCheckMethodHandler;

  @NonNull @VisibleForTesting
  final MethodChannel.MethodCallHandler parsingMethodHandler =
      new MethodChannel.MethodCallHandler() {
        @Override
        public void onMethodCall(@NonNull MethodCall call, @NonNull MethodChannel.Result result) {
          if (spellCheckMethodHandler == null) {
            // If no explicit SpellCheckMethodHandler has been registered then we don't
            // need to forward this call to an API. Return.
            return;
          }
          String method = call.method;
          Object args = call.arguments;
          Log.v(TAG, "Received '" + method + "' message.");
          switch (method) {
            case "SpellCheck.initiateSpellChecking":
              try {
                final JSONArray argumentList = (JSONArray) args;
                String locale = argumentList.getString(0);
                String text = argumentList.getString(1);
                spellCheckMethodHandler.initiateSpellChecking(locale, text);
                result.success(null);
              } catch (JSONException exception) {
                result.error("error", exception.getMessage(), null);
              }
              break;
            default:
              System.out.println(
              result.notImplemented();
              break;
          }
        }
      };

  public SpellCheckChannel(@NonNull DartExecutor dartExecutor) {
    // TODO(camillesimon): Use JSON?
    this.channel = new MethodChannel(dartExecutor, "flutter/spellcheck", JSONMethodCodec.INSTANCE);
    channel.setMethodCallHandler(parsingMethodHandler);
  }

  /** Responsible for sending spell checker results across to the framework. */
  public void updateSpellCheckerResults(
      ArrayList<String> spellCheckerResults, String spellCheckedText) {
    channel.invokeMethod(
        "SpellCheck.updateSpellCheckerResults",
        Arrays.asList(spellCheckerResults, spellCheckedText));
  }

  public void setSpellCheckMethodHandler(
      @Nullable SpellCheckMethodHandler spellCheckMethodHandler) {
    this.spellCheckMethodHandler = spellCheckMethodHandler;
  }

  public interface SpellCheckMethodHandler {
    /**
     * Requests that spell checking is initiated for the inputted text recognized by the framework,
     * which will automatically result in spell checking resutls being sent back to the framework.
     */
    void initiateSpellChecking(String locale, String text);
  }
}
