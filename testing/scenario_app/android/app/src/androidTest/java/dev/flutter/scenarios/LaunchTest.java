package dev.flutter.scenarios;

import android.content.Context;
import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;

import org.junit.runner.Description;
import org.junit.runner.RunWith;
import org.junit.runner.Runner;
import org.junit.runner.notification.Failure;
import org.junit.runner.notification.RunNotifier;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.dart.DartExecutor;

@RunWith(AndroidJUnit4.class)
public class LaunchTest extends Runner {
    @Override
    public Description getDescription() {
        return Description.createTestDescription(BlankActivity.class, "can launch engine");
    }

    @Override
    public void run(RunNotifier notifier) {
        notifier.fireTestStarted(getDescription());
        Context applicationContext = InstrumentationRegistry.getTargetContext();
        // Specifically, create the engine without running FlutterMain first.
        FlutterEngine engine = new FlutterEngine(applicationContext);
        CompletableFuture<Boolean> statusReceived = new CompletableFuture<>();

        engine.getDartExecutor().setMessageHandler(
            "scenario_status",
            (byteBuffer, binaryReply) -> statusReceived.complete(Boolean.TRUE)
        );

        engine.getDartExecutor().executeDartEntrypoint(DartExecutor.DartEntrypoint.createDefault());

        try {
            Boolean result = statusReceived.get(10, TimeUnit.SECONDS);
            if (!result) {
                notifier.fireTestFailure(
                        new Failure(getDescription(),
                        new Exception("expected message on scenario_status not received")));
            }
        } catch (ExecutionException e) {
            notifier.fireTestFailure(new Failure(getDescription(), e));
        } catch (InterruptedException e) {
            notifier.fireTestFailure(new Failure(getDescription(), e));
        } catch (TimeoutException e) {
            notifier.fireTestFailure(
                    new Failure(getDescription(),
                    new Exception("timed out waiting for engine started signal")));
        }
    }
}
