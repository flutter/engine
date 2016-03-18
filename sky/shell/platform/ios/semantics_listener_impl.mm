// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/platform/ios/semantics_listener_impl.h"

namespace sky {
namespace shell {

SemanticsListenerImpl::SemanticsListenerImpl(
    FlutterView* view,
    semantics::SemanticsServerPtr semanticsServer)
    : _view(view), _binding(this) {
  mojo::InterfaceHandle<semantics::SemanticsListener> listener;
  _binding.Bind(&listener);
  semanticsServer->AddSemanticsListener(listener.Pass());
}

void SemanticsListenerImpl::UpdateSemanticsTree(
    mojo::Array<semantics::SemanticsNodePtr> nodes) {
  for (const semantics::SemanticsNodePtr& node : nodes) {
    updateSemanticsNode(node);
  }
  NSArray* accessibleElements = createAccessibleElements();
  [_view updateAccessibleElements:accessibleElements];
  [accessibleElements release];
}

SemanticsListenerImpl::AccessibilityNode*
SemanticsListenerImpl::updateSemanticsNode(
    const semantics::SemanticsNodePtr& node) {
  AccessibilityNode* persistentNode;
  NodeMapIterator iter = _nodes.find(node->id);
  if (iter == _nodes.end()) {
    persistentNode = new AccessibilityNode(this, node);
    _nodes.insert(NodeMap::value_type(node->id, persistentNode));
  } else {
    persistentNode = iter->second;
    persistentNode->update(node);
  }
  assert(persistentNode != NULL);
  return persistentNode;
}

void SemanticsListenerImpl::removePersistentNode(AccessibilityNode* node) {
  assert(_nodes.find(node->id) != _nodes.end());
  assert(_nodes.at(node->id)->parent == NULL);
  _nodes.erase(node->id);
  for (AccessibilityNode* child : node->children) {
    removePersistentNode(child);
  }
}

NSArray* SemanticsListenerImpl::createAccessibleElements() {
  NSMutableArray* accessibleElements = [[NSMutableArray alloc] init];
  for (NodeMapIterator iter = _nodes.begin(); iter != _nodes.end(); iter++) {
    if (iter->second->parent == NULL) {
      iter->second->populateAccessibleElements(accessibleElements);
    }
  }
  return accessibleElements;
}

SemanticsListenerImpl::~SemanticsListenerImpl() {}

SemanticsListenerImpl::AccessibilityNode::AccessibilityNode(
    SemanticsListenerImpl* outer,
    const semantics::SemanticsNodePtr& node)
    : outer(outer) {
  update(node);
}

void SemanticsListenerImpl::AccessibilityNode::update(
    const semantics::SemanticsNodePtr& node) {
  if (id == (uint32_t)-1) {
    id = node->id;
  }
  assert(id == node->id);

  if (!node->flags.is_null()) {
    canBeTapped = node->flags->canBeTapped;
    canBeLongPressed = node->flags->canBeLongPressed;
    canBeScrolledHorizontally = node->flags->canBeScrolledHorizontally;
    canBeScrolledVertically = node->flags->canBeScrolledVertically;
    hasCheckedState = node->flags->hasCheckedState;
    isChecked = node->flags->isChecked;
  }

  if (!node->strings.is_null()) {
    if (label) {
      delete label;
    }
    label = (char*)malloc((node->strings->label.size() + 1) * sizeof(char));
    std::strcpy(label, node->strings->label.data());
  }

  if (!node->geometry.is_null()) {
    if (!node->geometry->transform.is_null()) {
      float* values = asFloatArray(node->geometry->transform.Pass());
      transform.setColMajorf(values);
      delete values;
    }
    left = node->geometry->left;
    top = node->geometry->top;
    width = node->geometry->width;
    height = node->geometry->height;
  }

  if (!node->children.is_null()) {
    const std::vector<AccessibilityNode*>& oldChildren = children;
    children = std::vector<AccessibilityNode*>();
    for (AccessibilityNode* child : oldChildren) {
      assert(child->parent != NULL);
      child->parent = NULL;
      // TODO(tvolkert): recursively set parent to null on descendant
      // tree, or assert may fail in removePersistentNode()
    }
    for (const semantics::SemanticsNodePtr& childNode : node->children) {
      AccessibilityNode* child = outer->updateSemanticsNode(childNode);
      child->parent = this;
      children.push_back(child);
    }
    for (AccessibilityNode* child : oldChildren) {
      if (child->parent == NULL) {
        outer->removePersistentNode(child);
      }
    }
  }

  // Has to be done after children are updated
  // since they also get marked dirty
  invalidateGlobalGeometry();
}

float* asFloatArray(mojo::Array<float> input) {
  float* result = (float*)malloc(input.size() * sizeof(float));
  for (size_t i = 0; i < input.size(); i++) {
    result[i] = input[i];
  }
  return result;
}

void SemanticsListenerImpl::AccessibilityNode::invalidateGlobalGeometry() {
  if (geometryDirty) {
    return;
  }
  geometryDirty = true;
  for (AccessibilityNode* child : children) {
    child->invalidateGlobalGeometry();
  }
}

SkMatrix44& SemanticsListenerImpl::AccessibilityNode::getGlobalTransform() {
  if (geometryDirty) {
    if (parent == NULL) {
      globalTransform = transform;
    } else {
      SkMatrix44 parentTransform = parent->getGlobalTransform();
      if (transform.isIdentity()) {
        globalTransform = parentTransform;
      } else if (parentTransform.isIdentity()) {
        globalTransform = transform;
      } else {
        globalTransform = transform * parentTransform;
      }
    }
  }
  return globalTransform;
}

CGRect SemanticsListenerImpl::AccessibilityNode::getGlobalRect() {
  if (geometryDirty) {
    // TODO(tvolkert): optimize identity transform
    SkMatrix44 globalTransform = getGlobalTransform();
    float point1[] = {left, top, 0, 1};
    float point2[] = {left + width, top, 0, 1};
    float point3[] = {left + width, top + height, 0, 1};
    float point4[] = {left, top + height, 0, 1};
    float* transformedPoint1 = transformPoint(globalTransform, point1);
    float* transformedPoint2 = transformPoint(globalTransform, point2);
    float* transformedPoint3 = transformPoint(globalTransform, point3);
    float* transformedPoint4 = transformPoint(globalTransform, point4);
    float minX = std::min({transformedPoint1[0], transformedPoint2[0],
                           transformedPoint3[0], transformedPoint4[0]});
    float minY = std::min({transformedPoint1[1], transformedPoint2[1],
                           transformedPoint3[1], transformedPoint4[1]});
    float maxX = std::max({transformedPoint1[0], transformedPoint2[0],
                           transformedPoint3[0], transformedPoint4[0]});
    float maxY = std::max({transformedPoint1[1], transformedPoint2[1],
                           transformedPoint3[1], transformedPoint4[1]});
    float width = maxX - minX;
    float height = maxY - minY;
    globalRect = CGRectMake(minX, minY, width, height);
    delete transformedPoint1;
    delete transformedPoint2;
    delete transformedPoint3;
    delete transformedPoint4;
  }
  return globalRect;
}

// TODO(tvolkert): call map2() with x, y, w, h all at once
float* SemanticsListenerImpl::AccessibilityNode::transformPoint(
    SkMatrix44& transform,
    float point[]) {
  float* transformed = (float*)malloc(4 * sizeof(float));
  if (transform.isIdentity()) {
    std::copy(point, point + 4, transformed);
  } else {
    transform.map2(point, 1, transformed);
  }
  return transformed;
}

void SemanticsListenerImpl::AccessibilityNode::populateAccessibleElements(
    NSMutableArray* accessibleElements) {
  if (width > 0 && height > 0) {
    UIAccessibilityElement* element = [[[UIAccessibilityElement alloc]
        initWithAccessibilityContainer:outer->_view] autorelease];
    element.isAccessibilityElement = YES;
    element.accessibilityFrame = getGlobalRect();
    if (canBeTapped) {
      // TODO(tvolkert): What about links? We need semantic info in the mojom
      // definition
      element.accessibilityTraits = UIAccessibilityTraitButton;
    }
    if (label) {
      element.accessibilityLabel = [NSString stringWithUTF8String:label];
    }
    [accessibleElements insertObject:element atIndex:0];
  }

  for (AccessibilityNode* child : children) {
    child->populateAccessibleElements(accessibleElements);
  }
}

SemanticsListenerImpl::AccessibilityNode::~AccessibilityNode() {
  if (label) {
    delete label;
    label = NULL;
  }
}
}  // namespace shell
}  // namespace sky
