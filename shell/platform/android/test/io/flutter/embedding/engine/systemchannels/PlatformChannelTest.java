package io.flutter.embedding.engine.systemchannels;

import android.util.Log;

import org.hamcrest.Description;
import org.json.JSONArray;
import org.json.JSONObject;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatcher;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.nio.ByteBuffer;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.systemchannels.PlatformChannel;
import io.flutter.embedding.engine.systemchannels.PlatformChannel.PlatformMessageHandler;
import io.flutter.plugin.common.MethodChannel.Result;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodCall;

import static org.mockito.Matchers.argThat;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

@Config(manifest=Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class PlatformChannelTest {
    // @Test
    // public void itSendsSuccessMessageToFramework() {
    //     DartExecutor dartExecutor = mock(DartExecutor.class);
    //     ResultsMock resultsMock = mock(ResultsMock.class);

    //     PlatformChannel platformChannel = new PlatformChannel(dartExecutor);

    //     // invoke method with correct arguments
    //     JSONArray inputRects = new JSONArray();
    //     platformChannel.channel.invokeMethod("SystemGestures.setSystemGestureExclusionRects", inputRects, resultsMock);

    //     // verify(dartExecutor, times(1)).send(
    //     //     eq("flutter/platform"),
    //     //     ByteBufferMatcher.eqByteBuffer(JSONMethodCodec.INSTANCE.encodeMethodCall(
    //     //         new MethodCall(
    //     //             "SystemGestures.setSystemGestureExclusionRects",
    //     //             inputRects
    //     //         )
    //     //     )),

    //     //     // TODO (create incoming result callback handler -- see MethodChannel.java)
    //     // );
    // }

    @Test
    public void itRequiresJSONArrayInput() {
        DartExecutor dartExecutor = mock(DartExecutor.class);
        ResultsMock resultsMock = mock(ResultsMock.class);
        PlatformMessageHandler platformMessageHandler = mock(PlatformMessageHandler.class);

        PlatformChannel platformChannel = new PlatformChannel(dartExecutor);
        platformChannel.setPlatformMessageHandler(platformMessageHandler);

        // invoke method with incorrect shape
        JSONObject inputRects = new JSONObject();
        platformChannel.channel.invokeMethod("SystemGestures.setSystemGestureExclusionRects", inputRects, resultsMock);

        Log.v("test", "got to this point");

        // verify(dartExecutor, times(1)).send(
        //     eq("flutter/platform"),
        //     ByteBufferMatcher.eqByteBuffer(JSONMethodCodec.INSTANCE.encodeMethodCall(
        //         new MethodCall(
        //             "SystemGestures.setSystemGestureExclusionRects",
        //             inputRects
        //         )
        //     )),
        //     eq(null)
        // );

        // verify(platformChannel.channel, times(1)).send();
        // String inputTypeError = "Input type is incorrect. Ensure that a List<Map<String, int>> is passed as the input for SystemGestureExclusionRects.setSystemGestureExclusionRects.";
        // verify(resultsMock, times(1)).error(
        //     "inputTypeError",
        //     inputTypeError,
        //     null
        // );
    }

    // @Test
    // public void itSendsJSONExceptionOnIncorrectDataShape() {

    // }

    /**
     * Mockito matcher that compares two {@link ByteBuffer}s by resetting both buffers and then
     * utilizing their standard {@code equals()} method.
     * <p>
     * This matcher will change the state of the expected and actual buffers. The exact change in
     * state depends on where the comparison fails or succeeds.
     */
    static class ByteBufferMatcher extends ArgumentMatcher<ByteBuffer> {
        static ByteBuffer eqByteBuffer(ByteBuffer expected) {
            return argThat(new ByteBufferMatcher(expected));
        }

        private ByteBuffer expected;

        ByteBufferMatcher(ByteBuffer expected) {
            this.expected = expected;
        }

        @Override
        public boolean matches(Object argument) {
            if (!(argument instanceof ByteBuffer)) {
            return false;
            }

            // Reset the buffers for content comparison.
            ((ByteBuffer) argument).position(0);
            expected.position(0);

            return expected.equals(argument);
        }

        // Implemented so that during a failure the expected value is
        // shown in logs, rather than the name of this class.
        @Override
        public void describeTo(Description description) {
            description.appendText(expected.toString());
        }
    }
}

class ResultsMock implements Result {
    @Override
    public void success(Object result) {}

    @Override
    public void error(String errorCode, String errorMessage, Object errorDetails) {}

    @Override
    public void notImplemented() {}
}