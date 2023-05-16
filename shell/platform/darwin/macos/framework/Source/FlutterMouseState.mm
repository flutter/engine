// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMouseState.h"

namespace {

// Use different device ID for mouse and pan/zoom events, since we can't differentiate the actual
// device (mouse v.s. trackpad).
static constexpr int32_t kMousePointerDeviceId = 0;
static constexpr int32_t kPointerPanZoomDeviceId = 1;

// A trackpad touch following inertial scrolling should cause an inertia cancel
// event to be issued. Use a window of 50 milliseconds after the scroll to account
// for delays in event propagation observed in macOS Ventura.
static constexpr double kTrackpadTouchInertiaCancelWindowMs = 0.050;
}

@interface FlutterMouseState ()

/**
 * Calls dispatchMouseEvent:phase: with a phase determined by self.mouseState.
 *
 * mouseState.buttons should be updated before calling this method.
 */
- (void)dispatchMouseEvent:(nonnull NSEvent*)event inView:(NSView*)flutterView;

/**
 * Calls dispatchMouseEvent:phase: with a phase determined by event.phase.
 */
- (void)dispatchGestureEvent:(nonnull NSEvent*)event inView:(NSView*)flutterView;

/**
 * Converts |event| to a FlutterPointerEvent with the given phase, and sends it to the engine.
 */
- (void)dispatchMouseEvent:(NSEvent*)event phase:(FlutterPointerPhase)phase inView:(NSView*)flutterView;

/**
  * Resets all gesture state to default values.
  */
- (void)gestureReset;

/**
  * Resets all state to default values.
  */
- (void)reset;

@end

@implementation FlutterMouseState {
  SendPointerEventCallback _engineDispatchPointerEvent;

  /**
   * The currently pressed buttons, as represented in FlutterPointerEvent.
   */
  int64_t buttons;

  /**
   * The accumulated gesture pan.
   */
  CGFloat delta_x;
  CGFloat delta_y;

  /**
   * The accumulated gesture zoom scale.
   */
  CGFloat scale;

  /**
   * The accumulated gesture rotation.
   */
  CGFloat rotation;

  /**
   * Whether or not a kAdd event has been sent (or sent again since the last kRemove if tracking is
   * enabled). Used to determine whether to send a kAdd event before sending an incoming mouse
   * event, since Flutter expects pointers to be added before events are sent for them.
   */
  bool flutter_state_is_added;

  /**
   * Whether or not a kDown has been sent since the last kAdd/kUp.
   */
  bool flutter_state_is_down;

  /**
   * Whether or not mouseExited: was received while a button was down. Cocoa's behavior when
   * dragging out of a tracked area is to send an exit, then keep sending drag events until the last
   * button is released. Flutter doesn't expect to receive events after a kRemove, so the kRemove
   * for the exit needs to be delayed until after the last mouse button is released. If cursor
   * returns back to the window while still dragging, the flag is cleared in mouseEntered:.
   */
  bool has_pending_exit;

  /**
   * Pan gesture is currently sending us events.
   */
  bool pan_gesture_active;

  /**
   * Scale gesture is currently sending us events.
   */
  bool scale_gesture_active;

  /**
   * Rotate gesture is currently sending use events.
   */
  bool rotate_gesture_active;

  /**
   * Time of last scroll momentum event.
   */
  NSTimeInterval last_scroll_momentum_changed_time;
}

- (instancetype)initWithCallback:(SendPointerEventCallback)callback {
  self = [super init];
  if (self) {
    _engineDispatchPointerEvent = callback;
    buttons = 0;
    delta_x = 0;
    delta_y = 0;
    scale = 0;
    rotation = 0;
    flutter_state_is_added = false;
    flutter_state_is_down = false;
    has_pending_exit = false;
    pan_gesture_active = false;
    scale_gesture_active = false;
    rotate_gesture_active = false;
    last_scroll_momentum_changed_time = 0;
  }
  return self;
}

- (void)mouseEntered:(NSEvent*)event inView:(NSView*)flutterView {
  if (has_pending_exit) {
    has_pending_exit = false;
  } else {
    [self dispatchMouseEvent:event phase:kAdd inView:flutterView];
  }
}

