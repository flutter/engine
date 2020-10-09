package io.flutter.embedding.engine.mutatorsstack;

import static junit.framework.TestCase.*;
import static org.mockito.Mockito.*;

import android.graphics.Matrix;
import android.view.MotionEvent;
import android.view.View;
import android.view.accessibility.AccessibilityEvent;
import io.flutter.embedding.android.AndroidTouchProcessor;
import io.flutter.plugin.platform.AccessibilityEventsDelegate;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class FlutterMutatorViewTest {

  @Test
  public void canDragViews() {
    final AndroidTouchProcessor touchProcessor = mock(AndroidTouchProcessor.class);
    final FlutterMutatorView view =
        new FlutterMutatorView(RuntimeEnvironment.systemContext, 1.0f, touchProcessor, null);
    final FlutterMutatorsStack mutatorStack = mock(FlutterMutatorsStack.class);

    assertTrue(view.onInterceptTouchEvent(mock(MotionEvent.class)));

    {
      view.readyToDisplay(mutatorStack, /*left=*/ 1, /*top=*/ 2, /*width=*/ 0, /*height=*/ 0);
      view.onTouchEvent(MotionEvent.obtain(0, 0, MotionEvent.ACTION_DOWN, 0.0f, 0.0f, 0));
      final ArgumentCaptor<Matrix> matrixCaptor = ArgumentCaptor.forClass(Matrix.class);
      verify(touchProcessor).onTouchEvent(any(), matrixCaptor.capture());

      final Matrix screenMatrix = new Matrix();
      screenMatrix.postTranslate(1, 2);
      assertTrue(matrixCaptor.getValue().equals(screenMatrix));
    }

    reset(touchProcessor);

    {
      view.readyToDisplay(mutatorStack, /*left=*/ 3, /*top=*/ 4, /*width=*/ 0, /*height=*/ 0);
      view.onTouchEvent(MotionEvent.obtain(0, 0, MotionEvent.ACTION_MOVE, 0.0f, 0.0f, 0));
      final ArgumentCaptor<Matrix> matrixCaptor = ArgumentCaptor.forClass(Matrix.class);
      verify(touchProcessor).onTouchEvent(any(), matrixCaptor.capture());

      final Matrix screenMatrix = new Matrix();
      screenMatrix.postTranslate(1, 2);
      assertTrue(matrixCaptor.getValue().equals(screenMatrix));
    }

    reset(touchProcessor);

    {
      view.readyToDisplay(mutatorStack, /*left=*/ 5, /*top=*/ 6, /*width=*/ 0, /*height=*/ 0);
      view.onTouchEvent(MotionEvent.obtain(0, 0, MotionEvent.ACTION_MOVE, 0.0f, 0.0f, 0));
      final ArgumentCaptor<Matrix> matrixCaptor = ArgumentCaptor.forClass(Matrix.class);
      verify(touchProcessor).onTouchEvent(any(), matrixCaptor.capture());

      final Matrix screenMatrix = new Matrix();
      screenMatrix.postTranslate(3, 4);
      assertTrue(matrixCaptor.getValue().equals(screenMatrix));
    }

    reset(touchProcessor);

    {
      view.readyToDisplay(mutatorStack, /*left=*/ 7, /*top=*/ 8, /*width=*/ 0, /*height=*/ 0);
      view.onTouchEvent(MotionEvent.obtain(0, 0, MotionEvent.ACTION_UP, 0.0f, 0.0f, 0));
      final ArgumentCaptor<Matrix> matrixCaptor = ArgumentCaptor.forClass(Matrix.class);
      verify(touchProcessor).onTouchEvent(any(), matrixCaptor.capture());

      final Matrix screenMatrix = new Matrix();
      screenMatrix.postTranslate(7, 8);
      assertTrue(matrixCaptor.getValue().equals(screenMatrix));
    }
  }

  @Test
  public void requestSendAccessibilityEvent_delegates() {
    final AndroidTouchProcessor touchProcessor = mock(AndroidTouchProcessor.class);
    final AccessibilityEventsDelegate delegate = mock(AccessibilityEventsDelegate.class);

    final FlutterMutatorView mutatorView =
        new FlutterMutatorView(RuntimeEnvironment.systemContext, 1.0f, touchProcessor, delegate);

    final View platformView = mock(View.class);
    mutatorView.addView(platformView);

    final View childView = mock(View.class);
    final AccessibilityEvent event = mock(AccessibilityEvent.class);

    mutatorView.requestSendAccessibilityEvent(childView, event);
    verify(delegate).requestSendAccessibilityEvent(platformView, childView, event);
  }
}
