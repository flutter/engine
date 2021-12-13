// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_MANAGER_H_

#include <windows.h>
#include <map>

namespace flutter {

class KeyboardManagerWin32 {
 public:
  class WindowDelegate {
   public:
    virtual ~WindowDelegate() = default;

    // Called when text input occurs.
    virtual void OnText(const std::u16string& text) = 0;

    // Called when raw keyboard input occurs.
    //
    // Returns true if the event was handled, indicating that DefWindowProc
    // should not be called on the event by the main message loop.
    virtual bool OnKey(int key,
                       int scancode,
                       int action,
                       char32_t character,
                       bool extended,
                       bool was_down) = 0;

    virtual UINT Win32DispatchEvent(UINT cInputs,
                                    LPINPUT pInputs,
                                    int cbSize) = 0;

    virtual SHORT Win32GetKeyState(int nVirtKey) = 0;

    // Win32's PeekMessage.
    //
    // Used to process key messages.
    virtual BOOL Win32PeekMessage(LPMSG lpMsg,
                                  UINT wMsgFilterMin,
                                  UINT wMsgFilterMax,
                                  UINT wRemoveMsg) = 0;

    // Win32's MapVirtualKey(*, MAPVK_VK_TO_CHAR).
    //
    // Used to process key messages.
    virtual uint32_t Win32MapVkToChar(uint32_t virtual_key) = 0;
  };

  KeyboardManagerWin32(WindowDelegate* delegate);

  // TODO
  // Returns true if handled.
  bool HandleMessage(UINT const message,
                     WPARAM const wparam,
                     LPARAM const lparam);

 private:
  // Returns the type of the next WM message.
  //
  // The parameters limits the range of interested messages. See Win32's
  // |PeekMessage| for information.
  //
  // If there's no message, returns 0.
  UINT PeekNextMessageType(UINT wMsgFilterMin, UINT wMsgFilterMax);

  WindowDelegate* window_delegate_;

  // Keeps track of the last key code produced by a WM_KEYDOWN or WM_SYSKEYDOWN
  // message.
  int keycode_for_char_message_ = 0;

  std::map<uint16_t, std::u16string> text_for_scancode_on_redispatch_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_MANAGER_H_
