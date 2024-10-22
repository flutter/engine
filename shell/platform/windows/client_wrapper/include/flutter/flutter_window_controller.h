#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_WINDOW_CONTROLLER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_WINDOW_CONTROLLER_H_

#include <mutex>

#include "flutter_engine.h"
#include "method_channel.h"
#include "win32_wrapper.h"
#include "windowing.h"

namespace flutter {

// A singleton controller for Flutter windows.
class FlutterWindowController {
 public:
  virtual ~FlutterWindowController();

  // Prevent copying.
  FlutterWindowController(FlutterWindowController const&) = delete;
  FlutterWindowController& operator=(FlutterWindowController const&) = delete;

  void SetEngine(std::shared_ptr<FlutterEngine> engine);
  auto CreateFlutterWindow(std::wstring const& title,
                           WindowSize const& size,
                           WindowArchetype archetype,
                           std::optional<WindowPositioner> positioner,
                           std::optional<FlutterViewId> parent_view_id)
      -> std::optional<WindowMetadata>;
  auto DestroyFlutterWindow(FlutterViewId view_id) -> bool;

  static FlutterWindowController& GetInstance() {
    static FlutterWindowController instance;
    return instance;
  }

 protected:
  FlutterWindowController();
  FlutterWindowController(std::shared_ptr<Win32Wrapper> wrapper);

  void MethodCallHandler(MethodCall<> const& call, MethodResult<>& result);
  auto MessageHandler(HWND hwnd,
                      UINT message,
                      WPARAM wparam,
                      LPARAM lparam) -> LRESULT;

  virtual void SendOnWindowCreated(
      FlutterViewId view_id,
      std::optional<FlutterViewId> parent_view_id) const;
  virtual void SendOnWindowDestroyed(FlutterViewId view_id) const;
  virtual void SendOnWindowChanged(FlutterViewId view_id) const;

 private:
  friend class Win32Window;

  void DestroyWindows();
  auto GetWindowSize(FlutterViewId view_id) const -> WindowSize;
  void HandleCreateWindow(WindowArchetype archetype,
                          MethodCall<> const& call,
                          MethodResult<>& result);
  void HandleDestroyWindow(MethodCall<> const& call, MethodResult<>& result);
  // Hides all satellite windows in the application, except those that are
  // descendants of |opt_out_hwnd| or have a dialog as a child. By default,
  // |opt_out_hwnd| is null, so no window is excluded.
  void HideWindowsSatellites(HWND opt_out_hwnd = nullptr);
  // Shows the satellite windows of |hwnd| and of its ancestors.
  void ShowWindowAndAncestorsSatellites(HWND hwnd);

  // Controls whether satellites are hidden when their top-level window
  // and all its children become inactive. If null, satellite hiding
  // is enabled. If not null, it contains the handle of the window that
  // disabled the hiding, and it will be reset when the window if fully
  // destroyed.
  HWND disable_satellite_hiding_{nullptr};

  mutable std::mutex mutex_;
  std::shared_ptr<Win32Wrapper> win32_;
  std::unique_ptr<MethodChannel<>> channel_;
  std::shared_ptr<FlutterEngine> engine_;
  std::unordered_map<FlutterViewId, std::unique_ptr<Win32Window>> windows_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_WINDOW_CONTROLLER_H_
