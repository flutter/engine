// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_UI_TESTING_VIEWS_OPACITY_VIEW_H_
#define SRC_UI_TESTING_VIEWS_OPACITY_VIEW_H_

#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <lib/ui/scenic/cpp/resources.h>
#include <lib/ui/scenic/cpp/session.h>

#include "src/lib/ui/base_view/base_view.h"
#include "src/ui/testing/views/color.h"
#include "src/ui/testing/views/test_view.h"

namespace scenic {

// Test view with a solid background and a translucent foreground layer.  This
// is a simplified |BaseView| that exposes the present callback.
//
// See also //src/lib/ui/base_view.
class OpacityView : public TestView, private fuchsia::ui::scenic::SessionListener {
 public:
  static constexpr float kBackgroundElevation = 0.f;
  static constexpr float kForegroundElevation = 10.f;

  explicit OpacityView(ViewContext context, const std::string& debug_name = "OpacityView");

  // |TestView|
  void set_present_callback(Session::PresentCallback present_callback) override;

  // Present() must be called afterward in order for any of these setters to
  // take effect.
  void set_foreground_opacity(float opacity);
  void set_background_color(uint8_t r, uint8_t g, uint8_t b);
  void set_foreground_color(uint8_t r, uint8_t g, uint8_t b);

 protected:
  Session* session() { return &session_; }
  View* view() { return &view_; }

  // Updates the scene graph; does not present.
  virtual void Draw(float cx, float cy, float sx, float sy);

  // Presents updates to the scene graph, with the previously set present
  // callback, if set, as a one-off.
  void Present();

 private:
  // |fuchsia::ui::scenic::SessionListener|
  void OnScenicEvent(std::vector<fuchsia::ui::scenic::Event> events) override;

  // |fuchsia::ui::scenic::SessionListener|
  void OnScenicError(std::string error) override;

  void OnViewPropertiesChanged(const fuchsia::ui::gfx::vec3& sz);

  fidl::Binding<fuchsia::ui::scenic::SessionListener> binding_;
  Session session_;
  View view_;

  ShapeNode background_node_;
  Material background_material_;
  OpacityNodeHACK opacity_node_;
  ShapeNode foreground_node_;
  Material foreground_material_;

  Session::PresentCallback present_callback_;
};

}  // namespace scenic
#endif  // SRC_UI_TESTING_VIEWS_OPACITY_VIEW_H_
