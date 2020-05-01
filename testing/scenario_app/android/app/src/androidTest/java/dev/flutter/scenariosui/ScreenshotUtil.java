// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package dev.flutter.scenariosui;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Xml;
import androidx.test.InstrumentationRegistry;
import androidx.test.runner.AndroidJUnitRunner;
import androidx.test.runner.screenshot.Screenshot;
import com.facebook.testing.screenshot.ScreenshotRunner;
import com.facebook.testing.screenshot.internal.AlbumImpl;
import com.facebook.testing.screenshot.internal.Registry;
import com.facebook.testing.screenshot.internal.TestNameDetector;
import dev.flutter.scenarios.TestableFlutterActivity;
import java.io.BufferedOutputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import org.xmlpull.v1.XmlSerializer;

/**
 * Adapter for {@code com.facebook.testing.screenshot.Screenshot} that supports Flutter apps.
 *
 * <p>{@code com.facebook.testing.screenshot.Screenshot} relies on {@code View#draw(canvas)}, which
 * doesn't draw Flutter's Surface or SurfaceTexture.
 *
 * <p>The workaround takes a full screenshot of the device and removes the status and action bars.
 */
public class ScreenshotUtil {
  private XmlSerializer mXmlSerializer;
  private AlbumImpl album;
  private OutputStream mOutputStream;

  private static ScreenshotUtil instance;
  private static int BUFFER_SIZE = 1 << 16; // 64K

  private static ScreenshotUtil getInstance() {
    synchronized (ScreenshotUtil.class) {
      if (instance == null) {
        instance = new ScreenshotUtil();
      }
      return instance;
    }
  }

  /** Starts the album, which contains the screenshots in a zip file, and a metadata.xml file. */
  void init() {
    if (mXmlSerializer != null) {
      return;
    }
    album = AlbumImpl.create(Registry.getRegistry().instrumentation.getContext(), "default");
    album.cleanup();

    mXmlSerializer = Xml.newSerializer();
    try {
      mOutputStream =
          new BufferedOutputStream(new FileOutputStream(album.getMetadataFile()), BUFFER_SIZE);
    } catch (FileNotFoundException e) {
      throw new RuntimeException(e);
    }
    try {
      mXmlSerializer.setOutput(mOutputStream, "utf-8");
      mXmlSerializer.startDocument("utf-8", null);
      // Start tag <screenshots>.
      mXmlSerializer.startTag(null, "screenshots");
    } catch (IOException e) {
      throw new RuntimeException(e);
    }
  }

  void writeText(String tagName, String value) throws IOException {
    if (mXmlSerializer == null) {
      throw new RuntimeException("ScreenshotUtil must be initialized.");
    }
    mXmlSerializer.startTag(null, tagName);
    mXmlSerializer.text(value);
    mXmlSerializer.endTag(null, tagName);
  }

  void writeBitmap(Bitmap bitmap, String name, String testClass, String testName)
      throws IOException {
    if (mXmlSerializer == null) {
      throw new RuntimeException("ScreenshotUtil must be initialized.");
    }
    album.writeBitmap(name, 0, 0, bitmap);

    mXmlSerializer.startTag(null, "screenshot");
    writeText("name", name);
    writeText("test_class", testClass);
    writeText("test_name", testName);
    writeText("tile_width", "1");
    writeText("tile_height", "1");
    mXmlSerializer.endTag(null, "screenshot");
  }

  /** Finishes metadata.xml. */
  void flush() {
    if (mXmlSerializer == null) {
      throw new RuntimeException("ScreenshotUtil must be initialized.");
    }
    try {
      // End tag </screenshots>
      mXmlSerializer.endTag(null, "screenshots");
      mXmlSerializer.endDocument();
      mXmlSerializer.flush();
    } catch (IOException e) {
      throw new RuntimeException(e);
    }
    try {
      mOutputStream.close();
    } catch (IOException e) {
      throw new RuntimeException(e);
    }

    album.flush();

    mXmlSerializer = null;
    mOutputStream = null;
    album = null;
  }

