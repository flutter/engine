package io.flutter.plugin.platform;

import android.app.Activity;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import io.flutter.embedding.engine.systemchannels.PlatformChannel;

import static org.mockito.Mockito.mock;

@Config(manifest=Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class PlatformPluginTest {
  @Config(sdk = 20)
  @Test
  public void itIgnoresNewHapticEventsOnOldAndroidPlatforms() {
    Activity fakeActivity = mock(Activity.class);
    PlatformChannel fakePlatformChannel = mock(PlatformChannel.class);
    PlatformPlugin platformPlugin = new PlatformPlugin(fakeActivity, fakePlatformChannel);

    // HEAVY_IMPACT haptic response is only available on "M" (23) and later.
    platformPlugin.vibrateHapticFeedback(PlatformChannel.HapticFeedbackType.HEAVY_IMPACT);

    // SELECTION_CLICK haptic response is only available on "LOLLIPOP" (21) and later.
    platformPlugin.vibrateHapticFeedback(PlatformChannel.HapticFeedbackType.SELECTION_CLICK);
  }
}
