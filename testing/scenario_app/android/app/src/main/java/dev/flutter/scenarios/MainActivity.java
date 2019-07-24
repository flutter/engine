package dev.flutter.scenarios;

import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;

import io.flutter.app.FlutterActivity;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.StringCodec;

public class MainActivity extends FlutterActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            getFlutterView().addFirstFrameListener(() -> reportFullyDrawn());
        }
        final Intent launchIntent = getIntent();
         if (launchIntent.getAction().equals("com.google.intent.action.TEST_LOOP")) {
         // Run for one minute, get the timeline data, write it, and finish.
         final Uri logFileUri = launchIntent.getData();
            new Handler().postDelayed(() -> writeTimelineData(logFileUri.getPath()), 60000);
         }
        super.onCreate(savedInstanceState);
    }

    private void writeTimelineData(String path) {
        final BasicMessageChannel channel = new BasicMessageChannel<>(
                getFlutterView(), "write_timeline", StringCodec.INSTANCE);
        channel.send(path, (Object reply) -> finish());
    }
}