- (void)mouseExited:(NSEvent*)event inView:(NSView*)flutterView {
  if (buttons != 0) {
    has_pending_exit = true;
    return;
  }
  [self dispatchMouseEvent:event phase:kRemove inView:flutterView];
}

- (void)mouseDown:(NSEvent*)event inView:(NSView*)flutterView {
  buttons |= kFlutterPointerButtonMousePrimary;
  [self dispatchMouseEvent:event inView:flutterView];
}

- (void)mouseUp:(NSEvent*)event inView:(NSView*)flutterView {
  buttons &= ~static_cast<uint64_t>(kFlutterPointerButtonMousePrimary);
  [self dispatchMouseEvent:event inView:flutterView];
}

- (void)mouseDragged:(NSEvent*)event inView:(NSView*)flutterView {
  [self dispatchMouseEvent:event inView:flutterView];
}

- (void)rightMouseDown:(NSEvent*)event inView:(NSView*)flutterView {
  buttons |= kFlutterPointerButtonMouseSecondary;
  [self dispatchMouseEvent:event inView:flutterView];
}

- (void)rightMouseUp:(NSEvent*)event inView:(NSView*)flutterView {
  buttons &= ~static_cast<uint64_t>(kFlutterPointerButtonMouseSecondary);
  [self dispatchMouseEvent:event inView:flutterView];
}

- (void)rightMouseDragged:(NSEvent*)event inView:(NSView*)flutterView {
  [self dispatchMouseEvent:event inView:flutterView];
}

- (void)otherMouseDown:(NSEvent*)event inView:(NSView*)flutterView {
  buttons |= (1 << event.buttonNumber);
  [self dispatchMouseEvent:event inView:flutterView];
}

- (void)otherMouseUp:(NSEvent*)event inView:(NSView*)flutterView {
  buttons &= ~static_cast<uint64_t>(1 << event.buttonNumber);
  [self dispatchMouseEvent:event inView:flutterView];
}

- (void)otherMouseDragged:(NSEvent*)event inView:(NSView*)flutterView {
  [self dispatchMouseEvent:event inView:flutterView];
}

- (void)mouseMoved:(NSEvent*)event inView:(NSView*)flutterView {
  [self dispatchMouseEvent:event inView:flutterView];
}

- (void)scrollWheel:(NSEvent*)event inView:(NSView*)flutterView {
  [self dispatchGestureEvent:event inView:flutterView];
}

- (void)magnifyWithEvent:(NSEvent*)event inView:(NSView*)flutterView {
  [self dispatchGestureEvent:event inView:flutterView];
}

- (void)rotateWithEvent:(NSEvent*)event inView:(NSView*)flutterView {
  [self dispatchGestureEvent:event inView:flutterView];
}

- (void)touchesBeganWithEvent:(NSEvent*)event inView:(NSView*)flutterView {
  NSTouch* touch = event.allTouches.anyObject;
  if (touch != nil) {
    if ((event.timestamp - last_scroll_momentum_changed_time) <
        kTrackpadTouchInertiaCancelWindowMs) {
      // The trackpad has been touched following a scroll momentum event.
      // A scroll inertia cancel message should be sent to the framework.
      NSPoint locationInView = [flutterView convertPoint:event.locationInWindow fromView:nil];
      NSPoint locationInBackingCoordinates =
          [flutterView convertPointToBacking:locationInView];
      FlutterPointerEvent flutterEvent = {
          .struct_size = sizeof(flutterEvent),
          .timestamp = static_cast<size_t>(event.timestamp * USEC_PER_SEC),
          .x = locationInBackingCoordinates.x,
          .y = -locationInBackingCoordinates.y,  // convertPointToBacking makes this negative.
          .device = kPointerPanZoomDeviceId,
          .signal_kind = kFlutterPointerSignalKindScrollInertiaCancel,
          .device_kind = kFlutterPointerDeviceKindTrackpad,
      };

      _engineDispatchPointerEvent(flutterEvent);
      // Ensure no further scroll inertia cancel event will be sent.
      last_scroll_momentum_changed_time = 0;
    }
  }
}

- (void)dispatchMouseEvent:(nonnull NSEvent*)event inView:(NSView*)flutterView {
  FlutterPointerPhase phase = buttons == 0
                                  ? (flutter_state_is_down ? kUp : kHover)
                                  : (flutter_state_is_down ? kMove : kDown);
  [self dispatchMouseEvent:event phase:phase inView:flutterView];
}

