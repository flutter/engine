// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.View;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.Log;
import io.flutter.embedding.engine.systemchannels.KeyEventChannel;
import io.flutter.embedding.engine.systemchannels.KeyEventChannel.FlutterKeyEvent;
import io.flutter.plugin.editing.TextInputPlugin;
import java.util.AbstractMap.SimpleImmutableEntry;
import java.util.ArrayDeque;
import java.util.Deque;
import java.util.Map.Entry;

/**
 * A class to process key events from Android, passing them to the framework as messages using
 * {@link KeyEventChannel}.
 *
 * <p>A class that sends Android key events to the framework, and re-dispatches those not handled by
 * the framework.
 *
 * <p>Flutter uses asynchronous event handling to avoid blocking the UI thread, but Android requires
 * that events are handled synchronously. So, when a key event is received by Flutter, it tells
 * Android synchronously that the key has been handled so that it won't propagate to other
 * components. Flutter then uses "delayed event synthesis", where it sends the event to the
 * framework, and if the framework responds that it has not handled the event, then this class
 * synthesizes a new event to send to Android, without handling it this time.
 */
public class AndroidKeyProcessor {
  private static final String TAG = "AndroidKeyProcessor";

  @NonNull private final KeyEventChannel keyEventChannel;
  @NonNull private final TextInputPlugin textInputPlugin;
  private int combiningCharacter;
  @NonNull private EventResponder eventResponder;

  /**
   * Constructor for AndroidKeyProcessor.
   *
   * <p>The view is used as the destination to send the synthesized key to. This means that the the
   * next thing in the focus chain will get the event when the framework returns false from
   * onKeyDown/onKeyUp
   *
   * <p>It is possible that that in the middle of the async round trip, the focus chain could
   * change, and instead of the native widget that was "next" when the event was fired getting the
   * event, it may be the next widget when the event is synthesized that gets it. In practice, this
   * shouldn't be a huge problem, as this is an unlikely occurrence to happen without user input,
   * and it may actually be desired behavior, but it is possible.
   *
   * @param view takes the activity to use for re-dispatching of events that were not handled by the
   *     framework.
   * @param keyEventChannel the event channel to listen to for new key events.
   * @param textInputPlugin a plugin, which, if set, is given key events before the framework is,
   *     and if it has a valid input connection and is accepting text, then it will handle the event
   *     and the framework will not receive it.
   */
  public AndroidKeyProcessor(
      @NonNull View view,
      @NonNull KeyEventChannel keyEventChannel,
      @NonNull TextInputPlugin textInputPlugin) {
    this.keyEventChannel = keyEventChannel;
    this.textInputPlugin = textInputPlugin;
    textInputPlugin.setKeyEventProcessor(this);
    this.eventResponder = new EventResponder(view, textInputPlugin);
    this.keyEventChannel.setEventResponseHandler(eventResponder);
  }

  /**
   * Detaches the key processor from the Flutter engine.
   *
   * <p>The AndroidKeyProcessor instance should not be used after calling this.
   */
  public void destroy() {
    keyEventChannel.setEventResponseHandler(null);
  }

  /**
   * Called when a key event is received by the {@link FlutterView} or the {@link
   * InputConnectionAdaptor}.
   *
   * @param keyEvent the Android key event to respond to.
   * @return true if the key event should not be propagated to other Android components. Delayed
   *     synthesis events will return false, so that other components may handle them.
   */
  public boolean onKeyEvent(@NonNull KeyEvent event) {
    int action = event.getAction();
    if (action != event.ACTION_DOWN && action != event.ACTION_UP) {
      // There is theoretically a KeyEvent.ACTION_MULTIPLE, but theoretically
      // that isn't sent by Android anymore, so this is just for protection in
      // case the theory is wrong.
      return false;
    }
    long eventId = FlutterKeyEvent.computeEventId(event);
    if (eventResponder.isHeadEvent(eventId)) {
      // If the event is at the head of the queue of pending events we've seen,
      // and has the same id, then we know that this is a re-dispatched event, and
      // we shouldn't respond to it, but we should remove it from tracking now.
      eventResponder.removePendingEvent(eventId);
      return false;
    }

    Character complexCharacter = applyCombiningCharacterToBaseCharacter(event.getUnicodeChar());
    KeyEventChannel.FlutterKeyEvent flutterEvent =
        new KeyEventChannel.FlutterKeyEvent(event, complexCharacter);

    eventResponder.addEvent(flutterEvent.eventId, event);
    if (action == KeyEvent.ACTION_DOWN) {
      keyEventChannel.keyDown(flutterEvent);
    } else {
      keyEventChannel.keyUp(flutterEvent);
    }
    return true;
  }

  /**
   * Returns whether or not the given event is currently being processed by this key processor. This
   * is used to determine if a new key event sent to the {@link InputConnectionAdaptor} originates
   * from a hardware key event, or a soft keyboard editing event.
   *
   * @param event the event to check for being the current event.
   * @return
   */
  public boolean isCurrentEvent(@NonNull KeyEvent event) {
    long id = FlutterKeyEvent.computeEventId(event);
    return eventResponder.isHeadEvent(id);
  }

