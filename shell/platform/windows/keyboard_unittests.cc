// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/common/json_message_codec.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/embedder/test_utils/key_codes.h"
#include "flutter/shell/platform/embedder/test_utils/proc_table_replacement.h"
#include "flutter/shell/platform/windows/flutter_windows_engine.h"
#include "flutter/shell/platform/windows/keyboard_key_channel_handler.h"
#include "flutter/shell/platform/windows/keyboard_key_embedder_handler.h"
#include "flutter/shell/platform/windows/keyboard_key_handler.h"
#include "flutter/shell/platform/windows/testing/engine_modifier.h"
#include "flutter/shell/platform/windows/testing/flutter_window_win32_test.h"
#include "flutter/shell/platform/windows/testing/mock_window_binding_handler.h"
#include "flutter/shell/platform/windows/testing/mock_window_win32.h"
#include "flutter/shell/platform/windows/testing/test_keyboard.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <functional>
#include <rapidjson/document.h>
#include <vector>

using testing::_;
using testing::Invoke;
using testing::Return;
using namespace ::flutter::testing::keycodes;

namespace flutter {
namespace testing {

namespace {

constexpr SHORT kStateMaskToggled = 0x01;
constexpr SHORT kStateMaskPressed = 0x80;

static LPARAM CreateKeyEventLparam(USHORT scancode,
                                   bool extended,
                                   bool was_down,
                                   USHORT repeat_count = 1,
                                   bool context_code = 0,
                                   bool transition_state = 1) {
  return ((LPARAM(transition_state) << 31) | (LPARAM(was_down) << 30) |
          (LPARAM(context_code) << 29) | (LPARAM(extended ? 0x1 : 0x0) << 24) |
          (LPARAM(scancode) << 16) | LPARAM(repeat_count));
}

class MockFlutterWindowWin32 : public FlutterWindowWin32, public MockMessageQueue {
 public:
  typedef std::function<void(const std::u16string& text)> U16StringHandler;

  MockFlutterWindowWin32(U16StringHandler on_text)
      : FlutterWindowWin32(800, 600), on_text_(std::move(on_text)) {
    ON_CALL(*this, GetDpiScale())
        .WillByDefault(Return(this->FlutterWindowWin32::GetDpiScale()));
  }
  virtual ~MockFlutterWindowWin32() {}

  // Prevent copying.
  MockFlutterWindowWin32(MockFlutterWindowWin32 const&) = delete;
  MockFlutterWindowWin32& operator=(MockFlutterWindowWin32 const&) = delete;

  // Wrapper for GetCurrentDPI() which is a protected method.
  UINT GetDpi() { return GetCurrentDPI(); }

  LRESULT Win32DefWindowProc(HWND hWnd,
                            UINT Msg,
                            WPARAM wParam,
                            LPARAM lParam) override {
    return kMockDefaultResult;
  }

  // Simulates a WindowProc message from the OS.
  LRESULT InjectWindowMessage(UINT const message,
                              WPARAM const wparam,
                              LPARAM const lparam) {
    return Win32SendMessage(NULL, message, wparam, lparam);
  }

  void OnText(const std::u16string& text) override { on_text_(text); }

  MOCK_METHOD1(OnDpiScale, void(unsigned int));
  MOCK_METHOD2(OnResize, void(unsigned int, unsigned int));
  MOCK_METHOD2(OnPointerMove, void(double, double));
  MOCK_METHOD3(OnPointerDown, void(double, double, UINT));
  MOCK_METHOD3(OnPointerUp, void(double, double, UINT));
  MOCK_METHOD0(OnPointerLeave, void());
  MOCK_METHOD0(OnSetCursor, void());
  MOCK_METHOD2(OnScroll, void(double, double));
  MOCK_METHOD0(GetDpiScale, float());
  MOCK_METHOD0(IsVisible, bool());
  MOCK_METHOD1(UpdateCursorRect, void(const Rect&));

  virtual BOOL Win32PeekMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) override {
    return MockMessageQueue::Win32PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
  }

 private:
  U16StringHandler on_text_;

