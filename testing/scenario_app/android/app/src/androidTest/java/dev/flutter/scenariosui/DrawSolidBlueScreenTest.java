package dev.flutter.scenariosui;

import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.test.filters.LargeTest;
import androidx.test.rule.ActivityTestRule;
import androidx.test.runner.AndroidJUnit4;
import dev.flutter.scenarios.PlatformViewsActivity;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
@LargeTest
public final class DrawSolidBlueScreenTest {
  @Rule @NonNull
  public ActivityTestRule<PlatformViewsActivity> activityRule =
      new ActivityTestRule<>(
          PlatformViewsActivity.class, /*initialTouchMode=*/ false, /*launchActivity=*/ false);

  @Rule @NonNull public ArgumentAwareIntent intentRule = new ArgumentAwareIntent();

  @Test
  public void test() throws Exception {
    Intent intent = intentRule.getIntent();
    intent.putExtra("scenario_name", "solid_blue");
    ScreenshotUtil.capture(activityRule.launchActivity(intent), "DrawSolidBlueScreenTest");
  }
}
