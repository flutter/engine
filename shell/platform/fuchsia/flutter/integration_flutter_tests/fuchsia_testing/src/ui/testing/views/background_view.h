// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_UI_TESTING_VIEWS_BACKGROUND_VIEW_H_
#define SRC_UI_TESTING_VIEWS_BACKGROUND_VIEW_H_

#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <lib/ui/scenic/cpp/resources.h>
#include <lib/ui/scenic/cpp/session.h>

#include "src/lib/ui/base_view/base_view.h"
#include "src/ui/testing/views/color.h"
#include "src/ui/testing/views/test_view.h"

namespace scenic {

// Test view with a solid background. This is a simplified |BaseView| that
// exposes the present callback.
//
// See also //src/lib/ui/base_view.
class BackgroundView : public TestView, private fuchsia::ui::scenic::SessionListener {
 public:
  static constexpr float kBackgroundElevation = 0.f;
  static constexpr Color kBackgroundColor = {0x67, 0x3a, 0xb7, 0xff};  // Deep Purple 500

  BackgroundView(ViewContext context, const std::string& debug_name = "BackgroundView");

  // |TestView|
  void set_present_callback(Session::PresentCallback present_callback) override;

  void SetImage(zx::vmo vmo, uint64_t size, fuchsia::images::ImageInfo info,
                fuchsia::images::MemoryType memory_type);

  void SetBackgroundColor(Color color);

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
  Session::PresentCallback present_callback_;
};

}  // namespace scenic
#endif  // SRC_UI_TESTING_VIEWS_BACKGROUND_VIEW_H_
