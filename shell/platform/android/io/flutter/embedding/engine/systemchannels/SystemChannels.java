package io.flutter.embedding.engine.systemchannels;

import io.flutter.embedding.engine.dart.DartExecutor;

/**
 * Official platform channels that are responsible for fundamental Flutter/Platform interactions.
 */
public class SystemChannels {

  public final AccessibilityChannel accessibility;
  public final KeyEventChannel keyEvent;
  public final LifecycleChannel lifecycle;
  public final LocalizationChannel localization;
  public final NavigationChannel navigation;
  public final PlatformChannel platform;
  public final PlatformViewsChannel platformViews;
  public final SettingsChannel settings;
  public final SystemChannel system;
  public final TextInputChannel textInput;

  public SystemChannels(final DartExecutor dartExecutor) {
    accessibility = new AccessibilityChannel(dartExecutor);
    keyEvent = new KeyEventChannel(dartExecutor);
    lifecycle = new LifecycleChannel(dartExecutor);
    localization = new LocalizationChannel(dartExecutor);
    navigation = new NavigationChannel(dartExecutor);
    platform = new PlatformChannel(dartExecutor);
    platformViews = new PlatformViewsChannel(dartExecutor);
    settings = new SettingsChannel(dartExecutor);
    system = new SystemChannel(dartExecutor);
    textInput = new TextInputChannel(dartExecutor);
  }

}
