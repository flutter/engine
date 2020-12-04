// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ACCESSIBILITY_AX_AX_NODE_DATA_H_
#define ACCESSIBILITY_AX_AX_NODE_DATA_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ax_base_export.h"
#include "ax_enums.h"
#include "ax_node_text_styles.h"
#include "ax_relative_bounds.h"

namespace ax {

// Return true if |attr| should be interpreted as the id of another node
// in the same tree.
AX_BASE_EXPORT bool IsNodeIdIntAttribute(ax::IntAttribute attr);

// Return true if |attr| should be interpreted as a list of ids of
// nodes in the same tree.
AX_BASE_EXPORT bool IsNodeIdIntListAttribute(ax::IntListAttribute attr);

// A compact representation of the accessibility information for a
// single accessible object, in a form that can be serialized and sent from
// one process to another.
struct AX_BASE_EXPORT AXNodeData {
  // Defines the type used for AXNode IDs.
  using AXID = int32_t;

  // If a node is not yet or no longer valid, its ID should have a value of
  // kInvalidAXID.
  static constexpr AXID kInvalidAXID = 0;

  AXNodeData();
  virtual ~AXNodeData();

  AXNodeData(const AXNodeData& other);
  AXNodeData(AXNodeData&& other);
  AXNodeData& operator=(AXNodeData other);

  // Accessing accessibility attributes:
  //
  // There are dozens of possible attributes for an accessibility node,
  // but only a few tend to apply to any one object, so we store them
  // in sparse arrays of <attribute id, attribute value> pairs, organized
  // by type (bool, int, float, string, int list).
  //
  // There are three accessors for each type of attribute: one that returns
  // true if the attribute is present and false if not, one that takes a
  // pointer argument and returns true if the attribute is present (if you
  // need to distinguish between the default value and a missing attribute),
  // and another that returns the default value for that type if the
  // attribute is not present. In addition, strings can be returned as
  // either std::string or std::u16string, for convenience.

  bool HasBoolAttribute(ax::BoolAttribute attribute) const;
  bool GetBoolAttribute(ax::BoolAttribute attribute) const;
  bool GetBoolAttribute(ax::BoolAttribute attribute, bool* value) const;

  bool HasFloatAttribute(ax::FloatAttribute attribute) const;
  float GetFloatAttribute(ax::FloatAttribute attribute) const;
  bool GetFloatAttribute(ax::FloatAttribute attribute, float* value) const;

  bool HasIntAttribute(ax::IntAttribute attribute) const;
  int GetIntAttribute(ax::IntAttribute attribute) const;
  bool GetIntAttribute(ax::IntAttribute attribute, int* value) const;

  bool HasStringAttribute(ax::StringAttribute attribute) const;
  const std::string& GetStringAttribute(ax::StringAttribute attribute) const;
  bool GetStringAttribute(ax::StringAttribute attribute,
                          std::string* value) const;

  bool GetString16Attribute(ax::StringAttribute attribute,
                            std::u16string* value) const;
  std::u16string GetString16Attribute(ax::StringAttribute attribute) const;

  bool HasIntListAttribute(ax::IntListAttribute attribute) const;
  const std::vector<int32_t>& GetIntListAttribute(
      ax::IntListAttribute attribute) const;
  bool GetIntListAttribute(ax::IntListAttribute attribute,
                           std::vector<int32_t>* value) const;

  bool HasStringListAttribute(ax::StringListAttribute attribute) const;
  const std::vector<std::string>& GetStringListAttribute(
      ax::StringListAttribute attribute) const;
  bool GetStringListAttribute(ax::StringListAttribute attribute,
                              std::vector<std::string>* value) const;

  bool GetHtmlAttribute(const char* attribute, std::string* value) const;

  //
  // Setting accessibility attributes.
  //
  // Replaces an attribute if present. This is safer than crashing via a
  // FML_DCHECK or doing nothing, because most likely replacing is what the
  // caller would have wanted or what existing code already assumes.
  //

  void AddStringAttribute(ax::StringAttribute attribute,
                          const std::string& value);
  void AddIntAttribute(ax::IntAttribute attribute, int32_t value);
  void AddFloatAttribute(ax::FloatAttribute attribute, float value);
  void AddBoolAttribute(ax::BoolAttribute attribute, bool value);
  void AddIntListAttribute(ax::IntListAttribute attribute,
                           const std::vector<int32_t>& value);
  void AddStringListAttribute(ax::StringListAttribute attribute,
                              const std::vector<std::string>& value);

  //
  // Removing accessibility attributes.
  //

  void RemoveStringAttribute(ax::StringAttribute attribute);
  void RemoveIntAttribute(ax::IntAttribute attribute);
  void RemoveFloatAttribute(ax::FloatAttribute attribute);
  void RemoveBoolAttribute(ax::BoolAttribute attribute);
  void RemoveIntListAttribute(ax::IntListAttribute attribute);
  void RemoveStringListAttribute(ax::StringListAttribute attribute);

  //
  // Text styles.
  //
  AXNodeTextStyles GetTextStyles() const;

  //
  // Convenience functions.
  //

  // Adds the name attribute or replaces it if already present. Also sets the
  // NameFrom attribute if not already set.
  void SetName(const std::string& name);

  // Allows nameless objects to pass accessibility checks.
  void SetNameExplicitlyEmpty();

  // Adds the description attribute or replaces it if already present.
  void SetDescription(const std::string& description);

  // Adds the value attribute or replaces it if already present.
  void SetValue(const std::string& value);

