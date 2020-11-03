#import "flutter/shell/platform/darwin/macos/framework/Source/MacOSGLContextSwitch.h"

MacOSGLContextSwitch::MacOSGLContextSwitch(NSOpenGLContext* context) {
  prev_ = [NSOpenGLContext currentContext];
  [context makeCurrentContext];
}

MacOSGLContextSwitch::~MacOSGLContextSwitch() {
  if (prev_) {
    [prev_ makeCurrentContext];
  } else {
    [NSOpenGLContext clearCurrentContext];
  }
}
