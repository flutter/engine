package test.io.flutter.embedding.engine;

import static junit.framework.TestCase.assertEquals;
import static org.mockito.Matchers.any;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.when;

import androidx.annotation.NonNull;
import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.embedding.engine.loader.FlutterApplicationInfo;
import io.flutter.embedding.engine.loader.FlutterLoader;
import io.flutter.embedding.engine.plugins.FlutterPlugin;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class PluginComponentTest {
  @Test
  public void pluginsCanAccessFlutterAssetPaths() {
    // Setup test.
    FlutterJNI flutterJNI = mock(FlutterJNI.class);
    when(flutterJNI.isAttached()).thenReturn(true);
    FlutterApplicationInfo emptyInfo =
        new FlutterApplicationInfo(null, null, null, null, null, null, false, false);

    FlutterLoader realFlutterLoader = new FlutterLoader();
    FlutterLoader spyFlutterLoader = spy(realFlutterLoader);

    // Let the real startInitialization be called, but mock the rest so it doesn't try to load
    // the real native library.
    doNothing().when(spyFlutterLoader).loadNativeLibrary();
    doNothing().when(spyFlutterLoader).ensureInitializationComplete(any(), any());

    // Execute behavior under test.
    FlutterEngine flutterEngine =
        new FlutterEngine(RuntimeEnvironment.application, spyFlutterLoader, flutterJNI);

    // As soon as our plugin is registered it will look up asset paths and store them
    // for our verification.
    PluginThatAccessesAssets plugin = new PluginThatAccessesAssets();
    flutterEngine.getPlugins().add(plugin);

    // Verify results.
    assertEquals("flutter_assets/fake_asset.jpg", plugin.getAssetPathBasedOnName());
    assertEquals(
        "flutter_assets/packages/fakepackage/fake_asset.jpg",
        plugin.getAssetPathBasedOnNameAndPackage());
    assertEquals("flutter_assets/some/path/fake_asset.jpg", plugin.getAssetPathBasedOnSubpath());
    assertEquals(
        "flutter_assets/packages/fakepackage/some/path/fake_asset.jpg",
        plugin.getAssetPathBasedOnSubpathAndPackage());
  }

  private static class PluginThatAccessesAssets implements FlutterPlugin {
    private String assetPathBasedOnName;
    private String assetPathBasedOnNameAndPackage;
    private String assetPathBasedOnSubpath;
    private String assetPathBasedOnSubpathAndPackage;

    public String getAssetPathBasedOnName() {
      return assetPathBasedOnName;
    }

    public String getAssetPathBasedOnNameAndPackage() {
      return assetPathBasedOnNameAndPackage;
    }

    public String getAssetPathBasedOnSubpath() {
      return assetPathBasedOnSubpath;
    }

    public String getAssetPathBasedOnSubpathAndPackage() {
      return assetPathBasedOnSubpathAndPackage;
    }

    @Override
    public void onAttachedToEngine(@NonNull FlutterPluginBinding binding) {
      assetPathBasedOnName = binding.getFlutterAssets().getAssetFilePathByName("fake_asset.jpg");

      assetPathBasedOnNameAndPackage =
          binding.getFlutterAssets().getAssetFilePathByName("fake_asset.jpg", "fakepackage");

      assetPathBasedOnSubpath =
          binding.getFlutterAssets().getAssetFilePathByName("some/path/fake_asset.jpg");

      assetPathBasedOnSubpathAndPackage =
          binding
              .getFlutterAssets()
              .getAssetFilePathByName("some/path/fake_asset.jpg", "fakepackage");
    }

    @Override
    public void onDetachedFromEngine(@NonNull FlutterPluginBinding binding) {}
  }
}
