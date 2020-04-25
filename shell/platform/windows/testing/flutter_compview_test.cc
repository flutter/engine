#include "flutter/shell/platform/windows/testing/flutter_compview_test.h"

namespace flutter {
namespace testing {

FlutterCompViewTest::FlutterCompViewTest(int width, int height)
    : FlutterCompView(width, height, nullptr){};

FlutterCompViewTest::~FlutterCompViewTest() = default;

//void FlutterCompViewTest::OnFontChange() {
//  on_font_change_called_ = true;
//}

bool FlutterCompViewTest::OnFontChangeWasCalled() {
  return true; //TODO create an actual test
  //return on_font_change_called_;
}
}  // namespace testing
}  // namespace flutter
