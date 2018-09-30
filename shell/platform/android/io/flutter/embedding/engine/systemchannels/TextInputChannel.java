package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.InputType;
import android.view.inputmethod.EditorInfo;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.Arrays;
import java.util.HashMap;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;

/**
 * {@link TextInputChannel} is a platform channel between Android and Flutter that is used to
 * communicate information about the user's text input.
 *
 * When the user presses an action button like "done" or "next", that action is sent from Android
 * to Flutter through this {@link TextInputChannel}.
 *
 * When an input system in the Flutter app wants to show the keyboard, or hide it, or configure
 * editing state, etc. a message is sent from Flutter to Android through this {@link TextInputChannel}.
 *
 * {@link TextInputChannel} comes with a default {@link io.flutter.plugin.common.MethodChannel.MethodCallHandler}
 * that parses incoming messages from Flutter. Register a {@link TextInputMethodHandler} to respond
 * to standard Flutter text input messages.
 *
 * To handle all incoming text input messages in a custom manner, developers may override the
 * default {@code MethodCallHandler} by using {@link #overrideDefaultMethodHandler(MethodChannel.MethodCallHandler)}.
 * Overriding the default {@code MethodCallHandler} prevents {@link TextInputMethodHandler} from
 * being invoked. All incoming message parsing responsibilities must be handled by the
 * {@code MethodCallHandler} that overrode the default.
 *
 * To restore the original default message handling responsibilities, call {@link #restoreDefaultMethodHandler()}.
 */
public class TextInputChannel {

  public final MethodChannel channel;
  private TextInputMethodHandler textInputMethodHandler;

  private final MethodChannel.MethodCallHandler parsingMethodHandler = new MethodChannel.MethodCallHandler() {
    @Override
    public void onMethodCall(MethodCall call, MethodChannel.Result result) {
      if (textInputMethodHandler == null) {
        // If no explicit TextInputMethodHandler has been registered then we don't
        // need to forward this call to an API. Return.
        return;
      }

      try {
        String method = call.method;
        Object args = call.arguments;
        switch (method) {
          case "TextInput.show":
            textInputMethodHandler.show();
            result.success(null);
            break;
          case "TextInput.hide":
            textInputMethodHandler.hide();
            result.success(null);
            break;
          case "TextInput.setClient":
            final JSONArray argumentList = (JSONArray) args;
            final int textInputClientId = argumentList.getInt(0);
            final JSONObject jsonConfiguration = argumentList.getJSONObject(1);
            textInputMethodHandler.setClient(textInputClientId, Configuration.fromJson(jsonConfiguration));
            result.success(null);
            break;
          case "TextInput.setEditingState":
            final JSONObject editingState = (JSONObject) args;
            textInputMethodHandler.setEditingState(TextEditState.fromJson(editingState));
            result.success(null);
            break;
          case "TextInput.clearClient":
            textInputMethodHandler.clearClient();
            result.success(null);
            break;
          default:
            result.notImplemented();
            break;
        }
      } catch (Exception exception) {
        result.error("error", "JSON error: " + exception.getMessage(), null);
      }
    }
  };

  public TextInputChannel(@NonNull DartExecutor dartExecutor) {
    this.channel = new MethodChannel(dartExecutor, "flutter/textinput", JSONMethodCodec.INSTANCE);
    channel.setMethodCallHandler(parsingMethodHandler);
  }

  public void updateEditingState(int inputClientId, String text, int selectionStart, int selectionEnd, int composingStart, int composingEnd) {
    HashMap<Object, Object> state = new HashMap<>();
    state.put("text", text);
    state.put("selectionBase", selectionStart);
    state.put("selectionExtent", selectionEnd);
    state.put("composingBase", composingStart);
    state.put("composingExtent", composingEnd);

    channel.invokeMethod(
        "TextInputClient.updateEditingState",
        Arrays.asList(inputClientId, state)
    );
  }

  public void newline(int inputClientId) {
    channel.invokeMethod(
        "TextInputClient.performAction",
        Arrays.asList(inputClientId, "TextInputAction.newline")
    );
  }

  public void go(int inputClientId) {
    channel.invokeMethod(
        "TextInputClient.performAction",
        Arrays.asList(inputClientId, "TextInputAction.go")
    );
  }

  public void search(int inputClientId) {
    channel.invokeMethod(
        "TextInputClient.performAction",
        Arrays.asList(inputClientId, "TextInputAction.search")
    );
  }

  public void send(int inputClientId) {
    channel.invokeMethod(
        "TextInputClient.performAction",
        Arrays.asList(inputClientId, "TextInputAction.send")
    );
  }

  public void done(int inputClientId) {
    channel.invokeMethod(
        "TextInputClient.performAction",
        Arrays.asList(inputClientId, "TextInputAction.done")
    );
  }

  public void next(int inputClientId) {
    channel.invokeMethod(
        "TextInputClient.performAction",
        Arrays.asList(inputClientId, "TextInputAction.next")
    );
  }

  public void previous(int inputClientId) {
    channel.invokeMethod(
        "TextInputClient.performAction",
        Arrays.asList(inputClientId, "TextInputAction.previous")
    );
  }

  public void unspecifiedAction(int inputClientId) {
    channel.invokeMethod(
        "TextInputClient.performAction",
        Arrays.asList(inputClientId, "TextInputAction.unspecified")
    );
  }

  // TODO(mattcarroll): javadoc
  public void setTextInputMethodHandler(@Nullable TextInputMethodHandler textInputMethodHandler) {
    this.textInputMethodHandler = textInputMethodHandler;
  }

