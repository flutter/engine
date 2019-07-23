package dev.flutter.scenarios;

import android.os.Bundle;

import io.flutter.app.FlutterActivity;
import io.flutter.embedding.engine.renderer.OnFirstFrameRenderedListener;

public class MainActivity extends FlutterActivity implements OnFirstFrameRenderedListener {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onFirstFrameRendered() {
        reportFullyDrawn();
    }
}
