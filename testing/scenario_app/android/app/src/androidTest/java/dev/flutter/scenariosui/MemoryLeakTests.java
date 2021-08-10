// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package dev.flutter.scenariosui;

import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.test.filters.LargeTest;
import androidx.test.rule.ActivityTestRule;
import androidx.test.runner.AndroidJUnit4;
import dev.flutter.scenarios.StrictModeFlutterActivity;
import dev.flutter.scenarios.TextPlatformViewActivity;
import io.flutter.embedding.engine.FlutterJNI;
import leakcanary.FailTestOnLeak;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
@LargeTest
public class MemoryLeakTests {
  private static ByteBuffer encodeMethodCall(MethodCall call) {
    final ByteBuffer buffer = StandardMethodCodec.INSTANCE.encodeMethodCall(call);
    buffer.rewind();
    return buffer;
  }
  
  @Rule @NonNull
  public ActivityTestRule<TextPlatformViewActivity> activityRule =
      new ActivityTestRule<>(
          TextPlatformViewActivity.class, /*initialTouchMode=*/ false, /*launchActivity=*/ false);

  @Rule @NonNull
  public ActivityTestRule<TextPlatformViewActivity> strictActivityRule =
      new ActivityTestRule<>(
          StrictModeFlutterActivity.class, /*initialTouchMode=*/ false, /*launchActivity=*/ false);

  @Test
  @FailTestOnLeak
  public void platformViewHybridComposition_launchActivityFinishAndLaunchAgain() throws Exception {
    Intent intent = new Intent(Intent.ACTION_MAIN);
    intent.putExtra("scenario_name", "platform_view");
    intent.putExtra("use_android_view", true);

    activityRule.launchActivity(intent);
  }

  @Test
  @FailTestOnLeak
  public void platformViewHybridComposition_launchActivityFinishAndLaunchAgain() throws Exception {
    Intent intent = new Intent(Intent.ACTION_MAIN);
    intent.putExtra("scenario_name", "platform_view");
    intent.putExtra("use_android_view", true);

    activityRule.launchActivity(intent);
  }

  @Test
  @FailTestOnLeak
  public void platformViewHybridComposition_releaseRenderSurface() throws Exception {
    strictActivityRule.launchActivity(intent);
    
    final PlatformViewsController platformViewsController = new PlatformViewsController();

    final int platformViewId = 0;
    assertNull(platformViewsController.getPlatformViewById(platformViewId));

    final PlatformViewFactory viewFactory = mock(PlatformViewFactory.class);
    final PlatformView platformView = mock(PlatformView.class);
    final View androidView = mock(View.class);
    when(platformView.getView()).thenReturn(androidView);
    when(viewFactory.create(any(), eq(platformViewId), any())).thenReturn(platformView);

    platformViewsController.getRegistry().registerViewFactory("testType", viewFactory);

    final FlutterJNI jni = new FlutterJNI();
    jni.attachToNative(false);
    final FlutterView flutterView = attach(jni, platformViewsController);

    jni.onFirstFrame();

    // Simulate create call from the framework.
    final Map<String, Object> platformViewCreateArguments = new HashMap<>();
    platformViewCreateArguments.put("hybrid", true);
    platformViewCreateArguments.put("id", platformViewId);
    platformViewCreateArguments.put("viewType", "testType");
    platformViewCreateArguments.put("direction", 0);
    final MethodCall platformCreateMethodCall =
        new MethodCall("create", platformViewCreateArguments);
    jni.handlePlatformMessage("flutter/platform_views", encodeMethodCall(platformCreateMethodCall), /*replyId=*/ 0);

    // Produce a frame that displays a platform view and an overlay surface.
    platformViewsController.onBeginFrame();
    platformViewsController.onDisplayPlatformView(
        platformViewId,
        /* x=*/ 0,
        /* y=*/ 0,
        /* width=*/ 10,
        /* height=*/ 10,
        /* viewWidth=*/ 10,
        /* viewHeight=*/ 10,
        /* mutatorsStack=*/ new FlutterMutatorsStack());

    assertEquals(flutterView.getChildCount(), 7);
  }
}