  LRESULT Win32SendMessage(HWND hWnd,
                           UINT const message,
                           WPARAM const wparam,
                           LPARAM const lparam) override {
    return HandleMessage(message, wparam, lparam);
  }
};

class TestKeystate {
 public:
  void Set(int virtual_key, bool pressed, bool toggled_on = false) {
    state_[virtual_key] = (pressed ? kStateMaskPressed : 0) |
                          (toggled_on ? kStateMaskToggled : 0);
  }

  SHORT Get(int virtual_key) { return state_[virtual_key]; }

  KeyboardKeyEmbedderHandler::GetKeyStateHandler Getter() {
    return [this](int virtual_key) { return Get(virtual_key); };
  }

 private:
  std::map<int, SHORT> state_;
};

// A FlutterWindowsView that overrides the RegisterKeyboardHandlers function
// to register the keyboard hook handlers that can be spied upon.
class TestFlutterWindowsView : public FlutterWindowsView {
 public:
  TestFlutterWindowsView()
      // The WindowBindingHandler is used for window size and such, and doesn't
      // affect keyboard.
      : FlutterWindowsView(
            std::make_unique<::testing::NiceMock<MockWindowBindingHandler>>()),
        is_printable(true) {}

  bool is_printable;

  void InjectPendingEvents(MockFlutterWindowWin32* win32window) {
    win32window->InjectMessageList(pending_responds_.size(), pending_responds_.data());
    pending_responds_.clear();
  }

 protected:
  void RegisterKeyboardHandlers(
      BinaryMessenger* messenger,
      KeyboardKeyHandler::EventDispatcher dispatch_event,
      KeyboardKeyEmbedderHandler::GetKeyStateHandler get_key_state) override {
    FlutterWindowsView::RegisterKeyboardHandlers(
        messenger,
        [this](UINT cInputs, LPINPUT pInputs, int cbSize) -> UINT {
          return this->SendInput(cInputs, pInputs, cbSize);
        },
        key_state_.Getter());
  }

 private:
  UINT SendInput(UINT cInputs, LPINPUT pInputs, int cbSize) {
    // Simulate the event loop by just sending the event sent to
    // "SendInput" directly to the window.
    const KEYBDINPUT kbdinput = pInputs->ki;
    const UINT message =
        (kbdinput.dwFlags & KEYEVENTF_KEYUP) ? WM_KEYUP : WM_KEYDOWN;
    const bool is_key_up = kbdinput.dwFlags & KEYEVENTF_KEYUP;
    const LPARAM lparam = CreateKeyEventLparam(
        kbdinput.wScan, kbdinput.dwFlags & KEYEVENTF_EXTENDEDKEY, is_key_up);
    pending_responds_.push_back(Win32Message{message, kbdinput.wVk, lparam, kMockDefaultResult});
    if (is_printable && (kbdinput.dwFlags & KEYEVENTF_KEYUP) == 0) {
      pending_responds_.push_back(Win32Message{WM_CHAR, kbdinput.wVk, lparam, kMockDefaultResult});
    }
    return 1;
  }

  std::vector<Win32Message> pending_responds_;
  TestKeystate key_state_;
};

// A struct to use as a FlutterPlatformMessageResponseHandle so it can keep the
// callbacks and user data passed to the engine's
// PlatformMessageCreateResponseHandle for use in the SendPlatformMessage
// overridden function.
struct TestResponseHandle {
  FlutterDesktopBinaryReply callback;
  void* user_data;
};

typedef enum {
  kKeyCallOnKey,
  kKeyCallOnText,
} KeyCallType;

typedef struct {
  KeyCallType type;

  // Only one of the following fields should be assigned.
  FlutterKeyEvent key_event;
  std::u16string text;
} KeyCall;

static std::vector<KeyCall> key_calls;

void clear_key_calls() {
  for (KeyCall& key_call : key_calls) {
    if (key_call.type == kKeyCallOnKey &&
        key_call.key_event.character != nullptr) {
      delete[] key_call.key_event.character;
    }
  }
  key_calls.clear();
}

std::unique_ptr<FlutterWindowsEngine> GetTestEngine();

class KeyboardTester {
 public:
  explicit KeyboardTester() {
    view_ = std::make_unique<TestFlutterWindowsView>();
    view_->SetEngine(std::move(GetTestEngine()));
    window_ = std::make_unique<MockFlutterWindowWin32>(
        [](const std::u16string& text) {
          key_calls.push_back(KeyCall{
              .type = kKeyCallOnText,
              .text = text,
          });
        });
    window_->SetView(view_.get());
  }

