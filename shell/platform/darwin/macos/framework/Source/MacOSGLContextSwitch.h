#import <Cocoa/Cocoa.h>

/**
 * RAII wrapper that sets provided NSOpenGLContext as current and restores
 * original context on scope exit.
 */
class MacOSGLContextSwitch {
 public:
  explicit MacOSGLContextSwitch(NSOpenGLContext* context);
  ~MacOSGLContextSwitch();

  MacOSGLContextSwitch(const MacOSGLContextSwitch&) = delete;
  MacOSGLContextSwitch(MacOSGLContextSwitch&&) = delete;

 private:
  NSOpenGLContext* prev_;
};
