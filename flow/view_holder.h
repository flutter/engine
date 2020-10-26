// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_VIEW_HOLDER_H_
#define FLUTTER_FLOW_VIEW_HOLDER_H_

#include <fuchsia/ui/gfx/cpp/fidl.h>
#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/ui/scenic/cpp/id.h>
#include <lib/ui/scenic/cpp/resources.h>
#include <lib/ui/scenic/cpp/session.h>
#include <zircon/types.h>

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/task_runner.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkSize.h"

namespace flutter {

// Represents a Scenic |ViewHolder| resource that imports a |View| from another
// session.
//
// This object is created and destroyed on the |Rasterizer|'s thread.
class ViewHolder {
 public:
  using BindCallback = std::function<void(scenic::ResourceId)>;

  static void Create(zx_koid_t id,
                     fml::RefPtr<fml::TaskRunner> ui_task_runner,
                     fuchsia::ui::views::ViewHolderToken view_holder_token,
                     const BindCallback& on_bind_callback,
                     bool intercept_all_input = false);
  static void Destroy(zx_koid_t id);
  static ViewHolder* FromId(zx_koid_t id);

  ViewHolder(fml::RefPtr<fml::TaskRunner> ui_task_runner,
             fuchsia::ui::views::ViewHolderToken view_holder_token,
             const BindCallback& on_bind_callback,
             bool intercept_all_input);
  ~ViewHolder() = default;

  // Sets the properties/opacity of the child view by issuing a Scenic command.
  void SetProperties(double width,
                     double height,
                     double insetTop,
                     double insetRight,
                     double insetBottom,
                     double insetLeft,
                     bool focusable);

  // Creates or updates the contained ViewHolder resource using the specified
  // |SceneUpdateContext|.
  void UpdateScene(scenic::Session* session,
                   scenic::ContainerNode& container_node,
                   const SkPoint& offset,
                   const SkSize& size,
                   SkAlpha opacity,
                   bool hit_testable);

  bool hit_testable() { return hit_testable_; }
  void set_hit_testable(bool value) { hit_testable_ = value; }

  bool focusable() { return focusable_; }
  void set_focusable(bool value) { focusable_ = value; }

 private:
  // Helper class for setting up an invisible rectangle to catch all input. Rejected input will then
  // be re-injected into the scene below us.
  class InputInterceptor {
   public:
    InputInterceptor(scenic::Session* session) : opacity_node_(session), shape_node_(session) {
      opacity_node_.SetLabel("Flutter::InputInterceptor");
      opacity_node_.SetOpacity(0.f);

      // Set the shape node to capture all input. Any unwanted input will be reinjected.
      shape_node_.SetHitTestBehavior(fuchsia::ui::gfx::HitTestBehavior::kDefault);
      shape_node_.SetSemanticVisibility(false);


   scenic::Material material(session);
    material.SetColor(0xff, 0xff, 0xff, 0xff);  // White
    shape_node_.SetMaterial(material);

      opacity_node_.AddChild(shape_node_);
    }

    void UpdateDimensions(scenic::Session* session, float width, float height, float elevation) {
      opacity_node_.SetTranslation(width * 0.5f, height * 0.5f, elevation);
      shape_node_.SetShape(scenic::Rectangle(session, width, height));
    }

    const scenic::Node& node() { return opacity_node_; }

   private:
    scenic::OpacityNodeHACK opacity_node_;
    scenic::ShapeNode shape_node_;
  };

  fml::RefPtr<fml::TaskRunner> ui_task_runner_;

  std::unique_ptr<scenic::EntityNode> entity_node_;
  std::unique_ptr<scenic::OpacityNodeHACK> opacity_node_;
  std::unique_ptr<scenic::ViewHolder> view_holder_;

  std::unique_ptr<InputInterceptor> input_interceptor_;

  fuchsia::ui::views::ViewHolderToken pending_view_holder_token_;
  BindCallback pending_bind_callback_;

  bool hit_testable_ = true;
  bool focusable_ = true;
  bool intercept_all_input_ = false;

  fuchsia::ui::gfx::ViewProperties pending_properties_;
  bool has_pending_properties_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(ViewHolder);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_VIEW_HOLDER_H_
