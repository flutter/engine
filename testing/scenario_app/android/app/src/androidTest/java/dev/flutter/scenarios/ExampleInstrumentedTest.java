package dev.flutter.scenarios;

import android.content.Context;
import android.content.Intent;
import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.*;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class ExampleInstrumentedTest {
    @Test
    public void useAppContext() {
        // Context of the app under test.
        Context appContext = InstrumentationRegistry.getTargetContext();

        assertEquals("dev.flutter.scenarios", appContext.getPackageName());
    }

    @Test
    public void testExternalTexture(){

        Context appContext = InstrumentationRegistry.getTargetContext();
        Intent intent = new Intent(appContext,MainActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setAction("com.google.intent.action.TEST_EXTERNAL_TEXTURE");
        appContext.startActivity(intent);
    }
}
