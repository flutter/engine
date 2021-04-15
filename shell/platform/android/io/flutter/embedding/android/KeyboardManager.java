// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.view.KeyEvent;
import android.view.View;
import androidx.annotation.NonNull;
import io.flutter.Log;
import io.flutter.embedding.android.KeyboardManager.PrimaryResponder.OnKeyEventHandledCallback;
import io.flutter.embedding.engine.systemchannels.KeyEventChannel;
import io.flutter.plugin.editing.TextInputPlugin;
import java.util.HashSet;

/**
 * A class to process {@link KeyEvent}s dispatched to a {@link FlutterView}.
 *
 * <p>A class that sends Android key events to the currently registered {@link PrimaryResponder}s,
 * and re-dispatches those not handled by the primary responders.
 *
 * <p>Flutter uses asynchronous event handling to avoid blocking the UI thread, but Android requires
 * that events are handled synchronously. So, when a key event is received by Flutter, it tells
 * Android synchronously that the key has been handled so that it won't propagate to other
 * components. Flutter then uses "delayed event synthesis", where it sends the event to the
 * framework, and if the framework responds that it has not handled the event, then this class
 * synthesizes a new event to send to Android, without handling it this time.
 *
 * <p>A new {@link KeyEvent} sent to a {@link KeyboardManager} may be processed by 3 different types
 * of "responder"s:
 *
 * <ul>
 *   <li>{@link PrimaryResponder}s: the {@link KeyboardManager} calls the {@link
 *       PrimaryResponder#handleEvent(KeyEvent, OnKeyEventHandledCallback)} method on the currently
 *       registered {@link PrimaryResponder}s. When each {@link PrimaryResponder} has decided wether
 *       to handle the key event, it must call the supplied {@link OnKeyEventHandledCallback}
 *       callback. More than one {@link PrimaryResponder} is allowed to reply true and handle the
 *       same {@link KeyEvent}.
 *   <li>{@link TextInputPlugin}: if every {@link PrimaryResponder} has replied false to a {@link
 *       KeyEvent}, the {@link KeyEvent} will be sent to the currently focused editable text field
 *       in {@link TextInputPlugin}, if any.
 *   <li><b>"Redispatch"</b>: if there's no currently focused text field in {@link TextInputPlugin},
 *       or the text field does not handle the {@link KeyEvent} either, the {@link KeyEvent} will be
 *       sent back to the top of the activity's view hierachy, allowing the {@link KeyEvent} to be
 *       "redispatched", only this time the {@link KeyboardManager} will not try to handle the
 *       redispatched {@link KeyEvent}.
 * </ul>
 */
public class KeyboardManager {
  private static final String TAG = "KeyboardManager";

  KeyboardManager(
      View view, @NonNull TextInputPlugin textInputPlugin, PrimaryResponder[] primaryResponders) {
    this.view = view;
    this.textInputPlugin = textInputPlugin;
    this.primaryResponders = primaryResponders;
  }

  public KeyboardManager(
      View view, @NonNull TextInputPlugin textInputPlugin, KeyEventChannel keyEventChannel) {
    this(
        view,
        textInputPlugin,
        new KeyboardManager.PrimaryResponder[] {new KeyChannelResponder(keyEventChannel)});
  }

  /**
   * The interface for responding to a {@link KeyEvent} asynchronously.
   *
   * <p>Implementers of this interface should be added to a {@link KeyboardManager} using the {@link
   * KeyboardManager#addPrimaryResponder(PrimaryResponder)}, in order to receive key events.
   *
   * <p>After receiving a {@link KeyEvent}, the {@link PrimaryResponder} must call the supplied
   * {@link OnKeyEventHandledCallback} to inform the {@link KeyboardManager} whether it is capable
   * of handling the {@link KeyEvent}.
   *
   * <p>If a {@link PrimaryResponder} fails to call the {@link OnKeyEventHandledCallback} callback,
   * the {@link KeyEvent} will never be sent to the {@link TextInputPlugin}, and the {@link
   * KeyboardManager} class can't detect such errors as there is no timeout.
   */
  interface PrimaryResponder {
    interface OnKeyEventHandledCallback {
      void onKeyEventHandled(Boolean canHandleEvent);
    }

    /**
     * Informs this {@link PrimaryResponder} that a new {@link KeyEvent} needs processing.
     *
     * @param keyEvent the new {@link KeyEvent} this {@link PrimaryResponder} may be interested in.
     * @param onKeyEventHandledCallback the method to call when this {@link PrimaryResponder} has
     *     decided whether to handle {@link keyEvent}.
     */
    void handleEvent(
        @NonNull KeyEvent keyEvent, @NonNull OnKeyEventHandledCallback onKeyEventHandledCallback);
  }

  private class PerEventCallbackBuilder {
    private class Callback implements OnKeyEventHandledCallback {
      boolean isCalled = false;

      @Override
      public void onKeyEventHandled(Boolean canHandleEvent) {
        if (isCalled) {
          throw new IllegalStateException(
              "The onKeyEventHandledCallback should be called exactly once.");
        }
        isCalled = true;
        unrepliedCount -= 1;
        isEventHandled |= canHandleEvent;
        if (unrepliedCount == 0 && !isEventHandled) {
          onUnhandled(keyEvent);
        }
      }
    }

    PerEventCallbackBuilder(@NonNull KeyEvent keyEvent) {
      this.keyEvent = keyEvent;
    }

    @NonNull final KeyEvent keyEvent;
    int unrepliedCount = primaryResponders.length;
    boolean isEventHandled = false;

    public OnKeyEventHandledCallback buildCallback() {
      return new Callback();
    }
  }

  @NonNull protected final PrimaryResponder[] primaryResponders;
  @NonNull private final HashSet<KeyEvent> redispatchedEvents = new HashSet<>();
  @NonNull private final TextInputPlugin textInputPlugin;
  private final View view;

  public boolean handleEvent(@NonNull KeyEvent keyEvent) {
    final boolean isRedispatchedEvent = redispatchedEvents.remove(keyEvent);
    if (isRedispatchedEvent) {
      return !isRedispatchedEvent;
    }

    if (primaryResponders.length > 0) {
      final PerEventCallbackBuilder callbackBuilder = new PerEventCallbackBuilder(keyEvent);
      for (final PrimaryResponder primaryResponder : primaryResponders) {
        primaryResponder.handleEvent(keyEvent, callbackBuilder.buildCallback());
      }
    } else {
      onUnhandled(keyEvent);
    }

    return !isRedispatchedEvent;
  }

  public void destroy() {
    final int remainingRedispatchCount = redispatchedEvents.size();
    if (remainingRedispatchCount > 0) {
      Log.w(
          TAG,
          "A KeyboardManager was destroyed with "
              + String.valueOf(remainingRedispatchCount)
              + " unhandled redispatch event(s).");
    }
  }

  private void onUnhandled(@NonNull KeyEvent keyEvent) {
    if (textInputPlugin.handleKeyEvent(keyEvent) || view == null) {
      return;
    }

    redispatchedEvents.add(keyEvent);
    view.getRootView().dispatchKeyEvent(keyEvent);
    if (redispatchedEvents.remove(keyEvent)) {
      Log.w(TAG, "A redispatched key event was consumed before reaching KeyboardManager");
    }
  }
}
