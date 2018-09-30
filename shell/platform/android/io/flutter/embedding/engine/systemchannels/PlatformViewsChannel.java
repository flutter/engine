package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.View;

import java.util.List;
import java.util.Map;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.StandardMethodCodec;

public class PlatformViewsChannel {
  private static void validateDirection(int direction) throws IllegalArgumentException {
    if (direction != View.LAYOUT_DIRECTION_LTR && direction != View.LAYOUT_DIRECTION_RTL) {
      throw new IllegalArgumentException(String.format(
          "%d is not a valid View layout direction. Should either be LTR (%d) or RTL (%d).",
          direction,
          View.LAYOUT_DIRECTION_LTR,
          View.LAYOUT_DIRECTION_RTL
      ));
    }
  }

  public final MethodChannel channel;
  private PlatformViewsMethodHandler platformViewsMethodHandler;

  private final MethodChannel.MethodCallHandler parsingMethodHandler = new MethodChannel.MethodCallHandler() {
    @Override
    public void onMethodCall(MethodCall call, final MethodChannel.Result result) {
      if (platformViewsMethodHandler == null) {
        // If no explicit PlatformViewsMethodHandler has been registered then we don't
        // need to forward this call to an API. Return.
        return;
      }

      switch (call.method) {
        case "create":
          try {
            // The following message parsing might throw an IllegalArgumentException.
            final PlatformViewConfiguration config = PlatformViewConfiguration.fromMessage((Map<String, Object>) call.arguments());

            // The following call can throw any Exception due to any mismatch between the incoming
            // message and the current state of platform views on the platform side.
            long platformViewTexture = platformViewsMethodHandler.createPlatformView(config);

            // Send the new platform view's texture ID back to Flutter.
            result.success(platformViewTexture);
          } catch (Exception exception) {
            result.error(
                "error",
                exception.getMessage(),
                null
            );
          }

          return;
        case "dispose":
          try {
            final int id = call.arguments();
            platformViewsMethodHandler.disposePlatformView(id);
            result.success(null);
          } catch (Exception exception) {
            result.error(
                "error",
                exception.getMessage(),
                null
            );
          }

          return;
        case "resize":
          try {
            final PlatformViewResizeConfiguration config = PlatformViewResizeConfiguration.fromMessage((Map<String, Object>) call.arguments());
            platformViewsMethodHandler.resizePlatformView(config, new SuccessCallback() {
              @Override
              public void onSuccess() {
                result.success(null);
              }

              @Override
              public void onFailure() {
                result.error("error", "Failed to resize view: " + config.id, null);
              }
            });
          } catch (Exception exception) {
            result.error(
                "error",
                exception.getMessage(),
                null
            );
          }

          return;
        case "touch":
          try {
            final TouchInfo touchInfo = TouchInfo.fromMessage((List<Object>) call.arguments());
            platformViewsMethodHandler.onTouch(touchInfo);
          } catch (Exception exception) {
            result.error(
                "error",
                exception.getMessage(),
                null
            );
          }

          return;
        case "setDirection":
          try {
            Map<String, Object> args = call.arguments();
            int id = (int) args.get("id");
            int direction = (int) args.get("direction");
            validateDirection(direction);

            platformViewsMethodHandler.setDirection(id, direction);
          } catch (Exception exception) {
            result.error(
                "error",
                exception.getMessage(),
                null
            );
          }

          return;
      }
      result.notImplemented();
    }
  };

  public PlatformViewsChannel(@NonNull DartExecutor dartExecutor) {
    channel = new MethodChannel(dartExecutor, "flutter/platform_views", StandardMethodCodec.INSTANCE);
    channel.setMethodCallHandler(parsingMethodHandler);
  }

  // TODO(mattcarroll): javadoc
  public void setPlatformViewsMethodHandler(@Nullable PlatformViewsMethodHandler platformViewsMethodHandler) {
    this.platformViewsMethodHandler = platformViewsMethodHandler;
  }

  // TODO(mattcarroll): javadoc
  public void overrideDefaultMessageHandler(@NonNull MethodChannel.MethodCallHandler methodCallHandler) {
    channel.setMethodCallHandler(methodCallHandler);
  }

