package io.flutter.embedding.engine.systemchannels;

import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.content.res.AssetManager;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.MethodChannel;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(AndroidJUnit4.class)
public class MethodChannelTest {
  @Test
  public void methodChannel_resizeChannelBuffer() {
    FlutterJNI mockFlutterJNI = mock(FlutterJNI.class);
    DartExecutor dartExecutor = new DartExecutor(mockFlutterJNI, mock(AssetManager.class));
    MethodChannel rawChannel = new MethodChannel(dartExecutor, "flutter/test");

    BinaryMessenger messenger = dartExecutor.getBinaryMessenger();
    rawChannel.resizeChannelBuffer(3);

    ByteBuffer packet = null;
    try {
      final String content = String.format("resize\r%s\r%d", "flutter/test", 3);
      final byte[] bytes = content.getBytes("UTF-8");
      packet = ByteBuffer.allocateDirect(bytes.length);
      packet.put(bytes);
    } catch (UnsupportedEncodingException e) {
      throw new AssertionError("UTF-8 only");
    }

    // Verify that DartExecutor sent the correct message to FlutterJNI.
    verify(mockFlutterJNI, times(1))
        .dispatchPlatformMessage(
            eq(BasicMessageChannel.CHANNEL_BUFFERS_CHANNEL), eq(packet), anyInt(), anyInt());
  }
}
