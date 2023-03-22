// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "window_top_level_message_handler.h"

#include <TlHelp32.h>
#include <Windows.h>
#include <WinUser.h>
#include <tchar.h>

#include "flutter/shell/platform/windows/flutter_windows_engine.h"

namespace flutter {

static constexpr char kExitTypeCancelable[] = "cancelable";

WindowTopLevelMessageHandler::WindowTopLevelMessageHandler(FlutterWindowsEngine& engine) : engine_(engine) {}

WindowTopLevelMessageHandler::~WindowTopLevelMessageHandler() {}

void WindowTopLevelMessageHandler::Quit(int64_t exit_code) {
  PostQuitMessage(exit_code);
}

bool WindowTopLevelMessageHandler::WindowProc(HWND hwnd, UINT msg, WPARAM wpar, LPARAM lpar, LRESULT* result) {
  switch (msg) {
    case WM_CLOSE:
      if (IsLastWindowOfProcess()) {
        engine_.RequestApplicationQuit(kExitTypeCancelable, wpar);
      }
      return true;
  }
  return false;
}

static BOOL CALLBACK WindowEnumCallback(HWND hwnd, LPARAM user_data) {
  HWND parent = GetParent(hwnd);
  if (parent == NULL) {
    int64_t& count = *static_cast<int64_t*>(reinterpret_cast<void*>(user_data));
    count++;
  }
  return true;
}

bool WindowTopLevelMessageHandler::IsLastWindowOfProcess() {
  // TODO(schectman): This currently is not working. Test apps have 3 "windows" to their process.
  DWORD pid = GetCurrentProcessId();
  HANDLE thread_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (thread_snapshot == INVALID_HANDLE_VALUE) {
    FML_LOG(ERROR) << "Failed to get threads snapshot";
    return true;
  }

  THREADENTRY32 thread;
  thread.dwSize = sizeof(thread);
  if (!Thread32First(thread_snapshot, &thread)) {
    DWORD error_num = GetLastError();
    char msg[256];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error_num, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg, 256, nullptr);
    FML_LOG(ERROR) << "Failed to get thread(" << error_num << "): " << msg;
    CloseHandle(thread_snapshot);
    return true;
  }

  int num_windows = 0;
  do {
    if (thread.th32OwnerProcessID == pid) {
      EnumThreadWindows(thread.th32ThreadID, WindowEnumCallback, reinterpret_cast<LPARAM>(static_cast<void*>(&num_windows)));
    }
  } while (Thread32Next(thread_snapshot, &thread));

  CloseHandle(thread_snapshot);
  return num_windows == 1;
}

}
