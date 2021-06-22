// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/ui/testing/views/opacity_view.h"

#include <lib/fostr/fidl/fuchsia/ui/gfx/formatting.h>
#include <lib/syslog/cpp/macros.h>
#include <zircon/status.h>

namespace scenic {

OpacityView::OpacityView(ViewContext context, const std::string& debug_name)
    : binding_(this, std::move(context.session_and_listener_request.second)),
      session_(std::move(context.session_and_listener_request.first)),
      view_(&session_, std::move(context.view_token), debug_name),
      background_node_(&session_),
      background_material_(&session_),
      opacity_node_(&session_),
      foreground_node_(&session_),
      foreground_material_(&session_) {
  binding_.set_error_handler([](zx_status_t status) {
    FX_LOGS(FATAL) << "Session listener binding: " << zx_status_get_string(status);
  });

  background_node_.SetMaterial(background_material_);
  view_.AddChild(background_node_);

  foreground_node_.SetMaterial(foreground_material_);
  view_.AddChild(opacity_node_);
  opacity_node_.AddChild(foreground_node_);

  session_.Present(0, [](auto) {});
}

void OpacityView::set_present_callback(Session::PresentCallback present_callback) {
  FX_CHECK(present_callback && !present_callback_);
  present_callback_ = std::move(present_callback);
}

void OpacityView::set_foreground_opacity(float opacity) { opacity_node_.SetOpacity(opacity); }

void OpacityView::set_background_color(uint8_t r, uint8_t g, uint8_t b) {
  background_material_.SetColor(r, g, b, 0xff);
}

void OpacityView::set_foreground_color(uint8_t r, uint8_t g, uint8_t b) {
  foreground_material_.SetColor(r, g, b, 0xff);
}

void OpacityView::Draw(float cx, float cy, float sx, float sy) {
  Rectangle shape(&session_, sx, sy);
  background_node_.SetShape(shape);
  background_node_.SetTranslation({cx, cy, -kBackgroundElevation});
  foreground_node_.SetShape(shape);
  foreground_node_.SetTranslation({cx, cy, -kForegroundElevation});
}

void OpacityView::Present() {
  session_.Present(
      0, present_callback_ ? std::move(present_callback_) : [](auto) {});
}

void OpacityView::OnScenicEvent(std::vector<fuchsia::ui::scenic::Event> events) {
  FX_LOGS(INFO) << "OnScenicEvent";
  for (const auto& event : events) {
    if (event.Which() == fuchsia::ui::scenic::Event::Tag::kGfx &&
        event.gfx().Which() == fuchsia::ui::gfx::Event::Tag::kViewPropertiesChanged) {
      const auto& evt = event.gfx().view_properties_changed();
      fuchsia::ui::gfx::BoundingBox layout_box = ViewPropertiesLayoutBox(evt.properties);

      const auto sz = Max(layout_box.max - layout_box.min, 0.f);
      OnViewPropertiesChanged(sz);
    }
  }
}

void OpacityView::OnScenicError(std::string error) { FX_LOGS(FATAL) << "OnScenicError: " << error; }

void OpacityView::OnViewPropertiesChanged(const fuchsia::ui::gfx::vec3& sz) {
  FX_LOGS(INFO) << "Metrics: " << sz.x << "x" << sz.y << "x" << sz.z;
  if (sz.x == 0.f || sz.y == 0.f || sz.z == 0.f)
    return;

  Draw(sz.x * .5f, sz.y * .5f, sz.x, sz.y);
  Present();
}

}  // namespace scenic
