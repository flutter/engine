#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_WIN32_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_WIN32_WINDOW_H_

#include "win32_wrapper.h"
#include "windowing.h"

#include <windows.h>

#include <memory>
#include <optional>
#include <set>
#include <string>

namespace flutter {

// A class abstraction for a high DPI-aware Win32 Window. Intended to be
// inherited from by classes that wish to specialize with custom
// rendering and input handling.
class Win32Window {
 public:
  Win32Window();
  explicit Win32Window(std::shared_ptr<Win32Wrapper> wrapper);
  virtual ~Win32Window();

  // Retrieves a class instance pointer for |hwnd|.
  static auto GetThisFromHandle(HWND hwnd) -> Win32Window*;

  // Returns the backing window handle to enable clients to set icon and other
  // window properties. Returns nullptr if the window has been destroyed.
  auto GetHandle() const -> HWND;

  // If |quit_on_close| is true, closing this window will quit the application.
  void SetQuitOnClose(bool quit_on_close);

  // Returns true if closing this window will cause the application to quit.
  auto GetQuitOnClose() const -> bool;

  // Returns the bounds of the current client area.
  auto GetClientArea() const -> RECT;

  // Returns the current window archetype.
  auto GetArchetype() const -> WindowArchetype;

  // Returns the child windows.
  auto GetChildren() const -> std::set<Win32Window*> const&;

 protected:
  // Creates a native Win32 window. |title| is the window title string.
  // |client_size| specifies the requested size of the client rectangle (i.e.,
  // the size of the view). The window style is determined by |archetype|. For
  // |FlutterWindowArchetype::satellite| and |FlutterWindowArchetype::popup|,
  // both |parent| and |positioner| must be provided; |positioner| is used only
  // for these archetypes. For |FlutterWindowArchetype::dialog|, a modal dialog
  // is created if |parent| is specified; otherwise, the dialog is modeless.
  // After successful creation, |OnCreate| is called, and its result is
  // returned. Otherwise, the return value is false.
  auto Create(std::wstring const& title,
              WindowSize const& client_size,
              WindowArchetype archetype,
              std::optional<HWND> parent,
              std::optional<WindowPositioner> positioner) -> bool;

  // Release OS resources associated with window.
  void Destroy();

  // Inserts |content| into the window tree.
  void SetChildContent(HWND content);

  // Processes and route salient window messages for mouse handling,
  // size change and DPI. Delegates handling of these to member overloads that
  // inheriting classes can handle.
  virtual auto MessageHandler(HWND hwnd,
                              UINT message,
                              WPARAM wparam,
                              LPARAM lparam) -> LRESULT;

  // Called when Create is called, allowing subclass window-related setup.
  // Subclasses should return false if setup fails.
  virtual auto OnCreate() -> bool;

  // Called when Destroy is called.
  virtual void OnDestroy();

 private:
  friend class FlutterWindowController;

  // OS callback called by message pump. Handles the WM_NCCREATE message which
  // is passed when the non-client area is being created and enables automatic
  // non-client DPI scaling so that the non-client area automatically
  // responds to changes in DPI. All other messages are handled by the
  // controller's MessageHandler.
  static auto CALLBACK WndProc(HWND hwnd,
                               UINT message,
                               WPARAM wparam,
                               LPARAM lparam) -> LRESULT;

  // Wrapper for Win32 API calls.
  std::shared_ptr<Win32Wrapper> win32_;

  // The window's archetype (e.g., regular, dialog, popup).
  WindowArchetype archetype_{WindowArchetype::regular};

  // Windows that have this window as their parent or owner.
  std::set<Win32Window*> children_;

  // The number of popups in |children_|, used to quickly check whether this
  // window has any popups.
  size_t num_child_popups_{0};

  // Indicates whether closing this window will quit the application.
  bool quit_on_close_{false};

  // Handle for the top-level window.
  HWND window_handle_{nullptr};

  // Handle for hosted child content window.
  HWND child_content_{nullptr};

  // Offset between this window's position and its owner's position.
  POINT offset_from_owner_{0, 0};

  // Controls whether the non-client area can be redrawn as inactive.
  // Enabled by default, but temporarily disabled during child popup destruction
  // to prevent flickering.
  bool enable_redraw_non_client_as_inactive_{true};

  // Closes the popups of this window and returns the number of popups closed.
  auto CloseChildPopups() -> std::size_t;

  // Enables or disables this window and all its descendants.
  void EnableWindowAndDescendants(bool enable);

  // Enforces modal behavior by enabling the deepest dialog in the subtree
  // rooted at the top-level window, along with its descendants, while
  // disabling all other windows in the subtree. This ensures that the dialog
  // and its children remain active and interactive. If no dialog is found,
  // all windows in the subtree are enabled.
  void UpdateModalState();
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_WIN32_WINDOW_H_
