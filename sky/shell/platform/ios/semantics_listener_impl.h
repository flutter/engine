// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_SHELL_PLATFORM_IOS_SEMANTICS_LISTENER_IMPL_H_
#define SKY_SHELL_PLATFORM_IOS_SEMANTICS_LISTENER_IMPL_H_

#include <map>
#include <UIKit/UIKit.h>

#include "base/macros.h"
#include "mojo/public/cpp/bindings/array.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "sky/services/semantics/semantics.mojom.h"
#include "sky/shell/platform/ios/FlutterView.h"
#include "third_party/skia/include/utils/SkMatrix44.h"

namespace sky {
namespace shell {

float* asFloatArray(mojo::Array<float> input);

class AccessibilityNode;

class SemanticsListenerImpl : public semantics::SemanticsListener {
 public:
  SemanticsListenerImpl(FlutterView*, semantics::SemanticsServerPtr);
  ~SemanticsListenerImpl() override;

  void UpdateSemanticsTree(
      mojo::Array<semantics::SemanticsNodePtr> nodes) override;

 private:
  class AccessibilityNode;

  AccessibilityNode* updateSemanticsNode(
      const semantics::SemanticsNodePtr& node);
  void removePersistentNode(AccessibilityNode* node);

  NSArray* createAccessibleElements();

  FlutterView* _view;
  mojo::StrongBinding<semantics::SemanticsListener> _binding;

  typedef std::map<long, AccessibilityNode*> NodeMap;
  typedef std::map<long, AccessibilityNode*>::iterator NodeMapIterator;
  NodeMap _nodes;

  DISALLOW_COPY_AND_ASSIGN(SemanticsListenerImpl);
};

class SemanticsListenerImpl::AccessibilityNode {
 public:
  AccessibilityNode(SemanticsListenerImpl* outer,
                    const semantics::SemanticsNodePtr& node);
  ~AccessibilityNode();

  void update(const semantics::SemanticsNodePtr& node);
  void populateAccessibleElements(NSMutableArray* accessibleElements);

  uint32_t id = -1;
  AccessibilityNode* parent = NULL;

  typedef std::vector<AccessibilityNode*> NodeList;
  typedef std::vector<AccessibilityNode*>::iterator NodeListIterator;
  NodeList children;

 private:
  SemanticsListenerImpl* outer;

  void invalidateGlobalGeometry();

  CGRect getGlobalRect();
  SkMatrix44& getGlobalTransform();
  float* transformPoint(SkMatrix44& transform, float point[]);

  bool canBeTapped;
  bool canBeLongPressed;
  bool canBeScrolledHorizontally;
  bool canBeScrolledVertically;
  bool hasCheckedState;
  bool isChecked;
  char* label = NULL;

  SkMatrix44 transform;
  SkMatrix44 globalTransform = SkMatrix44();
  float left;
  float top;
  float width;
  float height;

  bool geometryDirty = true;
  CGRect globalRect;

  DISALLOW_COPY_AND_ASSIGN(AccessibilityNode);
};

}  // namespace shell
}  // namespace sky

#endif  // SKY_SHELL_PLATFORM_IOS_SEMANTICS_LISTENER_IMPL_H_
