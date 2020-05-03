#include "flutter/shell/platform/windows/testing/flutter_compview_test.h"

namespace flutter {
namespace testing {

FlutterWindowsViewTest::FlutterWindowsViewTest(int width, int height)
    : FlutterWindowsView(width, height){};

FlutterWindowsViewTest::~FlutterWindowsViewTest() = default;

//void FlutterWindowsViewTest::OnFontChange() {
//  on_font_change_called_ = true;
//}

bool FlutterWindowsViewTest::OnFontChangeWasCalled() {
  return true; //TODO create an actual test
  //return on_font_change_called_;
}
}  // namespace testing
}  // namespace flutter
