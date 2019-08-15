package io.flutter.embedding.engine.renderer;

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
public class FlutterRendererTest {
  FlutterJNITest jniUnderTest;
  FlutterRenderer rendererUnderTest;

  @Before
  public void setUp() {
    jniUnderTest = new FlutterJNITest();
    rendererUnderTest = new FlutterRenderer(jniUnderTest);
  }

  @Test
  public void firstFrameListenerCanRemoveThemselves() {
    AtomicInteger callbackCalled = new AtomicInteger(0);
    OnFirstFrameRenderedListener callback = new OnFirstFrameRenderedListener() {
      @Override
      public void onFirstFrameRendered() {
        callbackCalled.incrementAndGet();
        rendererUnderTest.removeOnFirstFrameRenderedListener(this);
      };
    };

    rendererUnderTest.addOnFirstFrameRenderedListener(callback);
    jniUnderTest.onFirstFrame();

    assertEquals(1, callbackCalled.get());

    // The callback removed itself from the listener list. A second call doesn't call the callback.
    jniUnderTest.onFirstFrame();
    assertEquals(1, callbackCalled.get());
  }

  private static class FlutterJNITest extends FlutterJNI {
    protected void onFirstFrame() {
      // Exposed here for tests.
      super.onFirstFrame();
    }
  }
}
