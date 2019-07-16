// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOPAZ_RUNTIME_FLUTTER_RUNNER_FUCHSIA_ACCESSIBILITY_H_
#define TOPAZ_RUNTIME_FLUTTER_RUNNER_FUCHSIA_ACCESSIBILITY_H_

#include <fuchsia/accessibility/semantics/cpp/fidl.h>
#include <fuchsia/sys/cpp/fidl.h>
#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/fidl/cpp/binding_set.h>
#include <lib/sys/cpp/service_directory.h>

#include <memory>

namespace flutter_runner {

// Host platform accessibility API wrapper. Called by |AccessibilityBridge|.
//
// This provides an abstraction of the full set of host platform calls that
// |AccessibilityBridge| needs to call. Implemented as an abstract base class
// in order to allow passing a fake implementation to |AccessibilityBridge|
// unit tests.
class FuchsiaAccessibility {
 public:
  static std::unique_ptr<FuchsiaAccessibility> Create(
      std::shared_ptr<sys::ServiceDirectory> services,
      fuchsia::ui::views::ViewRef view_ref);

  virtual ~FuchsiaAccessibility() = default;

  virtual void UpdateSemanticNodes(
      std::vector<fuchsia::accessibility::semantics::Node> nodes) = 0;
  virtual void DeleteSemanticNodes(std::vector<uint32_t> node_ids) = 0;
  virtual void Commit() = 0;
};

}  // namespace flutter_runner

#endif  // TOPAZ_RUNTIME_FLUTTER_RUNNER_FUCHSIA_ACCESSIBILITY_H_