- (void)dispatchGestureEvent:(nonnull NSEvent*)event inView:(NSView*)flutterView {
  if (event.phase == NSEventPhaseBegan || event.phase == NSEventPhaseMayBegin) {
    [self dispatchMouseEvent:event phase:kPanZoomStart inView:flutterView];
  } else if (event.phase == NSEventPhaseChanged) {
    [self dispatchMouseEvent:event phase:kPanZoomUpdate inView:flutterView];
  } else if (event.phase == NSEventPhaseEnded || event.phase == NSEventPhaseCancelled) {
    [self dispatchMouseEvent:event phase:kPanZoomEnd inView:flutterView];
  } else if (event.phase == NSEventPhaseNone && event.momentumPhase == NSEventPhaseNone) {
    [self dispatchMouseEvent:event phase:kHover inView:flutterView];
  } else {
    // Waiting until the first momentum change event is a workaround for an issue where
    // touchesBegan: is called unexpectedly while in low power mode within the interval between
    // momentum start and the first momentum change.
    if (event.momentumPhase == NSEventPhaseChanged) {
      last_scroll_momentum_changed_time = event.timestamp;
    }
    // Skip momentum update events, the framework will generate scroll momentum.
    NSAssert(event.momentumPhase != NSEventPhaseNone,
             @"Received gesture event with unexpected phase");
  }
}

