package io.flutter.embedding.engine;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.LocaleList;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.renderer.FlutterUiDisplayListener;
import io.flutter.embedding.engine.systemchannels.LocalizationChannel;
import io.flutter.plugin.localization.LocalizationPlugin;
import java.util.Locale;
import java.util.concurrent.atomic.AtomicInteger;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
@TargetApi(24) // LocaleList and scriptCode are API 24+.
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
  public void computePlatformResolvedLocaleCallsLocalizationPluginProperly() {
    // --- Test Setup ---
    FlutterJNI flutterJNI = new FlutterJNI();

    Context context = mock(Context.class);
    Resources resources = mock(Resources.class);
    Configuration config = mock(Configuration.class);
    DartExecutor dartExecutor = mock(DartExecutor.class);
    LocaleList localeList =
        new LocaleList(new Locale("es", "MX"), new Locale("zh", "CN"), new Locale("en", "US"));
    when(context.getResources()).thenReturn(resources);
    when(resources.getConfiguration()).thenReturn(config);
    when(config.getLocales()).thenReturn(localeList);

    flutterJNI.setLocalizationPlugin(
        new LocalizationPlugin(context, new LocalizationChannel(dartExecutor)));
    String[] supportedLocales =
        new String[] {
          "fr", "FR", "",
          "zh", "", "",
          "en", "CA", ""
        };
    String[] result = flutterJNI.computePlatformResolvedLocale(supportedLocales);
    assertEquals(result.length, 3);
    assertEquals(result[0], "zh");
    assertEquals(result[1], "");
    assertEquals(result[2], "");

    supportedLocales =
        new String[] {
          "fr", "FR", "",
          "ar", "", "",
          "en", "CA", ""
        };
    result = flutterJNI.computePlatformResolvedLocale(supportedLocales);
    assertEquals(result.length, 0); // This should change when full algo is implemented.

    supportedLocales =
        new String[] {
          "fr", "FR", "",
          "ar", "", "",
          "en", "US", ""
        };
    result = flutterJNI.computePlatformResolvedLocale(supportedLocales);
    assertEquals(result.length, 3);
    assertEquals(result[0], "en");
    assertEquals(result[1], "US");
    assertEquals(result[2], "");

    supportedLocales =
        new String[] {
          "ar", "", "",
          "es", "MX", "",
          "en", "US", ""
        };
    result = flutterJNI.computePlatformResolvedLocale(supportedLocales);
    assertEquals(result.length, 3);
    assertEquals(result[0], "es");
    assertEquals(result[1], "MX");
    assertEquals(result[2], "");
  }
}
