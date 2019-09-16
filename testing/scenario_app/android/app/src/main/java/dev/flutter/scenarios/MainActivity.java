package dev.flutter.scenarios;

import android.Manifest;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Log;

import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import io.flutter.app.FlutterPluginRegistry;
import io.flutter.embedding.android.FlutterFragment;
import io.flutter.embedding.android.FlutterView;
import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.FlutterShellArgs;
import io.flutter.plugin.common.BasicMessageChannel;
import io.flutter.plugin.common.BinaryCodec;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.PluginRegistry;
import io.flutter.embedding.android.FlutterActivity;
import io.flutter.plugin.common.StringCodec;
import io.flutter.plugins.GeneratedPluginRegistrant;
import io.flutter.view.TextureRegistry;

public class MainActivity extends FlutterActivity {
  final static String TAG = "Scenarios";

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    final Intent launchIntent = getIntent();
    if ("com.google.intent.action.TEST_LOOP".equals(launchIntent.getAction())) {
      if (Build.VERSION.SDK_INT > 22) {
        requestPermissions(new String[] {Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
      }
      // Run for one minute, get the timeline data, write it, and finish.
      final Uri logFileUri = launchIntent.getData();
      new Handler().postDelayed(() -> writeTimelineData(logFileUri), 20000);
    }
    else if("com.google.intent.action.TEST_EXTERNAL_TEXTURE".equals(launchIntent.getAction())){
      startExternalTexture();
    }
    else if("com.google.intent.action.TEST_PLATFORM_VIEW".equals(launchIntent.getAction())){
      new Handler().postDelayed(() -> startPlatformView(), 20000);
    }
  }

  private void writeTimelineData(Uri logFile) {
    if (logFile == null) {
      throw new IllegalArgumentException();
    }
    final BasicMessageChannel<ByteBuffer> channel = new BasicMessageChannel<ByteBuffer>(
            getFlutterEngine().getDartExecutor(), "write_timeline", BinaryCodec.INSTANCE);
    channel.send(null, (ByteBuffer reply) -> {
      try {
        final FileDescriptor fd = getContentResolver()
                                          .openAssetFileDescriptor(logFile, "w")
                                          .getFileDescriptor();
        final FileOutputStream outputStream = new FileOutputStream(fd);
        outputStream.write(reply.array());
        outputStream.close();
      } catch (IOException ex) {
        Log.e(TAG, "Could not write timeline file: " + ex.toString());
      }
      finish();
    });
  }


  private void startPlatformView(){
    getFlutterEngine().getPlatformViewsController().getRegistry()
            .registerViewFactory(
                    "scenarios/textPlatformView",
                    new TextPlatformViewFactory());
  }

  private void startExternalTexture(){

    getFlutterEngine().getDartExecutor().setMessageHandler("scenario_status", (byteBuffer, binaryReply) -> {
      final BasicMessageChannel<String> channel = new BasicMessageChannel<>(
              getFlutterEngine().getDartExecutor(), "set_scenario", StringCodec.INSTANCE);
      channel.send("external_texture", null);
    });

    getFlutterEngine().getDartExecutor().setMessageHandler("create_external_texture", (byteBuffer, binaryReply) -> {

      TestExternalTexture testExternalTexture = new TestExternalTexture(getFlutterEngine());

      testExternalTexture.createTexture((int textureId)->{
        TextureRegistry.ShareTextureEntry entry = getFlutterEngine().getRenderer().createShareTexture(textureId);
        long textureIndex = entry.id();
        final BasicMessageChannel<String> channel = new BasicMessageChannel<>(
                getFlutterEngine().getDartExecutor(), "update_data", StringCodec.INSTANCE);
        channel.send(""+textureIndex, null);

        testExternalTexture.startWithId(textureIndex,getApplicationContext());
      });
    });
  }
}