  // TODO(mattcarroll): javadoc
  public void overrideDefaultMethodHandler(@NonNull MethodChannel.MethodCallHandler methodCallHandler) {
    channel.setMethodCallHandler(methodCallHandler);
  }

  // TODO(mattcarroll): javadoc
  public void restoreDefaultMethodHandler() {
    channel.setMethodCallHandler(parsingMethodHandler);
  }

  public interface TextInputMethodHandler {
    // TODO(mattcarroll): javadoc
    void show();

    // TODO(mattcarroll): javadoc
    void hide();

    // TODO(mattcarroll): javadoc
    void setClient(int textInputClientId, Configuration configuration);

    // TODO(mattcarroll): javadoc
    void setEditingState(TextEditState editingState);

    // TODO(mattcarroll): javadoc
    void clearClient();
  }

  public static class Configuration {
    public static Configuration fromJson(JSONObject json) throws JSONException {
      final Integer inputAction = inputActionFromTextInputAction(json.getString("inputAction"));
      return new Configuration(
          json.optBoolean("obscureText"),
          json.optBoolean("autocorrect", true),
          TextCapitalization.fromValue(json.getString("textCapitalization")),
          InputType.fromJson(json.getJSONObject("inputType")),
          inputAction,
          json.optString("actionLabel")
      );
    }

    private static Integer inputActionFromTextInputAction(String inputAction) {
      if (inputAction == null) {
        return null;
      }

      switch (inputAction) {
        case "TextInputAction.newline":
          return EditorInfo.IME_ACTION_NONE;
        case "TextInputAction.none":
          return EditorInfo.IME_ACTION_NONE;
        case "TextInputAction.unspecified":
          return EditorInfo.IME_ACTION_UNSPECIFIED;
        case "TextInputAction.done":
          return EditorInfo.IME_ACTION_DONE;
        case "TextInputAction.go":
          return EditorInfo.IME_ACTION_GO;
        case "TextInputAction.search":
          return EditorInfo.IME_ACTION_SEARCH;
        case "TextInputAction.send":
          return EditorInfo.IME_ACTION_SEND;
        case "TextInputAction.next":
          return EditorInfo.IME_ACTION_NEXT;
        case "TextInputAction.previous":
          return EditorInfo.IME_ACTION_PREVIOUS;
        default:
          // Present default key if bad input type is given.
          return EditorInfo.IME_ACTION_UNSPECIFIED;
      }
    }

    public final boolean obscureText;
    public final boolean autocorrect;
    public final TextCapitalization textCapitalization;
    public final InputType inputType;
    public final Integer inputAction;
    public final String actionLabel;

    public Configuration(
      boolean obscureText,
      boolean autocorrect,
      @NonNull TextCapitalization textCapitalization,
      @NonNull InputType inputType,
      @Nullable Integer inputAction,
      @Nullable String actionLabel
    ) {
      this.obscureText = obscureText;
      this.autocorrect = autocorrect;
      this.textCapitalization = textCapitalization;
      this.inputType = inputType;
      this.inputAction = inputAction;
      this.actionLabel = actionLabel;
    }
  }

  public static class InputType {
    public static InputType fromJson(JSONObject json) throws JSONException {
      return new InputType(
        TextInputType.fromValue(json.getString("name")),
        json.optBoolean("signed", false),
        json.optBoolean("decimal", false)
      );
    }

    public final TextInputType type;
    public final boolean isSigned;
    public final boolean isDecimal;

    public InputType(@NonNull TextInputType type, boolean isSigned, boolean isDecimal) {
      this.type = type;
      this.isSigned = isSigned;
      this.isDecimal = isDecimal;
    }
  }

  public enum TextInputType {
    DATETIME("TextInputType.datetime"),
    NUMBER("TextInputType.number"),
    PHONE("TextInputType.phone"),
    MULTILINE("TextInputType.multiline"),
    EMAIL_ADDRESS("TextInputType.emailAddress"),
    URL("TextInputType.url");

    static TextInputType fromValue(String encodedName) {
      for (TextInputType textInputType : TextInputType.values()) {
        if (textInputType.encodedName.equals(encodedName)) {
          return textInputType;
        }
      }
      throw new RuntimeException("No such TextInputType: " + encodedName);
    }

    private final String encodedName;

    TextInputType(String encodedName) {
      this.encodedName = encodedName;
    }
  }

  public enum TextCapitalization {
    CHARACTERS("TextCapitalization.characters"),
    WORDS("TextCapitalization.words"),
    SENTENCES("TextCapitalization.sentences");

    static TextCapitalization fromValue(String encodedName) {
      for (TextCapitalization textCapitalization : TextCapitalization.values()) {
        if (textCapitalization.encodedName.equals(encodedName)) {
          return textCapitalization;
        }
      }
      throw new RuntimeException("No such TextCapitalization: " + encodedName);
    }

    private final String encodedName;

    TextCapitalization(String encodedName) {
      this.encodedName = encodedName;
    }
  }

  public static class TextEditState {
    public static TextEditState fromJson(JSONObject textEditState) throws JSONException {
      return new TextEditState(
          textEditState.getString("text"),
          textEditState.getInt("selectionBase"),
          textEditState.getInt("selectionExtent")
      );
    }

    public final String text;
    public final int selectionStart;
    public final int selectionEnd;

    public TextEditState(@NonNull String text, int selectionStart, int selectionEnd) {
      this.text = text;
      this.selectionStart = selectionStart;
      this.selectionEnd = selectionEnd;
    }
  }
}
