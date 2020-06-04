package io.flutter.embedding.engine;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.mock;

import io.flutter.embedding.engine.renderer.FlutterUiDisplayListener;
import java.util.concurrent.atomic.AtomicInteger;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;
import org.mockito.Mock;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class FlutterJNITest {
  @Test
  public void itAllowsFirstFrameListenersToRemoveThemselvesInline() {
    // --- Test Setup ---
    FlutterJNI flutterJNI = new FlutterJNI();

    AtomicInteger callbackInvocationCount = new AtomicInteger(0);
    FlutterUiDisplayListener callback =
        new FlutterUiDisplayListener() {
          @Override
          public void onFlutterUiDisplayed() {
            callbackInvocationCount.incrementAndGet();
            flutterJNI.removeIsDisplayingFlutterUiListener(this);
          }

          @Override
          public void onFlutterUiNoLongerDisplayed() {}
        };
    flutterJNI.addIsDisplayingFlutterUiListener(callback);

    // --- Execute Test ---
    flutterJNI.onFirstFrame();

    // --- Verify Results ---
    assertEquals(1, callbackInvocationCount.get());

    // --- Execute Test ---
    // The callback removed itself from the listener list. A second call doesn't call the callback.
    flutterJNI.onFirstFrame();

    // --- Verify Results ---
    assertEquals(1, callbackInvocationCount.get());
  }

  @Test 
  public void onDisplayPlatformView__callsPlatformViewsController() {
    int[] platformViewProps = new int[5];
    int counter = 0;

    FlutterJNI flutterJNI = new FlutterJNI();
    flutterJNI.setPlatformViewsController(new PlatformViewsController() {
      @override 
      public void onDisplayPlatformView(int viewId, int x, int y, int width, int height) {
        PlatformViewsController platformViewsController = mock(PlatformViewsController.class);
        if (platformViewsController == null) {
          throw new RuntimeException(
              "platformViewsController must be set before attempting to position a platform view");
        }

        platformViewsController.onDisplayPlatformView(viewId, x, y, width, height);
        platformViewProps[0] = viewId;
        platformViewProps[1] = x;
        platformViewProps[2] = y;
        platformViewProps[3] = width;
        platformViewProps[4] = height;
        counter++;
      }
    });

    // --- Execute Test ---
    flutterJNI.onDisplayPlatformView(/*viewId=*/ 1, /*x=*/ 10, /*y=*/ 20, /*width=*/ 100, /*height=*/ 200);

    // --- Verify Results ---
    assertEquals(1, counter);
    assertEquals(1, platformViewProps[0]);
    assertEquals(10, platformViewProps[1]);
    assertEquals(20, platformViewProps[2]);
    assertEquals(100, platformViewProps[3]);
    assertEquals(200, platformViewProps[4]);
  }
}
