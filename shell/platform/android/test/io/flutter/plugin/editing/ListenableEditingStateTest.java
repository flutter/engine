package io.flutter.plugin.editing;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.mock;

import android.text.Editable;
import android.text.Selection;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import io.flutter.embedding.engine.systemchannels.TextInputChannel;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class ListenableEditingStateTest {
  private BaseInputConnection getTestInputConnection(View view, Editable mEditable) {
    new View(RuntimeEnvironment.application);
    return new BaseInputConnection(view, true) {
      @Override
      public Editable getEditable() {
        return mEditable;
      }
    };
  }

  // -------- Start: Test BatchEditing   -------
  @Test
  public void testBatchEditing() {
    final ListenableEditingState editingState =
        new ListenableEditingState(null, new View(RuntimeEnvironment.application));
    final Listener listener = new Listener();
    final View testView = new View(RuntimeEnvironment.application);
    final BaseInputConnection inputConnection = getTestInputConnection(testView, editingState);

    editingState.addEditingStateListener(listener);

    editingState.replace(0, editingState.length(), "update");
    assertTrue(listener.called);
    assertTrue(listener.textChanged);
    assertFalse(listener.selectionChanged);
    assertFalse(listener.composingRegionChanged);

    assertEquals(-1, editingState.getSelecionStart());
    assertEquals(-1, editingState.getSelecionEnd());

    listener.reset();

    // Batch edit depth = 1.
    editingState.beginBatchEdit();
    editingState.replace(0, editingState.length(), "update1");
    assertFalse(listener.called);
    // Batch edit depth = 2.
    editingState.beginBatchEdit();
    editingState.replace(0, editingState.length(), "update2");
    inputConnection.setComposingRegion(0, editingState.length());
    assertFalse(listener.called);
    // Batch edit depth = 1.
    editingState.endBatchEdit();
    assertFalse(listener.called);

    // Batch edit depth = 2.
    editingState.beginBatchEdit();
    assertFalse(listener.called);
    inputConnection.setSelection(0, 0);
    assertFalse(listener.called);
    // Batch edit depth = 1.
    editingState.endBatchEdit();
    assertFalse(listener.called);

    // Remove composing region.
    inputConnection.finishComposingText();

    // Batch edit depth = 0. Last endBatchEdit.
    editingState.endBatchEdit();

    // Now notify the listener.
    assertTrue(listener.called);
    assertTrue(listener.textChanged);
    assertFalse(listener.composingRegionChanged);
  }

  @Test
  public void testBatchingEditing_callEndBeforeBegin() {
    final ListenableEditingState editingState =
        new ListenableEditingState(null, new View(RuntimeEnvironment.application));
    final Listener listener = new Listener();
    editingState.addEditingStateListener(listener);

    editingState.endBatchEdit();
    assertFalse(listener.called);

    editingState.replace(0, editingState.length(), "text");
    assertTrue(listener.called);
    assertTrue(listener.textChanged);

    listener.reset();
    // Does not disrupt the followup events.
    editingState.beginBatchEdit();
    editingState.replace(0, editingState.length(), "more text");
    assertFalse(listener.called);
    editingState.endBatchEdit();
    assertTrue(listener.called);
  }

  @Test
  public void testBatchingEditing_addListenerDuringBatchEdit() {
    final ListenableEditingState editingState =
        new ListenableEditingState(null, new View(RuntimeEnvironment.application));
    final Listener listener = new Listener();

    editingState.beginBatchEdit();
    editingState.addEditingStateListener(listener);
    editingState.replace(0, editingState.length(), "update");
    editingState.endBatchEdit();
    editingState.removeEditingStateListener(listener);

    assertTrue(listener.called);
    assertTrue(listener.textChanged);
    assertTrue(listener.selectionChanged);
    assertTrue(listener.composingRegionChanged);

    listener.reset();

    // Now remove before endBatchEdit();
    editingState.beginBatchEdit();
    editingState.addEditingStateListener(listener);
    editingState.replace(0, editingState.length(), "update");
    editingState.removeEditingStateListener(listener);
    editingState.endBatchEdit();

    assertFalse(listener.called);
  }

  @Test
  public void testBatchingEditing_removeListenerDuringBatchEdit() {
    final ListenableEditingState editingState =
        new ListenableEditingState(null, new View(RuntimeEnvironment.application));
    final Listener listener = new Listener();
    editingState.addEditingStateListener(listener);

    editingState.beginBatchEdit();
    editingState.replace(0, editingState.length(), "update");
    editingState.removeEditingStateListener(listener);
    editingState.endBatchEdit();

    assertFalse(listener.called);
  }
  // -------- End: Test BatchEditing   -------

  @Test
  public void testSetComposingRegion() {
    final ListenableEditingState editingState =
        new ListenableEditingState(null, new View(RuntimeEnvironment.application));
    editingState.replace(0, editingState.length(), "text");

    // (-1, -1) clears the composing region.
    editingState.setComposingRange(-1, -1);
    assertEquals(-1, editingState.getComposingStart());
    assertEquals(-1, editingState.getComposingEnd());

    editingState.setComposingRange(-1, 5);
    assertEquals(-1, editingState.getComposingStart());
    assertEquals(-1, editingState.getComposingEnd());

    editingState.setComposingRange(2, 3);
    assertEquals(2, editingState.getComposingStart());
    assertEquals(3, editingState.getComposingEnd());

    // Empty range is invalid. Clears composing region.
    editingState.setComposingRange(1, 1);
    assertEquals(-1, editingState.getComposingStart());
    assertEquals(-1, editingState.getComposingEnd());

    // Covers everything.
    editingState.setComposingRange(0, editingState.length());
    assertEquals(0, editingState.getComposingStart());
    assertEquals(editingState.length(), editingState.getComposingEnd());
  }

  // -------- Start: Test InputMethods actions   -------
  @Test
  public void inputMethod_testSetSelection() {
    final ListenableEditingState editingState =
        new ListenableEditingState(null, new View(RuntimeEnvironment.application));
    final Listener listener = new Listener();
    final View testView = new View(RuntimeEnvironment.application);
    final InputConnectionAdaptor inputConnection =
        new InputConnectionAdaptor(
            testView, 0, mock(TextInputChannel.class), editingState, new EditorInfo());
    editingState.replace(0, editingState.length(), "initial text");

    editingState.addEditingStateListener(listener);

    inputConnection.setSelection(0, 0);

    assertTrue(listener.called);
    assertFalse(listener.textChanged);
    assertTrue(listener.selectionChanged);
    assertFalse(listener.composingRegionChanged);

    listener.reset();

    inputConnection.setSelection(5, 5);

    assertTrue(listener.called);
    assertFalse(listener.textChanged);
    assertTrue(listener.selectionChanged);
    assertFalse(listener.composingRegionChanged);
  }

  @Test
  public void inputMethod_testSetComposition() {
    final ListenableEditingState editingState =
        new ListenableEditingState(null, new View(RuntimeEnvironment.application));
    final Listener listener = new Listener();
    final View testView = new View(RuntimeEnvironment.application);
    final InputConnectionAdaptor inputConnection =
        new InputConnectionAdaptor(
            testView, 0, mock(TextInputChannel.class), editingState, new EditorInfo());
    editingState.replace(0, editingState.length(), "initial text");

    editingState.addEditingStateListener(listener);

    // setComposingRegion test.
    inputConnection.setComposingRegion(1, 3);
    assertTrue(listener.called);
    assertFalse(listener.textChanged);
    assertFalse(listener.selectionChanged);
    assertTrue(listener.composingRegionChanged);

    Selection.setSelection(editingState, 0, 0);
    listener.reset();

    // setComposingText test: non-empty text, does not move cursor.
    inputConnection.setComposingText("composing", -1);
    assertTrue(listener.called);
    assertTrue(listener.textChanged);
    assertFalse(listener.selectionChanged);
    assertTrue(listener.composingRegionChanged);

    listener.reset();
    // setComposingText test: non-empty text, moves cursor.
    inputConnection.setComposingText("composing2", 1);
    assertTrue(listener.called);
    assertTrue(listener.textChanged);
    assertTrue(listener.selectionChanged);
    assertTrue(listener.composingRegionChanged);

    listener.reset();
    // setComposingText test: empty text.
    inputConnection.setComposingText("", 1);
    assertTrue(listener.called);
    assertTrue(listener.textChanged);
    assertTrue(listener.selectionChanged);
    assertTrue(listener.composingRegionChanged);

    // finishComposingText test.
    inputConnection.setComposingText("composing text", 1);
    listener.reset();
    inputConnection.finishComposingText();
    assertTrue(listener.called);
    assertFalse(listener.textChanged);
    assertFalse(listener.selectionChanged);
    assertTrue(listener.composingRegionChanged);
  }

  @Test
  public void inputMethod_testCommitText() {
    final ListenableEditingState editingState =
        new ListenableEditingState(null, new View(RuntimeEnvironment.application));
    final Listener listener = new Listener();
    final View testView = new View(RuntimeEnvironment.application);
    final InputConnectionAdaptor inputConnection =
        new InputConnectionAdaptor(
            testView, 0, mock(TextInputChannel.class), editingState, new EditorInfo());
    editingState.replace(0, editingState.length(), "initial text");

    editingState.addEditingStateListener(listener);
  }
  // -------- End: Test InputMethods actions   -------

  public static class Listener implements ListenableEditingState.EditingStateWatcher {
    boolean called = false;
    boolean textChanged = false;
    boolean selectionChanged = false;
    boolean composingRegionChanged = false;

    @Override
    public void didChangeEditingState(
        boolean textChanged, boolean selectionChanged, boolean composingRegionChanged) {
      called = true;
      this.textChanged = textChanged;
      this.selectionChanged = selectionChanged;
      this.composingRegionChanged = composingRegionChanged;
    }

    public void reset() {
      called = false;
      textChanged = false;
      selectionChanged = false;
      composingRegionChanged = false;
    }
  }
}