  /**
   * Applies the given Unicode character in {@code newCharacterCodePoint} to a previously entered
   * Unicode combining character and returns the combination of these characters if a combination
   * exists.
   *
   * <p>This method mutates {@link #combiningCharacter} over time to combine characters.
   *
   * <p>One of the following things happens in this method:
   *
   * <ul>
   *   <li>If no previous {@link #combiningCharacter} exists and the {@code newCharacterCodePoint}
   *       is not a combining character, then {@code newCharacterCodePoint} is returned.
   *   <li>If no previous {@link #combiningCharacter} exists and the {@code newCharacterCodePoint}
   *       is a combining character, then {@code newCharacterCodePoint} is saved as the {@link
   *       #combiningCharacter} and null is returned.
   *   <li>If a previous {@link #combiningCharacter} exists and the {@code newCharacterCodePoint} is
   *       also a combining character, then the {@code newCharacterCodePoint} is combined with the
   *       existing {@link #combiningCharacter} and null is returned.
   *   <li>If a previous {@link #combiningCharacter} exists and the {@code newCharacterCodePoint} is
   *       not a combining character, then the {@link #combiningCharacter} is applied to the regular
   *       {@code newCharacterCodePoint} and the resulting complex character is returned. The {@link
   *       #combiningCharacter} is cleared.
   * </ul>
   *
   * <p>The following reference explains the concept of a "combining character":
   * https://en.wikipedia.org/wiki/Combining_character
   */
  @Nullable
  private Character applyCombiningCharacterToBaseCharacter(int newCharacterCodePoint) {
    if (newCharacterCodePoint == 0) {
      return null;
    }

    char complexCharacter = (char) newCharacterCodePoint;
    boolean isNewCodePointACombiningCharacter =
        (newCharacterCodePoint & KeyCharacterMap.COMBINING_ACCENT) != 0;
    if (isNewCodePointACombiningCharacter) {
      // If a combining character was entered before, combine this one with that one.
      int plainCodePoint = newCharacterCodePoint & KeyCharacterMap.COMBINING_ACCENT_MASK;
      if (combiningCharacter != 0) {
        combiningCharacter = KeyCharacterMap.getDeadChar(combiningCharacter, plainCodePoint);
      } else {
        combiningCharacter = plainCodePoint;
      }
    } else {
      // The new character is a regular character. Apply combiningCharacter to it, if
      // it exists.
      if (combiningCharacter != 0) {
        int combinedChar = KeyCharacterMap.getDeadChar(combiningCharacter, newCharacterCodePoint);
        if (combinedChar > 0) {
          complexCharacter = (char) combinedChar;
        }
        combiningCharacter = 0;
      }
    }

    return complexCharacter;
  }

  private static class EventResponder implements KeyEventChannel.EventResponseHandler {
    // The maximum number of pending events that are held before starting to
    // complain.
    private static final long MAX_PENDING_EVENTS = 1000;
    final Deque<Entry<Long, KeyEvent>> pendingEvents = new ArrayDeque<Entry<Long, KeyEvent>>();
    @NonNull private final View view;
    @NonNull private final TextInputPlugin textInputPlugin;

    public EventResponder(@NonNull View view, @NonNull TextInputPlugin textInputPlugin) {
      this.view = view;
      this.textInputPlugin = textInputPlugin;
    }

    /**
     * Removes the pending event with the given id from the cache of pending events.
     *
     * @param id the id of the event to be removed.
     */
    private KeyEvent removePendingEvent(long id) {
      if (pendingEvents.getFirst().getKey() != id) {
        throw new AssertionError(
            "Event response received out of order. Should have seen event "
                + pendingEvents.getFirst().getKey()
                + " first. Instead, received "
                + id);
      }
      return pendingEvents.removeFirst().getValue();
    }

    private KeyEvent findPendingEvent(long id) {
      if (pendingEvents.size() == 0) {
        throw new AssertionError(
            "Event response received when no events are in the queue. Received id " + id);
      }
      if (pendingEvents.getFirst().getKey() != id) {
        throw new AssertionError(
            "Event response received out of order. Should have seen event "
                + pendingEvents.getFirst().getKey()
                + " first. Instead, received "
                + id);
      }
      return pendingEvents.getFirst().getValue();
    }

    private boolean isHeadEvent(long id) {
      return pendingEvents.size() > 0 && pendingEvents.getFirst().getKey() == id;
    }

    /**
     * Called whenever the framework responds that a given key event was handled by the framework.
     *
     * @param id the event id of the event to be marked as being handled by the framework. Must not
     *     be null.
     */
    @Override
    public void onKeyEventHandled(long id) {
      removePendingEvent(id);
    }

    /**
     * Called whenever the framework responds that a given key event wasn't handled by the
     * framework.
     *
     * @param id the event id of the event to be marked as not being handled by the framework. Must
     *     not be null.
     */
    @Override
    public void onKeyEventNotHandled(long id) {
      dispatchKeyEvent(findPendingEvent(id), id);
    }

    /** Adds an Android key event with an id to the event responder to wait for a response. */
    public void addEvent(long id, @NonNull KeyEvent event) {
      pendingEvents.addLast(new SimpleImmutableEntry<Long, KeyEvent>(id, event));
      if (pendingEvents.size() > MAX_PENDING_EVENTS) {
        Log.e(
            TAG,
            "There are "
                + pendingEvents.size()
                + " keyboard events that have not yet received a response. Are responses being "
                + "sent?");
      }
    }

    /**
     * Dispatches the event to the activity associated with the context.
     *
     * @param event the event to be dispatched to the activity.
     */
    public void dispatchKeyEvent(KeyEvent event, long id) {
      // If the textInputPlugin is still valid and accepting text, then we'll try
      // and send the key event to it, assuming that if the event can be sent,
      // that it has been handled.
      if (textInputPlugin.getInputMethodManager().isAcceptingText()
          && textInputPlugin.getLastInputConnection() != null
          && textInputPlugin.getLastInputConnection().sendKeyEvent(event)) {
        // The event was handled, so we can remove it from the queue.
        removePendingEvent(id);
        return;
      }

      // Since the framework didn't handle it, dispatch the event again.
      if (view != null) {
        view.getRootView().dispatchKeyEvent(event);
      }
    }
  }
}