  // TODO(mattcarroll): javadoc
  public void restoreDefaultMessageHandler() {
    channel.setMethodCallHandler(parsingMethodHandler);
  }

  public interface PlatformViewsMethodHandler {
    /**
     *
     * @param config desired platform view's configuration
     * @return ID of the new platform view's texture
     */
    long createPlatformView(@NonNull PlatformViewConfiguration config);

    void resizePlatformView(@NonNull PlatformViewResizeConfiguration config, @NonNull SuccessCallback callback);

    void onTouch(@NonNull TouchInfo touchInfo);

    void setDirection(int platformViewId, int direction);

    void disposePlatformView(int platformViewId);
  }

  public interface SuccessCallback {
    void onSuccess();
    void onFailure();
  }

  public static class PlatformViewConfiguration {
    public static PlatformViewConfiguration fromMessage(Map<String, Object> args) throws IllegalArgumentException {
      validateDirection((int) args.get("direction"));

      return new PlatformViewConfiguration(
          (int) args.get("id"),
          (String) args.get("viewType"),
          (double) args.get("width"),
          (double) args.get("height"),
          (int) args.get("direction"),
          (byte[]) args.get("params")
      );
    }

    public final int id;
    public final String viewType;
    public final double logicalWidth;
    public final double logicalHeight;
    public final int direction;
    public final byte[] params;

    public PlatformViewConfiguration(
        int id,
        @NonNull String viewType,
        double logicalWidth,
        double logicalHeight,
        int direction,
        @Nullable byte[] params
    ) {
      this.id = id;
      this.viewType = viewType;
      this.logicalWidth = logicalWidth;
      this.logicalHeight = logicalHeight;
      this.direction = direction;
      this.params = params;
    }
  }

  public static class PlatformViewResizeConfiguration {
    public static PlatformViewResizeConfiguration fromMessage(Map<String, Object> args) {
      return new PlatformViewResizeConfiguration(
        (int) args.get("id"),
        (double) args.get("width"),
        (double) args.get("height")
      );
    }

    public final int id;
    public final double width;
    public final double height;

    public PlatformViewResizeConfiguration(int id, double width, double height) {
      this.id = id;
      this.width = width;
      this.height = height;
    }
  }

  public static class TouchInfo {
    public static TouchInfo fromMessage(List<Object> args) {
      return new TouchInfo(
        (int) args.get(0),
        (Number) args.get(1),
        (Number) args.get(2),
        (int) args.get(3),
        (int) args.get(4),
        (List<Object>) args.get(5), //).toArray(new MotionEvent.PointerProperties[pointerCount]),
        (List<Object>) args.get(6), //, density).toArray(new MotionEvent.PointerCoords[pointerCount]),
        (int) args.get(7),
        (int) args.get(8),
        (float) (double) args.get(9),
        (float) (double) args.get(10),
        (int) args.get(11),
        (int) args.get(12),
        (int) args.get(13),
        (int) args.get(14)
      );
    }

    public final int id;
    public final Number downTime;
    public final Number eventTime;
    public final int action;
    public final int pointerCount;
    public final List<Object> rawPointerProperties;
    public final List<Object> rawPointerCoords;
    public final int metaState;
    public final int buttonState;
    public final float xPrecision;
    public final float yPrecision;
    public final int deviceId;
    public final int edgeFlags;
    public final int source;
    public final int flags;

    public TouchInfo(
        int id,
        Number downTime,
        Number eventTime,
        int action,
        int pointerCount,
        List<Object> rawPointerProperties,
        List<Object> rawPointerCoords,
        int metaState,
        int buttonState,
        float xPrecision,
        float yPrecision,
        int deviceId,
        int edgeFlags,
        int source,
        int flags
    ) {
      this.id = id;
      this.downTime = downTime;
      this.eventTime = eventTime;
      this.action = action;
      this.pointerCount = pointerCount;
      this.rawPointerProperties = rawPointerProperties;
      this.rawPointerCoords = rawPointerCoords;
      this.metaState = metaState;
      this.buttonState = buttonState;
      this.xPrecision = xPrecision;
      this.yPrecision = yPrecision;
      this.deviceId = deviceId;
      this.edgeFlags = edgeFlags;
      this.source = source;
      this.flags = flags;
    }
  }
}