  private static int getStatusBarHeight() {
    final Context context = InstrumentationRegistry.getTargetContext();
    final int resourceId =
        context.getResources().getIdentifier("status_bar_height", "dimen", "android");
    int statusBarHeight = 0;
    if (resourceId > 0) {
      statusBarHeight = context.getResources().getDimensionPixelSize(resourceId);
    }
    return statusBarHeight;
  }

  private static int getActionBarHeight(Activity activity) {
    int actionBarHeight = 0;
    final android.content.res.TypedArray styledAttributes =
        activity.getTheme().obtainStyledAttributes(new int[] {android.R.attr.actionBarSize});
    actionBarHeight = (int) styledAttributes.getDimension(0, 0);
    styledAttributes.recycle();
    return actionBarHeight;
  }

  /**
   * Captures a screenshot of {@code TestableFlutterActivity}.
   *
   * <p>The activity must be already launched.
   */
  public static void capture(TestableFlutterActivity activity)
      throws InterruptedException, ExecutionException, IOException {
    // Yield and wait for the engine to render the first Flutter frame.
    activity.waitUntilFlutterRendered();

    // This method is called from the runner thread,
    // so block the UI thread while taking the screenshot.
    // UiThreadLocker locker = new UiThreadLocker();
    // locker.lock();

    // Screenshot.capture(view or activity) does not capture the Flutter UI.
    // Unfortunately, it doesn't work with Android's `Surface` or `TextureSurface`.
    //
    // As a result, capture a screenshot of the entire device and then clip
    // the status and action bars.
    //
    // Under the hood, this call is similar to `adb screencap`, which is used
    // to capture screenshots.
    final String testClass = TestNameDetector.getTestClass();
    final String testName = TestNameDetector.getTestName();

    runCallableOnUiThread(
        new Callable<Void>() {
          @Override
          public Void call() {
            Bitmap bitmap = Screenshot.capture().getBitmap();
            // Remove the status and action bars from the screenshot capture.
            bitmap =
                Bitmap.createBitmap(
                    bitmap,
                    0,
                    getStatusBarHeight(),
                    bitmap.getWidth(),
                    bitmap.getHeight() - getStatusBarHeight() - getActionBarHeight(activity));

            final String screenshotName = String.format("%s__%s", testClass, testName);
            // Write bitmap to the album.
            try {
              ScreenshotUtil.getInstance().writeBitmap(bitmap, screenshotName, testClass, testName);
            } catch (IOException e) {
              throw new RuntimeException(e);
            }
            return null;
          }
        });
  }

  /**
   * Initializes the {@code com.facebook.testing.screenshot.internal.Album}.
   *
   * <p>Call this method from {@code AndroidJUnitRunner#onCreate}.
   */
  public static void onCreate(AndroidJUnitRunner runner, Bundle arguments) {
    ScreenshotRunner.onCreate(runner, arguments);
    ScreenshotUtil.getInstance().init();
  }

  /**
   * Flushes the {@code com.facebook.testing.screenshot.internal.Album}.
   *
   * <p>Call this method from {@code AndroidJUnitRunner#onDestroy}.
   */
  public static void onDestroy() {
    ScreenshotRunner.onDestroy();
    ScreenshotUtil.getInstance().flush();
  }

  private static void runCallableOnUiThread(final Callable<Void> callable) {
    if (Looper.getMainLooper().getThread() == Thread.currentThread()) {
      try {
        callable.call();
      } catch (Exception e) {
        e.printStackTrace();
      }
      return;
    }
    Handler handler = new Handler(Looper.getMainLooper());
    final Object lock = new Object();
    synchronized (lock) {
      handler.post(
          new Runnable() {
            @Override
            public void run() {
              try {
                callable.call();
              } catch (Exception e) {
                e.printStackTrace();
              }
              synchronized (lock) {
                lock.notifyAll();
              }
            }
          });
      try {
        lock.wait();
      } catch (InterruptedException e) {
        throw new RuntimeException(e);
      }
    }
  }
}
