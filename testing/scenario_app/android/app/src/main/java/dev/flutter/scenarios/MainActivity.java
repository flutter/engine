package dev.flutter.scenarios;

import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.NonNull;

import io.flutter.Log;
import io.flutter.embedding.android.FlutterActivity;
import io.flutter.embedding.android.FlutterFragment;
import io.flutter.embedding.android.FlutterView;
import io.flutter.embedding.engine.FlutterShellArgs;
import io.flutter.embedding.engine.renderer.OnFirstFrameRenderedListener;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.StringCodec;

public class MainActivity extends FlutterActivity implements OnFirstFrameRenderedListener {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final Intent launchIntent = getIntent();
        if ("com.google.intent.action.TEST_LOOP".equals(launchIntent.getAction())) {
            // Run for one minute, get the timeline data, write it, and finish.
            final Uri logFileUri = launchIntent.getData();
            new Handler().postDelayed(() -> writeTimelineData(logFileUri), 10000);
        }
    }

    public void onFirstFrameRendered() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            reportFullyDrawn();
        }
    }

    private FlutterShellArgs getFlutterShellArgs() {
        FlutterShellArgs args = FlutterShellArgs.fromIntent(getIntent());
        args.add(FlutterShellArgs.ARG_TRACE_STARTUP);
        args.add(FlutterShellArgs.ARG_ENABLE_DART_PROFILING);
        args.add(FlutterShellArgs.ARG_VERBOSE_LOGGING);

        return args;
    }

    @Override
    @NonNull
    protected FlutterFragment createFlutterFragment() {
        return new FlutterFragment.Builder()
                .dartEntrypoint(getDartEntrypoint())
                .initialRoute(getInitialRoute())
                .appBundlePath(getAppBundlePath())
                .flutterShellArgs(getFlutterShellArgs())
                .renderMode(FlutterView.RenderMode.surface)
                .transparencyMode(FlutterView.TransparencyMode.opaque)
                .shouldAttachEngineToActivity(true)
                .build();
    }

    private void writeTimelineData(Uri uri) {
        if (uri == null) {
            throw new IllegalArgumentException();
        }
        if (getFlutterEngine() == null) {
            Log.e("Scenarios", "Could not write timeline data - no engine.");
            return;
        }
        final BasicMessageChannel<String> channel = new BasicMessageChannel<>(
                getFlutterEngine().getDartExecutor(), "write_timeline", StringCodec.INSTANCE);
        channel.send(uri.getPath(), (String reply) -> finish());
    }
}
