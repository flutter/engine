package com.example.android_test_adapter;
import com.example.android_host_app.MainActivity;
import androidx.test.rule.ActivityTestRule;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.MethodChannel.MethodCallHandler;
import io.flutter.plugin.common.MethodChannel.Result;
import io.flutter.view.FlutterView;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
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
    Description d = Description.createTestDescription(MainActivity.class, "example");
    notifier.fireTestStarted(d);
    try {
      Integer result = testResult.get();
      if (!result.equals(42)) {
        notifier.fireTestFailure(new Failure(d, new Exception("failure of test")));
      }
    } catch (ExecutionException e) {
        e.printStackTrace();
    } catch (InterruptedException e) {
        e.printStackTrace();
    }
    notifier.fireTestFinished(d);
  }
}