  void Responding(bool response) {
    test_response = response;
  }

  void NextMessageShouldDefault() {
    next_event_should_default_ = true;
  }

  void InjectMessages(int count, Win32Message message1, ...) {
    Win32Message messages[count];
    messages[0] = message1;
    va_list args;
    va_start(args, message1);
    for (int i = 1; i < count; i += 1) {
      messages[i] = va_arg(args, Win32Message);
    }
    va_end(args);
    window_->InjectMessageList(count, messages);
  }

  void InjectPendingEvents() {
    view_->InjectPendingEvents(window_.get());
  }

  static bool test_response;

 private:
  std::unique_ptr<TestFlutterWindowsView> view_;
  std::unique_ptr<MockFlutterWindowWin32> window_;

  bool next_event_should_default_ = false;
};


bool KeyboardTester::test_response = false;

std::unique_ptr<std::vector<uint8_t>> keyHandlingResponse(bool handled) {
  rapidjson::Document document;
  auto& allocator = document.GetAllocator();
  document.SetObject();
  document.AddMember("handled", KeyboardTester::test_response, allocator);
  return flutter::JsonMessageCodec::GetInstance().EncodeMessage(document);
}

// Returns an engine instance configured with dummy project path values, and
// overridden methods for sending platform messages, so that the engine can
// respond as if the framework were connected.
std::unique_ptr<FlutterWindowsEngine> GetTestEngine() {
  FlutterDesktopEngineProperties properties = {};
  properties.assets_path = L"C:\\foo\\flutter_assets";
  properties.icu_data_path = L"C:\\foo\\icudtl.dat";
  properties.aot_library_path = L"C:\\foo\\aot.so";
  FlutterProjectBundle project(properties);
  auto engine = std::make_unique<FlutterWindowsEngine>(project);

  EngineModifier modifier(engine.get());

  // This mock handles channel messages.
  modifier.embedder_api().SendPlatformMessage =
      [](FLUTTER_API_SYMBOL(FlutterEngine) engine,
         const FlutterPlatformMessage* message) {
        if (std::string(message->channel) == std::string("flutter/settings")) {
          return kSuccess;
        }
        if (std::string(message->channel) == std::string("flutter/keyevent")) {
          auto response = keyHandlingResponse(true);
          const TestResponseHandle* response_handle =
              reinterpret_cast<const TestResponseHandle*>(
                  message->response_handle);
          if (response_handle->callback != nullptr) {
            response_handle->callback(response->data(), response->size(),
                                      response_handle->user_data);
          }
          return kSuccess;
        }
        return kSuccess;
      };

  // This mock handles key events sent through the embedder API,
  // and records it in `key_calls`.
  modifier.embedder_api().SendKeyEvent =
      [](FLUTTER_API_SYMBOL(FlutterEngine) engine, const FlutterKeyEvent* event,
         FlutterKeyEventCallback callback, void* user_data) {
        FlutterKeyEvent clone_event = *event;
        clone_event.character = event->character == nullptr
                                    ? nullptr
                                    : clone_string(event->character);
        key_calls.push_back(KeyCall{
            .type = kKeyCallOnKey,
            .key_event = clone_event,
        });
        if (callback != nullptr) {
          callback(KeyboardTester::test_response, user_data);
        }
        return kSuccess;
      };

  // The following mocks enable channel mocking.
  modifier.embedder_api().PlatformMessageCreateResponseHandle =
      [](auto engine, auto data_callback, auto user_data, auto response_out) {
        TestResponseHandle* response_handle = new TestResponseHandle();
        response_handle->user_data = user_data;
        response_handle->callback = data_callback;
        *response_out = reinterpret_cast<FlutterPlatformMessageResponseHandle*>(
            response_handle);
        return kSuccess;
      };

  modifier.embedder_api().PlatformMessageReleaseResponseHandle =
      [](FLUTTER_API_SYMBOL(FlutterEngine) engine,
         FlutterPlatformMessageResponseHandle* response) {
        const TestResponseHandle* response_handle =
            reinterpret_cast<const TestResponseHandle*>(response);
        delete response_handle;
        return kSuccess;
      };

  // The following mocks allows RunWithEntrypoint to be run, which creates a
  // non-empty FlutterEngine and enables SendKeyEvent.

  modifier.embedder_api().Run =
      [](size_t version, const FlutterRendererConfig* config,
         const FlutterProjectArgs* args, void* user_data,
         FLUTTER_API_SYMBOL(FlutterEngine) * engine_out) {
        *engine_out = reinterpret_cast<FLUTTER_API_SYMBOL(FlutterEngine)>(1);

        return kSuccess;
      };
  modifier.embedder_api().UpdateLocales =
      [](auto engine, const FlutterLocale** locales, size_t locales_count) {
        return kSuccess;
      };
  modifier.embedder_api().SendWindowMetricsEvent =
      [](auto engine, const FlutterWindowMetricsEvent* event) {
        return kSuccess;
      };
  modifier.embedder_api().Shutdown = [](auto engine) { return kSuccess; };

  engine->RunWithEntrypoint(nullptr);
  return engine;
}

constexpr uint64_t kScanCodeKeyA = 0x1e;
// constexpr uint64_t kScanCodeNumpad1 = 0x4f;
// constexpr uint64_t kScanCodeNumLock = 0x45;
// constexpr uint64_t kScanCodeControl = 0x1d;
// constexpr uint64_t kScanCodeShiftLeft = 0x2a;
// constexpr uint64_t kScanCodeShiftRight = 0x36;

constexpr uint64_t kVirtualKeyA = 0x41;

constexpr bool kExtended = true;
constexpr bool kNotExtended = false;
constexpr bool kWasDown = true;
constexpr bool kWasUp = false;
constexpr bool kSynthesized = true;
constexpr bool kNotSynthesized = false;

}  // namespace

#define EXPECT_CALL_IS_EVENT(_key_call, ...) \
  EXPECT_EQ(_key_call.type, kKeyCallOnKey);  \
  EXPECT_EVENT_EQUALS(_key_call.key_event, __VA_ARGS__);

TEST(KeyboardTest, LowerCaseA) {
  KeyboardTester tester;
  tester.Responding(true);

  // US Keyboard layout

  // Press A
  tester.InjectMessages(2,
    Win32Message{WM_KEYDOWN, kVirtualKeyA, CreateKeyEventLparam(kScanCodeKeyA, kNotExtended, kWasUp)},
    Win32Message{WM_CHAR, kVirtualKeyA, CreateKeyEventLparam(kScanCodeKeyA, kNotExtended, kWasUp)}
  );

  EXPECT_EQ(key_calls.size(), 1);
  EXPECT_CALL_IS_EVENT(key_calls[0], kFlutterKeyEventTypeDown, kPhysicalKeyA,
                       kLogicalKeyA, "A", kNotSynthesized);
  clear_key_calls();

  tester.InjectPendingEvents();
  EXPECT_EQ(key_calls.size(), 0);

  // Release A
  tester.InjectMessages(1,
    Win32Message{WM_KEYUP, kVirtualKeyA, CreateKeyEventLparam(kScanCodeKeyA, kNotExtended, kWasDown)});

  EXPECT_EQ(key_calls.size(), 1);
  EXPECT_CALL_IS_EVENT(key_calls[0], kFlutterKeyEventTypeUp, kPhysicalKeyA,
                       kLogicalKeyA, "", kNotSynthesized);
  clear_key_calls();

  tester.InjectPendingEvents();
  EXPECT_EQ(key_calls.size(), 0);
}

}  // namespace testing
}  // namespace flutter
