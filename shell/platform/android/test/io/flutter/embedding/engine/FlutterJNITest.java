package io.flutter.embedding.engine;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.embedding.engine.renderer.FlutterRenderer;
import io.flutter.embedding.engine.renderer.OnFirstFrameRenderedListener;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.mock;

import java.util.concurrent.atomic.AtomicInteger;

@Config(manifest=Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class FlutterJNITest {
  FlutterJNI jniUnderTest;

  @Before
  public void setUp() {
    jniUnderTest = new FlutterJNI();
  }

  @Test
  public void itAllowsFirstFrameListenersToRemoveThemselvesInline() {
    // --- Test Setup ---
    AtomicInteger callbackCalled = new AtomicInteger(0);
    OnFirstFrameRenderedListener callback = new OnFirstFrameRenderedListener() {
      @Override
      public void onFirstFrameRendered() {
        callbackCalled.incrementAndGet();
        jniUnderTest.removeOnFirstFrameRenderedListener(this);
      };
    };
    jniUnderTest.addOnFirstFrameRenderedListener(callback);

    // --- Execute Test ---
    jniUnderTest.onFirstFrame();

    // --- Verify Results ---
    assertEquals(1, callbackCalled.get());

    // --- Execute Test ---
    // The callback removed itself from the listener list. A second call doesn't call the callback.
    jniUnderTest.onFirstFrame();

    // --- Verify Results ---
    assertEquals(1, callbackCalled.get());
  }
}
