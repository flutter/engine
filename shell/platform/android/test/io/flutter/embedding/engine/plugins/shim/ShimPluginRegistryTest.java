package io.flutter.embedding.engine.plugins.shim;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.Activity;
import android.content.Context;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.plugins.FlutterPlugin;
import io.flutter.embedding.engine.plugins.FlutterPlugin.FlutterPluginBinding;
import io.flutter.embedding.engine.plugins.PluginRegistry;
import io.flutter.embedding.engine.plugins.activity.ActivityAware;
import io.flutter.embedding.engine.plugins.activity.ActivityPluginBinding;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(AndroidJUnit4.class)
public class ShimPluginRegistryTest {

  @Mock private FlutterEngine mockFlutterEngine;
  @Mock private FlutterPluginBinding mockFlutterPluginBinding;
  @Mock private ActivityPluginBinding mockActivityPluginBinding;
  @Mock private PluginRegistry mockPluginRegistry;
  @Mock private Context mockApplicationContext;
  @Mock private Activity mockActivity;

  @Before
  public void setup() {
    MockitoAnnotations.openMocks(this);
    when(mockFlutterEngine.getPlugins()).thenReturn(mockPluginRegistry);
    when(mockFlutterPluginBinding.getApplicationContext()).thenReturn(mockApplicationContext);
    when(mockActivityPluginBinding.getActivity()).thenReturn(mockActivity);
  }

  // todo write some tests :)
}