- (void)dispatchMouseEvent:(NSEvent*)event phase:(FlutterPointerPhase)phase inView:(NSView*)flutterView {
  // There are edge cases where the system will deliver enter out of order relative to other
  // events (e.g., drag out and back in, release, then click; mouseDown: will be called before
  // mouseEntered:). Discard those events, since the add will already have been synthesized.
  if (flutter_state_is_added && phase == kAdd) {
    return;
  }

  // Multiple gesture recognizers could be active at once, we can't send multiple kPanZoomStart.
  // For example: rotation and magnification.
  if (phase == kPanZoomStart) {
    bool gestureAlreadyDown = pan_gesture_active || scale_gesture_active ||
                              rotate_gesture_active;
    if (event.type == NSEventTypeScrollWheel) {
      pan_gesture_active = true;
      // Ensure scroll inertia cancel event is not sent afterwards.
      last_scroll_momentum_changed_time = 0;
    } else if (event.type == NSEventTypeMagnify) {
      scale_gesture_active = true;
    } else if (event.type == NSEventTypeRotate) {
      rotate_gesture_active = true;
    }
    if (gestureAlreadyDown) {
      return;
    }
  }
  if (phase == kPanZoomEnd) {
    if (event.type == NSEventTypeScrollWheel) {
      pan_gesture_active = false;
    } else if (event.type == NSEventTypeMagnify) {
      scale_gesture_active = false;
    } else if (event.type == NSEventTypeRotate) {
      rotate_gesture_active = false;
    }
    if (pan_gesture_active || scale_gesture_active ||
        rotate_gesture_active) {
      return;
    }
  }

  // If a pointer added event hasn't been sent, synthesize one using this event for the basic
  // information.
  if (!flutter_state_is_added && phase != kAdd) {
    // Only the values extracted for use in flutterEvent below matter, the rest are dummy values.
    NSEvent* addEvent = [NSEvent enterExitEventWithType:NSEventTypeMouseEntered
                                               location:event.locationInWindow
                                          modifierFlags:0
                                              timestamp:event.timestamp
                                           windowNumber:event.windowNumber
                                                context:nil
                                            eventNumber:0
                                         trackingNumber:0
                                               userData:NULL];
    [self dispatchMouseEvent:addEvent phase:kAdd inView:flutterView];
  }

  NSPoint locationInView = [flutterView convertPoint:event.locationInWindow fromView:nil];
  NSPoint locationInBackingCoordinates = [flutterView convertPointToBacking:locationInView];
  int32_t device = kMousePointerDeviceId;
  FlutterPointerDeviceKind deviceKind = kFlutterPointerDeviceKindMouse;
  if (phase == kPanZoomStart || phase == kPanZoomUpdate || phase == kPanZoomEnd) {
    device = kPointerPanZoomDeviceId;
    deviceKind = kFlutterPointerDeviceKindTrackpad;
  }
  FlutterPointerEvent flutterEvent = {
      .struct_size = sizeof(flutterEvent),
      .phase = phase,
      .timestamp = static_cast<size_t>(event.timestamp * USEC_PER_SEC),
      .x = locationInBackingCoordinates.x,
      .y = -locationInBackingCoordinates.y,  // convertPointToBacking makes this negative.
      .device = device,
      .device_kind = deviceKind,
      // If a click triggered a synthesized kAdd, don't pass the buttons in that event.
      .buttons = phase == kAdd ? 0 : buttons,
  };

  if (phase == kPanZoomUpdate) {
    NSLog(@"kPanZoomUpdate");
    if (event.type == NSEventTypeScrollWheel) {
      NSLog(@"NSEventTypeScrollWheel");
      NSLog(@"%d %d %f", !!flutterView, !!flutterView.layer, flutterView.layer.contentsScale);
      delta_x += event.scrollingDeltaX * flutterView.layer.contentsScale;
      delta_y += event.scrollingDeltaY * flutterView.layer.contentsScale;
    } else if (event.type == NSEventTypeMagnify) {
      scale += event.magnification;
    } else if (event.type == NSEventTypeRotate) {
      rotation += event.rotation * (-M_PI / 180.0);
    }
    NSLog(@"%lu", event.type);
    flutterEvent.pan_x = delta_x;
    flutterEvent.pan_y = delta_y;
    // Scale value needs to be normalized to range 0->infinity.
    flutterEvent.scale = pow(2.0, scale);
    flutterEvent.rotation = rotation;
  } else if (phase == kPanZoomEnd) {
    [self gestureReset];
  } else if (phase != kPanZoomStart && event.type == NSEventTypeScrollWheel) {
    flutterEvent.signal_kind = kFlutterPointerSignalKindScroll;

    double pixelsPerLine = 1.0;
    if (!event.hasPreciseScrollingDeltas) {
      // The scrollingDelta needs to be multiplied by the line height.
      // CGEventSourceGetPixelsPerLine() will return 10, which will result in
      // scrolling that is noticeably slower than in other applications.
      // Using 40.0 as the multiplier to match Chromium.
      // See https://source.chromium.org/chromium/chromium/src/+/main:ui/events/cocoa/events_mac.mm
      pixelsPerLine = 40.0;
    }
    double scaleFactor = flutterView.layer.contentsScale;
    // When mouse input is received while shift is pressed (regardless of
    // any other pressed keys), Mac automatically flips the axis. Other
    // platforms do not do this, so we flip it back to normalize the input
    // received by the framework. The keyboard+mouse-scroll mechanism is exposed
    // in the ScrollBehavior of the framework so developers can customize the
    // behavior.
    // At time of change, Apple does not expose any other type of API or signal
    // that the X/Y axes have been flipped.
    double scaledDeltaX = -event.scrollingDeltaX * pixelsPerLine * scaleFactor;
    double scaledDeltaY = -event.scrollingDeltaY * pixelsPerLine * scaleFactor;
    if (event.modifierFlags & NSShiftKeyMask) {
      flutterEvent.scroll_delta_x = scaledDeltaY;
      flutterEvent.scroll_delta_y = scaledDeltaX;
    } else {
      flutterEvent.scroll_delta_x = scaledDeltaX;
      flutterEvent.scroll_delta_y = scaledDeltaY;
    }
  }

  _engineDispatchPointerEvent(flutterEvent);

  // Update tracking of state as reported to Flutter.
  if (phase == kDown) {
    flutter_state_is_down = true;
  } else if (phase == kUp) {
    flutter_state_is_down = false;
    if (has_pending_exit) {
      [self dispatchMouseEvent:event phase:kRemove inView:flutterView];
      has_pending_exit = false;
    }
  } else if (phase == kAdd) {
    flutter_state_is_added = true;
  } else if (phase == kRemove) {
    [self reset];
  }
}


- (void)gestureReset {
  delta_x = 0;
  delta_y = 0;
  scale = 0;
  rotation = 0;
}

- (void)reset {
  flutter_state_is_added = false;
  flutter_state_is_down = false;
  has_pending_exit = false;
  buttons = 0;
  [self gestureReset];
}

@end
