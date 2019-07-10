// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.example.android_host_app;
import com.example.android_host_app.MainActivity;
import androidx.test.rule.ActivityTestRule;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.MethodChannel.MethodCallHandler;
import io.flutter.plugin.common.MethodChannel.Result;
import io.flutter.view.FlutterView;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import org.junit.runner.Description;
import org.junit.runner.Runner;
import org.junit.runner.notification.Failure;
import org.junit.runner.notification.RunNotifier;

public class FlutterJUnitRunner extends Runner {

  private static final String CHANNEL = "dev.flutter.engine/test";

  CompletableFuture<Integer> testResult = new CompletableFuture<>();

  public FlutterJUnitRunner(Class<?> klass) {
      ActivityTestRule<MainActivity> rule = new ActivityTestRule<>(MainActivity.class);
      MainActivity activity = rule.launchActivity(null);
      FlutterView view = activity.getFlutterView();
      MethodChannel channel = new MethodChannel(view, CHANNEL);
      channel.setMethodCallHandler(
          new MethodCallHandler() {
              @Override
              public void onMethodCall(MethodCall call, Result result) {
                  if (call.method.equals("testMethodChannel")) {
                      testResult.complete(call.arguments());
                  } else {
                      result.notImplemented();
                  }
              }
          });
  }

  @Override
  public Description getDescription() { return Description.createTestDescription(MainActivity.class, "methodChannel");
  }

  @Override
  public void run(RunNotifier notifier) {
    Description description = Description.createTestDescription(MainActivity.class, "smoke test");
    notifier.fireTestStarted(description);
    try {
      Integer result = testResult.get();
      if (!result.equals(42)) {
        notifier.fireTestFailure(new Failure(description, new Exception("test failed")));
      }
    } catch (ExecutionException e) {
        notifier.fireTestFailure(new Failure(description, e));
    } catch (InterruptedException e) {
        notifier.fireTestFailure(new Failure(description, e));
    }
    notifier.fireTestFinished(description);
  }
}