  // Returns true if the given enum bit is 1.
  bool HasState(ax::State state) const;
  bool HasAction(ax::Action action) const;
  bool HasTextStyle(ax::TextStyle text_style) const;
  // aria-dropeffect is deprecated in WAI-ARIA 1.1.
  bool HasDropeffect(ax::Dropeffect dropeffect) const;

  // Set or remove bits in the given enum's corresponding bitfield.
  void AddState(ax::State state);
  void RemoveState(ax::State state);
  void AddAction(ax::Action action);
  void AddTextStyle(ax::TextStyle text_style);
  // aria-dropeffect is deprecated in WAI-ARIA 1.1.
  void AddDropeffect(ax::Dropeffect dropeffect);

  // Helper functions to get or set some common int attributes with some
  // specific enum types. To remove an attribute, set it to None.
  //
  // Please keep in alphabetic order.
  ax::CheckedState GetCheckedState() const;
  void SetCheckedState(ax::CheckedState checked_state);
  bool HasCheckedState() const;
  ax::DefaultActionVerb GetDefaultActionVerb() const;
  void SetDefaultActionVerb(ax::DefaultActionVerb default_action_verb);
  ax::HasPopup GetHasPopup() const;
  void SetHasPopup(ax::HasPopup has_popup);
  ax::InvalidState GetInvalidState() const;
  void SetInvalidState(ax::InvalidState invalid_state);
  ax::NameFrom GetNameFrom() const;
  void SetNameFrom(ax::NameFrom name_from);
  ax::DescriptionFrom GetDescriptionFrom() const;
  void SetDescriptionFrom(ax::DescriptionFrom description_from);
  ax::TextPosition GetTextPosition() const;
  void SetTextPosition(ax::TextPosition text_position);
  ax::Restriction GetRestriction() const;
  void SetRestriction(ax::Restriction restriction);
  ax::ListStyle GetListStyle() const;
  void SetListStyle(ax::ListStyle list_style);
  ax::TextAlign GetTextAlign() const;
  void SetTextAlign(ax::TextAlign text_align);
  ax::WritingDirection GetTextDirection() const;
  void SetTextDirection(ax::WritingDirection text_direction);
  ax::ImageAnnotationStatus GetImageAnnotationStatus() const;
  void SetImageAnnotationStatus(ax::ImageAnnotationStatus status);

  // Helper to determine if the data belongs to a node that gains focus when
  // clicked, such as a text field or a native HTML list box.
  bool IsActivatable() const;

  // Helper to determine if the data belongs to a node that is a native button
  // or ARIA role="button" in a pressed state.
  bool IsButtonPressed() const;

  // Helper to determine if the data belongs to a node that can respond to
  // clicks.
  bool IsClickable() const;

  // Helper to determine if the object is selectable.
  bool IsSelectable() const;

  // Helper to determine if the data has the ignored state or ignored role.
  bool IsIgnored() const;

  // Helper to determine if the data has the ignored state, the invisible state
  // or the ignored role.
  bool IsInvisibleOrIgnored() const;

  // Helper to determine if the data belongs to a node that is invocable.
  bool IsInvisible() const;

  // Helper to determine if the data belongs to a node that is a menu button.
  bool IsMenuButton() const;

  // This data belongs to a text field. This is any widget in which the user
  // should be able to enter and edit text.
  //
  // Examples include <input type="text">, <input type="password">, <textarea>,
  // <div contenteditable="true">, <div role="textbox">, <div role="searchbox">
  // and <div role="combobox">. Note that when an ARIA role that indicates that
  // the widget is editable is used, such as "role=textbox", the element doesn't
  // need to be contenteditable for this method to return true, as in theory
  // JavaScript could be used to implement editing functionality. In practice,
  // this situation should be rare.
  bool IsTextField() const;

  // This data belongs to a text field that is used for entering passwords.
  bool IsPasswordField() const;

  // This data belongs to a text field that doesn't accept rich text content,
  // such as text with special formatting or styling.
  bool IsPlainTextField() const;

  // This data belongs to a text field that accepts rich text content, such as
  // text with special formatting or styling.
  bool IsRichTextField() const;

  // Helper to determine if |GetRestriction| is either ReadOnly or Disabled.
  // By default, all nodes that can't be edited are readonly.
  bool IsReadOnlyOrDisabled() const;

  // Helper to determine if the data belongs to a node that supports
  // range-based value.
  bool IsRangeValueSupported() const;

  // Helper to determine if the data belongs to a node that supports
  // expand/collapse.
  bool SupportsExpandCollapse() const;

  // Return a string representation of this data, for debugging.
  virtual std::string ToString() const;

  // Return a string representation of |aria-dropeffect| values, for testing
  // and debugging.
  // aria-dropeffect is deprecated in WAI-ARIA 1.1.
  std::string DropeffectBitfieldToString() const;

  // As much as possible this should behave as a simple, serializable,
  // copyable struct.
  int32_t id = -1;
  ax::Role role;
  uint32_t state;
  uint64_t actions;
  std::vector<std::pair<ax::StringAttribute, std::string>> string_attributes;
  std::vector<std::pair<ax::IntAttribute, int32_t>> int_attributes;
  std::vector<std::pair<ax::FloatAttribute, float>> float_attributes;
  std::vector<std::pair<ax::BoolAttribute, bool>> bool_attributes;
  std::vector<std::pair<ax::IntListAttribute, std::vector<int32_t>>>
      intlist_attributes;
  std::vector<std::pair<ax::StringListAttribute, std::vector<std::string>>>
      stringlist_attributes;
  std::vector<std::pair<std::string, std::string>> html_attributes;
  std::vector<int32_t> child_ids;

  AXRelativeBounds relative_bounds;
};

}  // namespace ax

#endif  // ACCESSIBILITY_AX_AX_NODE_DATA_H_
