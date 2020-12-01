// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ax_node_data.h"

#include <stddef.h>
#include <codecvt>
#include <algorithm>
#include <set>
#include <utility>

#include "flutter/fml/logging.h"

#include "base/no_destructor.h"
#include "ax_enum_util.h"
#include "ax_role_properties.h"

namespace ax {

namespace {

std::string ToUpperASCII(std::string str) {
  std::string ret;
  ret.reserve(str.size());
  for (size_t i = 0; i < str.size(); i++)
    ret.push_back(std::toupper(str[i]));
  return ret;
}

bool IsFlagSet(uint32_t bitfield, uint32_t flag) {
  return (bitfield & (1U << flag)) != 0;
}

bool IsFlagSet(uint64_t bitfield, uint32_t flag) {
  return (bitfield & (1ULL << flag)) != 0;
}

uint32_t ModifyFlag(uint32_t bitfield, uint32_t flag, bool set) {
  return set ? (bitfield |= (1U << flag)) : (bitfield &= ~(1U << flag));
}

uint64_t ModifyFlag(uint64_t bitfield, uint32_t flag, bool set) {
  return set ? (bitfield |= (1ULL << flag)) : (bitfield &= ~(1ULL << flag));
}

std::string StateBitfieldToString(uint32_t state_enum) {
  std::string str;
  for (uint32_t i = static_cast<uint32_t>(ax::State::kNone) + 1;
       i <= static_cast<uint32_t>(ax::State::kMaxValue); ++i) {
    if (IsFlagSet(state_enum, i))
      str += " " +
             ToUpperASCII(ax::ToString(static_cast<ax::State>(i)));
  }
  return str;
}

std::string ActionsBitfieldToString(uint64_t actions) {
  std::string str;
  for (uint32_t i = static_cast<uint32_t>(ax::Action::kNone) + 1;
       i <= static_cast<uint32_t>(ax::Action::kMaxValue); ++i) {
    if (IsFlagSet(actions, i)) {
      str += ax::ToString(static_cast<ax::Action>(i));
      actions = ModifyFlag(actions, i, false);
      str += actions ? "," : "";
    }
  }
  return str;
}

std::string IntVectorToString(const std::vector<int>& items) {
  std::string str;
  for (size_t i = 0; i < items.size(); ++i) {
    if (i > 0)
      str += ",";
    str += std::to_string(items[i]);
  }
  return str;
}

// Predicate that returns true if the first value of a pair is |first|.
template <typename FirstType, typename SecondType>
struct FirstIs {
  explicit FirstIs(FirstType first) : first_(first) {}
  bool operator()(std::pair<FirstType, SecondType> const& p) {
    return p.first == first_;
  }
  FirstType first_;
};

// Helper function that finds a key in a vector of pairs by matching on the
// first value, and returns an iterator.
template <typename FirstType, typename SecondType>
typename std::vector<std::pair<FirstType, SecondType>>::const_iterator
FindInVectorOfPairs(
    FirstType first,
    const std::vector<std::pair<FirstType, SecondType>>& vector) {
  return std::find_if(vector.begin(), vector.end(),
                      FirstIs<FirstType, SecondType>(first));
}

}  // namespace

// Return true if |attr| is a node ID that would need to be mapped when
// renumbering the ids in a combined tree.
bool IsNodeIdIntAttribute(ax::IntAttribute attr) {
  switch (attr) {
    case ax::IntAttribute::kActivedescendantId:
    case ax::IntAttribute::kErrormessageId:
    case ax::IntAttribute::kInPageLinkTargetId:
    case ax::IntAttribute::kMemberOfId:
    case ax::IntAttribute::kNextOnLineId:
    case ax::IntAttribute::kPopupForId:
    case ax::IntAttribute::kPreviousOnLineId:
    case ax::IntAttribute::kTableHeaderId:
    case ax::IntAttribute::kTableColumnHeaderId:
    case ax::IntAttribute::kTableRowHeaderId:
    case ax::IntAttribute::kNextFocusId:
    case ax::IntAttribute::kPreviousFocusId:
      return true;

    // Note: all of the attributes are included here explicitly,
    // rather than using "default:", so that it's a compiler error to
    // add a new attribute without explicitly considering whether it's
    // a node id attribute or not.
    case ax::IntAttribute::kNone:
    case ax::IntAttribute::kDefaultActionVerb:
    case ax::IntAttribute::kScrollX:
    case ax::IntAttribute::kScrollXMin:
    case ax::IntAttribute::kScrollXMax:
    case ax::IntAttribute::kScrollY:
    case ax::IntAttribute::kScrollYMin:
    case ax::IntAttribute::kScrollYMax:
    case ax::IntAttribute::kTextSelStart:
    case ax::IntAttribute::kTextSelEnd:
    case ax::IntAttribute::kTableRowCount:
    case ax::IntAttribute::kTableColumnCount:
    case ax::IntAttribute::kTableRowIndex:
    case ax::IntAttribute::kTableColumnIndex:
    case ax::IntAttribute::kTableCellColumnIndex:
    case ax::IntAttribute::kTableCellColumnSpan:
    case ax::IntAttribute::kTableCellRowIndex:
    case ax::IntAttribute::kTableCellRowSpan:
    case ax::IntAttribute::kSortDirection:
    case ax::IntAttribute::kHierarchicalLevel:
    case ax::IntAttribute::kNameFrom:
    case ax::IntAttribute::kDescriptionFrom:
    case ax::IntAttribute::kSetSize:
    case ax::IntAttribute::kPosInSet:
    case ax::IntAttribute::kColorValue:
    case ax::IntAttribute::kAriaCurrentState:
    case ax::IntAttribute::kHasPopup:
    case ax::IntAttribute::kBackgroundColor:
    case ax::IntAttribute::kColor:
    case ax::IntAttribute::kInvalidState:
    case ax::IntAttribute::kCheckedState:
    case ax::IntAttribute::kRestriction:
    case ax::IntAttribute::kListStyle:
    case ax::IntAttribute::kTextAlign:
    case ax::IntAttribute::kTextDirection:
    case ax::IntAttribute::kTextPosition:
    case ax::IntAttribute::kTextStyle:
    case ax::IntAttribute::kTextOverlineStyle:
    case ax::IntAttribute::kTextStrikethroughStyle:
    case ax::IntAttribute::kTextUnderlineStyle:
    case ax::IntAttribute::kAriaColumnCount:
    case ax::IntAttribute::kAriaCellColumnIndex:
    case ax::IntAttribute::kAriaCellColumnSpan:
    case ax::IntAttribute::kAriaRowCount:
    case ax::IntAttribute::kAriaCellRowIndex:
    case ax::IntAttribute::kAriaCellRowSpan:
    case ax::IntAttribute::kImageAnnotationStatus:
    case ax::IntAttribute::kDropeffect:
    case ax::IntAttribute::kDOMNodeId:
      return false;
  }

  FML_DCHECK(false);
  return false;
}

// Return true if |attr| contains a vector of node ids that would need
// to be mapped when renumbering the ids in a combined tree.
bool IsNodeIdIntListAttribute(ax::IntListAttribute attr) {
  switch (attr) {
    case ax::IntListAttribute::kControlsIds:
    case ax::IntListAttribute::kDetailsIds:
    case ax::IntListAttribute::kDescribedbyIds:
    case ax::IntListAttribute::kFlowtoIds:
    case ax::IntListAttribute::kIndirectChildIds:
    case ax::IntListAttribute::kLabelledbyIds:
    case ax::IntListAttribute::kRadioGroupIds:
      return true;

    // Note: all of the attributes are included here explicitly,
    // rather than using "default:", so that it's a compiler error to
    // add a new attribute without explicitly considering whether it's
    // a node id attribute or not.
    case ax::IntListAttribute::kNone:
    case ax::IntListAttribute::kMarkerTypes:
    case ax::IntListAttribute::kMarkerStarts:
    case ax::IntListAttribute::kMarkerEnds:
    case ax::IntListAttribute::kCharacterOffsets:
    case ax::IntListAttribute::kCachedLineStarts:
    case ax::IntListAttribute::kWordStarts:
    case ax::IntListAttribute::kWordEnds:
    case ax::IntListAttribute::kCustomActionIds:
      return false;
  }

  FML_DCHECK(false);
  return false;
}

AXNodeData::AXNodeData()
    : role(ax::Role::kUnknown), state(0U), actions(0ULL) {}

AXNodeData::~AXNodeData() = default;

AXNodeData::AXNodeData(const AXNodeData& other) {
  id = other.id;
  role = other.role;
  state = other.state;
  actions = other.actions;
  string_attributes = other.string_attributes;
  int_attributes = other.int_attributes;
  float_attributes = other.float_attributes;
  bool_attributes = other.bool_attributes;
  intlist_attributes = other.intlist_attributes;
  stringlist_attributes = other.stringlist_attributes;
  html_attributes = other.html_attributes;
  child_ids = other.child_ids;
  relative_bounds = other.relative_bounds;
}

AXNodeData::AXNodeData(AXNodeData&& other) {
  id = other.id;
  role = other.role;
  state = other.state;
  actions = other.actions;
  string_attributes.swap(other.string_attributes);
  int_attributes.swap(other.int_attributes);
  float_attributes.swap(other.float_attributes);
  bool_attributes.swap(other.bool_attributes);
  intlist_attributes.swap(other.intlist_attributes);
  stringlist_attributes.swap(other.stringlist_attributes);
  html_attributes.swap(other.html_attributes);
  child_ids.swap(other.child_ids);
  relative_bounds = other.relative_bounds;
}

AXNodeData& AXNodeData::operator=(AXNodeData other) {
  id = other.id;
  role = other.role;
  state = other.state;
  actions = other.actions;
  string_attributes = other.string_attributes;
  int_attributes = other.int_attributes;
  float_attributes = other.float_attributes;
  bool_attributes = other.bool_attributes;
  intlist_attributes = other.intlist_attributes;
  stringlist_attributes = other.stringlist_attributes;
  html_attributes = other.html_attributes;
  child_ids = other.child_ids;
  relative_bounds = other.relative_bounds;
  return *this;
}

bool AXNodeData::HasBoolAttribute(ax::BoolAttribute attribute) const {
  auto iter = FindInVectorOfPairs(attribute, bool_attributes);
  return iter != bool_attributes.end();
}

bool AXNodeData::GetBoolAttribute(ax::BoolAttribute attribute) const {
  bool result;
  if (GetBoolAttribute(attribute, &result))
    return result;
  return false;
}

bool AXNodeData::GetBoolAttribute(ax::BoolAttribute attribute,
                                  bool* value) const {
  auto iter = FindInVectorOfPairs(attribute, bool_attributes);
  if (iter != bool_attributes.end()) {
    *value = iter->second;
    return true;
  }

  return false;
}

bool AXNodeData::HasFloatAttribute(ax::FloatAttribute attribute) const {
  auto iter = FindInVectorOfPairs(attribute, float_attributes);
  return iter != float_attributes.end();
}

float AXNodeData::GetFloatAttribute(ax::FloatAttribute attribute) const {
  float result;
  if (GetFloatAttribute(attribute, &result))
    return result;
  return 0.0;
}

bool AXNodeData::GetFloatAttribute(ax::FloatAttribute attribute,
                                   float* value) const {
  auto iter = FindInVectorOfPairs(attribute, float_attributes);
  if (iter != float_attributes.end()) {
    *value = iter->second;
    return true;
  }

  return false;
}

bool AXNodeData::HasIntAttribute(ax::IntAttribute attribute) const {
  auto iter = FindInVectorOfPairs(attribute, int_attributes);
  return iter != int_attributes.end();
}

int AXNodeData::GetIntAttribute(ax::IntAttribute attribute) const {
  int result;
  if (GetIntAttribute(attribute, &result))
    return result;
  return 0;
}

bool AXNodeData::GetIntAttribute(ax::IntAttribute attribute,
                                 int* value) const {
  auto iter = FindInVectorOfPairs(attribute, int_attributes);
  if (iter != int_attributes.end()) {
    *value =  static_cast<int>(iter->second);
    return true;
  }

  return false;
}

bool AXNodeData::HasStringAttribute(
    ax::StringAttribute attribute) const {
  auto iter = FindInVectorOfPairs(attribute, string_attributes);
  return iter != string_attributes.end();
}

const std::string& AXNodeData::GetStringAttribute(
    ax::StringAttribute attribute) const {
  auto iter = FindInVectorOfPairs(attribute, string_attributes);
  if (iter != string_attributes.end()) {
    return iter->second;
  }
  static const base::NoDestructor<std::string> s;
  return *s;
}

bool AXNodeData::GetStringAttribute(ax::StringAttribute attribute,
                                    std::string* value) const {
  auto iter = FindInVectorOfPairs(attribute, string_attributes);
  if (iter != string_attributes.end()) {
    *value = iter->second;
    return true;
  }

  return false;
}

std::u16string AXNodeData::GetString16Attribute(
    ax::StringAttribute attribute) const {
  std::string value_utf8;
  if (!GetStringAttribute(attribute, &value_utf8))
    return std::u16string();
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert; 
  return convert.from_bytes(value_utf8);
}

bool AXNodeData::GetString16Attribute(ax::StringAttribute attribute,
                                      std::u16string* value) const {
  std::string value_utf8;
  if (!GetStringAttribute(attribute, &value_utf8))
    return false;
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert; 
  *value = convert.from_bytes(value_utf8);
  return true;
}

bool AXNodeData::HasIntListAttribute(
    ax::IntListAttribute attribute) const {
  auto iter = FindInVectorOfPairs(attribute, intlist_attributes);
  return iter != intlist_attributes.end();
}

const std::vector<int32_t>& AXNodeData::GetIntListAttribute(
    ax::IntListAttribute attribute) const {
  static const std::vector<int32_t> empty_vector;
  auto iter = FindInVectorOfPairs(attribute, intlist_attributes);
  if (iter != intlist_attributes.end())
    return iter->second;
  return empty_vector;
}

bool AXNodeData::GetIntListAttribute(ax::IntListAttribute attribute,
                                     std::vector<int32_t>* value) const {
  auto iter = FindInVectorOfPairs(attribute, intlist_attributes);
  if (iter != intlist_attributes.end()) {
    *value = iter->second;
    return true;
  }

  return false;
}

bool AXNodeData::HasStringListAttribute(
    ax::StringListAttribute attribute) const {
  auto iter = FindInVectorOfPairs(attribute, stringlist_attributes);
  return iter != stringlist_attributes.end();
}

const std::vector<std::string>& AXNodeData::GetStringListAttribute(
    ax::StringListAttribute attribute) const {
  static const std::vector<std::string> empty_vector;
  auto iter = FindInVectorOfPairs(attribute, stringlist_attributes);
  if (iter != stringlist_attributes.end())
    return iter->second;
  return empty_vector;
}

bool AXNodeData::GetStringListAttribute(
    ax::StringListAttribute attribute,
    std::vector<std::string>* value) const {
  auto iter = FindInVectorOfPairs(attribute, stringlist_attributes);
  if (iter != stringlist_attributes.end()) {
    *value = iter->second;
    return true;
  }

  return false;
}

bool AXNodeData::GetHtmlAttribute(const char* html_attr,
                                  std::string* value) const {
  for (const std::pair<std::string, std::string>& html_attribute :
       html_attributes) {
    const std::string& attr = html_attribute.first;
    if (attr.compare(html_attr)) {
      *value = html_attribute.second;
      return true;
    }
  }

  return false;
}

void AXNodeData::AddStringAttribute(ax::StringAttribute attribute,
                                    const std::string& value) {
  FML_DCHECK(attribute != ax::StringAttribute::kNone);
  if (HasStringAttribute(attribute))
    RemoveStringAttribute(attribute);
  string_attributes.push_back(std::make_pair(attribute, value));
}

void AXNodeData::AddIntAttribute(ax::IntAttribute attribute, int value) {
  FML_DCHECK(attribute != ax::IntAttribute::kNone);
  if (HasIntAttribute(attribute))
    RemoveIntAttribute(attribute);
  int_attributes.push_back(std::make_pair(attribute, value));
}

void AXNodeData::AddFloatAttribute(ax::FloatAttribute attribute,
                                   float value) {
  FML_DCHECK(attribute != ax::FloatAttribute::kNone);
  if (HasFloatAttribute(attribute))
    RemoveFloatAttribute(attribute);
  float_attributes.push_back(std::make_pair(attribute, value));
}

void AXNodeData::AddBoolAttribute(ax::BoolAttribute attribute,
                                  bool value) {
  FML_DCHECK(attribute != ax::BoolAttribute::kNone);
  if (HasBoolAttribute(attribute))
    RemoveBoolAttribute(attribute);
  bool_attributes.push_back(std::make_pair(attribute, value));
}

void AXNodeData::AddIntListAttribute(ax::IntListAttribute attribute,
                                     const std::vector<int32_t>& value) {
  FML_DCHECK(attribute != ax::IntListAttribute::kNone);
  if (HasIntListAttribute(attribute))
    RemoveIntListAttribute(attribute);
  intlist_attributes.push_back(std::make_pair(attribute, value));
}

void AXNodeData::AddStringListAttribute(
    ax::StringListAttribute attribute,
    const std::vector<std::string>& value) {
  FML_DCHECK(attribute != ax::StringListAttribute::kNone);
  if (HasStringListAttribute(attribute))
    RemoveStringListAttribute(attribute);
  stringlist_attributes.push_back(std::make_pair(attribute, value));
}

void AXNodeData::RemoveStringAttribute(ax::StringAttribute attribute) {
  FML_DCHECK(attribute != ax::StringAttribute::kNone);
  string_attributes.erase(
    std::remove_if(string_attributes.begin(), string_attributes.end(),
    [attribute](const auto& string_attribute) {
      return string_attribute.first == attribute;
    }),
    string_attributes.end()
  );
}

void AXNodeData::RemoveIntAttribute(ax::IntAttribute attribute) {
  FML_DCHECK(attribute != ax::IntAttribute::kNone);
  int_attributes.erase(
    std::remove_if(int_attributes.begin(), int_attributes.end(),
    [attribute](const auto& int_attribute) {
      return int_attribute.first == attribute;
    }),
    int_attributes.end()
  );
}

void AXNodeData::RemoveFloatAttribute(ax::FloatAttribute attribute) {
  FML_DCHECK(attribute != ax::FloatAttribute::kNone);
  float_attributes.erase(
    std::remove_if(float_attributes.begin(), float_attributes.end(),
    [attribute](const auto& float_attribute) {
      return float_attribute.first == attribute;
    }),
    float_attributes.end()
  );
}

void AXNodeData::RemoveBoolAttribute(ax::BoolAttribute attribute) {
  FML_DCHECK(attribute != ax::BoolAttribute::kNone);
  bool_attributes.erase(
    std::remove_if(bool_attributes.begin(), bool_attributes.end(),
    [attribute](const auto& bool_attribute) {
      return bool_attribute.first == attribute;
    }),
    bool_attributes.end()
  );
}

void AXNodeData::RemoveIntListAttribute(ax::IntListAttribute attribute) {
  FML_DCHECK(attribute != ax::IntListAttribute::kNone);
  intlist_attributes.erase(
    std::remove_if(intlist_attributes.begin(), intlist_attributes.end(),
    [attribute](const auto& intlist_attribute) {
      return intlist_attribute.first == attribute;
    }),
    intlist_attributes.end()
  );
}

void AXNodeData::RemoveStringListAttribute(
    ax::StringListAttribute attribute) {
  FML_DCHECK(attribute != ax::StringListAttribute::kNone);
  stringlist_attributes.erase(
    std::remove_if(stringlist_attributes.begin(), stringlist_attributes.end(),
    [attribute](const auto& stringlist_attribute) {
      return stringlist_attribute.first == attribute;
    }),
    stringlist_attributes.end()
  );
}

AXNodeTextStyles AXNodeData::GetTextStyles() const {
  AXNodeTextStyles style_attributes;

  GetIntAttribute(ax::IntAttribute::kBackgroundColor,
                  &style_attributes.background_color);
  GetIntAttribute(ax::IntAttribute::kColor, &style_attributes.color);
  GetIntAttribute(ax::IntAttribute::kInvalidState,
                  &style_attributes.invalid_state);
  GetIntAttribute(ax::IntAttribute::kTextOverlineStyle,
                  &style_attributes.overline_style);
  GetIntAttribute(ax::IntAttribute::kTextDirection,
                  &style_attributes.text_direction);
  GetIntAttribute(ax::IntAttribute::kTextPosition,
                  &style_attributes.text_position);
  GetIntAttribute(ax::IntAttribute::kTextStrikethroughStyle,
                  &style_attributes.strikethrough_style);
  GetIntAttribute(ax::IntAttribute::kTextStyle,
                  &style_attributes.text_style);
  GetIntAttribute(ax::IntAttribute::kTextUnderlineStyle,
                  &style_attributes.underline_style);
  GetFloatAttribute(ax::FloatAttribute::kFontSize,
                    &style_attributes.font_size);
  GetFloatAttribute(ax::FloatAttribute::kFontWeight,
                    &style_attributes.font_weight);
  GetStringAttribute(ax::StringAttribute::kFontFamily,
                     &style_attributes.font_family);

  return style_attributes;
}

void AXNodeData::SetName(const std::string& name) {
  if (role == ax::Role::kNone) {
    FML_LOG(ERROR) << "A valid role is required before setting the name attribute, because "
      "the role is used for setting the required NameFrom attribute.";
    FML_DCHECK(false);
  }

  auto iter = std::find_if(string_attributes.begin(), string_attributes.end(),
                           [](const auto& string_attribute) {
                             return string_attribute.first ==
                                    ax::StringAttribute::kName;
                           });

  if (iter == string_attributes.end()) {
    string_attributes.push_back(
        std::make_pair(ax::StringAttribute::kName, name));
  } else {
    iter->second = name;
  }

  if (HasIntAttribute(ax::IntAttribute::kNameFrom))
    return;
  // Since this method is mostly used by tests which don't always set the
  // "NameFrom" attribute, we need to set it here to the most likely value if
  // not set, otherwise code that tries to calculate the node's inner text, its
  // hypertext, or even its value, might not know whether to include the name in
  // the result or not.
  //
  // For example, if there is a text field, but it is empty, i.e. it has no
  // value, its value could be its name if "NameFrom" is set to "kPlaceholder"
  // or to "kContents" but not if it's set to "kAttribute". Similarly, if there
  // is a button without any unignored children, it's name can only be
  // equivalent to its inner text if "NameFrom" is set to "kContents" or to
  // "kValue", but not if it is set to "kAttribute".
  if (IsText(role)) {
    SetNameFrom(ax::NameFrom::kContents);
  } else {
    SetNameFrom(ax::NameFrom::kAttribute);
  }
}

void AXNodeData::SetNameExplicitlyEmpty() {
  SetNameFrom(ax::NameFrom::kAttributeExplicitlyEmpty);
}

void AXNodeData::SetDescription(const std::string& description) {
  AddStringAttribute(ax::StringAttribute::kDescription, description);
}

void AXNodeData::SetValue(const std::string& value) {
  AddStringAttribute(ax::StringAttribute::kValue, value);
}

bool AXNodeData::HasState(ax::State state_enum) const {
  return IsFlagSet(state, static_cast<uint32_t>(state_enum));
}

bool AXNodeData::HasAction(ax::Action action) const {
  return IsFlagSet(actions, static_cast<uint32_t>(action));
}

bool AXNodeData::HasTextStyle(ax::TextStyle text_style_enum) const {
  int32_t style = GetIntAttribute(ax::IntAttribute::kTextStyle);
  return IsFlagSet(static_cast<uint32_t>(style),
                   static_cast<uint32_t>(text_style_enum));
}

bool AXNodeData::HasDropeffect(ax::Dropeffect dropeffect_enum) const {
  int32_t dropeffect = GetIntAttribute(ax::IntAttribute::kDropeffect);
  return IsFlagSet(static_cast<uint32_t>(dropeffect),
                   static_cast<uint32_t>(dropeffect_enum));
}

void AXNodeData::AddState(ax::State state_enum) {
  FML_DCHECK(static_cast<int>(state_enum) >
            static_cast<int>(ax::State::kNone));
  FML_DCHECK(static_cast<int>(state_enum) <=
            static_cast<int>(ax::State::kMaxValue));
  state = ModifyFlag(state, static_cast<uint32_t>(state_enum), true);
}

void AXNodeData::RemoveState(ax::State state_enum) {
  FML_DCHECK(static_cast<int>(state_enum) >
            static_cast<int>(ax::State::kNone));
  FML_DCHECK(static_cast<int>(state_enum) <=
            static_cast<int>(ax::State::kMaxValue));
  state = ModifyFlag(state, static_cast<uint32_t>(state_enum), false);
}

void AXNodeData::AddAction(ax::Action action_enum) {
  switch (action_enum) {
    case ax::Action::kNone:
      FML_DCHECK(false);
      break;

    // Note: all of the attributes are included here explicitly, rather than
    // using "default:", so that it's a compiler error to add a new action
    // without explicitly considering whether there are mutually exclusive
    // actions that can be performed on a UI control at the same time.
    case ax::Action::kBlur:
    case ax::Action::kFocus: {
      ax::Action excluded_action =
          (action_enum == ax::Action::kBlur) ? ax::Action::kFocus
                                                    : ax::Action::kBlur;
      FML_DCHECK(!HasAction(excluded_action));
      break;
    }

    case ax::Action::kClearAccessibilityFocus:
    case ax::Action::kCollapse:
    case ax::Action::kCustomAction:
    case ax::Action::kDecrement:
    case ax::Action::kDoDefault:
    case ax::Action::kExpand:
    case ax::Action::kGetImageData:
    case ax::Action::kHitTest:
    case ax::Action::kIncrement:
    case ax::Action::kInternalInvalidateTree:
    case ax::Action::kLoadInlineTextBoxes:
    case ax::Action::kReplaceSelectedText:
    case ax::Action::kScrollToMakeVisible:
    case ax::Action::kScrollToPoint:
    case ax::Action::kSetAccessibilityFocus:
    case ax::Action::kSetScrollOffset:
    case ax::Action::kSetSelection:
    case ax::Action::kSetSequentialFocusNavigationStartingPoint:
    case ax::Action::kSetValue:
    case ax::Action::kShowContextMenu:
    case ax::Action::kScrollBackward:
    case ax::Action::kScrollForward:
    case ax::Action::kScrollUp:
    case ax::Action::kScrollDown:
    case ax::Action::kScrollLeft:
    case ax::Action::kScrollRight:
    case ax::Action::kGetTextLocation:
    case ax::Action::kAnnotatePageImages:
    case ax::Action::kSignalEndOfTest:
    case ax::Action::kHideTooltip:
    case ax::Action::kShowTooltip:
      break;
    case ax::Action::kMaxValue:
      FML_DCHECK(false);
      break;
  }

  actions = ModifyFlag(actions, static_cast<uint32_t>(action_enum), true);
}

void AXNodeData::AddTextStyle(ax::TextStyle text_style_enum) {
  FML_DCHECK(static_cast<int>(text_style_enum) >=
             static_cast<int>(ax::TextStyle::kMinValue));
  FML_DCHECK(static_cast<int>(text_style_enum) <=
             static_cast<int>(ax::TextStyle::kMaxValue));
  int32_t style = GetIntAttribute(ax::IntAttribute::kTextStyle);
  style = ModifyFlag(static_cast<uint32_t>(style),
                     static_cast<uint32_t>(text_style_enum), true);
  RemoveIntAttribute(ax::IntAttribute::kTextStyle);
  AddIntAttribute(ax::IntAttribute::kTextStyle, style);
}

void AXNodeData::AddDropeffect(ax::Dropeffect dropeffect_enum) {
  FML_DCHECK(static_cast<int>(dropeffect_enum) >=
             static_cast<int>(ax::Dropeffect::kMinValue));
  FML_DCHECK(static_cast<int>(dropeffect_enum) <=
             static_cast<int>(ax::Dropeffect::kMaxValue));
  int32_t dropeffect = GetIntAttribute(ax::IntAttribute::kDropeffect);
  dropeffect = ModifyFlag(static_cast<uint32_t>(dropeffect),
                          static_cast<uint32_t>(dropeffect_enum), true);
  RemoveIntAttribute(ax::IntAttribute::kDropeffect);
  AddIntAttribute(ax::IntAttribute::kDropeffect, dropeffect);
}

ax::CheckedState AXNodeData::GetCheckedState() const {
  return static_cast<ax::CheckedState>(
      GetIntAttribute(ax::IntAttribute::kCheckedState));
}

void AXNodeData::SetCheckedState(ax::CheckedState checked_state) {
  if (HasCheckedState())
    RemoveIntAttribute(ax::IntAttribute::kCheckedState);
  if (checked_state != ax::CheckedState::kNone) {
    AddIntAttribute(ax::IntAttribute::kCheckedState,
                    static_cast<int32_t>(checked_state));
  }
}

bool AXNodeData::HasCheckedState() const {
  return HasIntAttribute(ax::IntAttribute::kCheckedState);
}

ax::DefaultActionVerb AXNodeData::GetDefaultActionVerb() const {
  return static_cast<ax::DefaultActionVerb>(
      GetIntAttribute(ax::IntAttribute::kDefaultActionVerb));
}

void AXNodeData::SetDefaultActionVerb(
    ax::DefaultActionVerb default_action_verb) {
  if (HasIntAttribute(ax::IntAttribute::kDefaultActionVerb))
    RemoveIntAttribute(ax::IntAttribute::kDefaultActionVerb);
  if (default_action_verb != ax::DefaultActionVerb::kNone) {
    AddIntAttribute(ax::IntAttribute::kDefaultActionVerb,
                    static_cast<int32_t>(default_action_verb));
  }
}

ax::HasPopup AXNodeData::GetHasPopup() const {
  return static_cast<ax::HasPopup>(
      GetIntAttribute(ax::IntAttribute::kHasPopup));
}

void AXNodeData::SetHasPopup(ax::HasPopup has_popup) {
  if (HasIntAttribute(ax::IntAttribute::kHasPopup))
    RemoveIntAttribute(ax::IntAttribute::kHasPopup);
  if (has_popup != ax::HasPopup::kFalse) {
    AddIntAttribute(ax::IntAttribute::kHasPopup,
                    static_cast<int32_t>(has_popup));
  }
}

ax::InvalidState AXNodeData::GetInvalidState() const {
  return static_cast<ax::InvalidState>(
      GetIntAttribute(ax::IntAttribute::kInvalidState));
}

void AXNodeData::SetInvalidState(ax::InvalidState invalid_state) {
  if (HasIntAttribute(ax::IntAttribute::kInvalidState))
    RemoveIntAttribute(ax::IntAttribute::kInvalidState);
  if (invalid_state != ax::InvalidState::kNone) {
    AddIntAttribute(ax::IntAttribute::kInvalidState,
                    static_cast<int32_t>(invalid_state));
  }
}

ax::NameFrom AXNodeData::GetNameFrom() const {
  return static_cast<ax::NameFrom>(
      GetIntAttribute(ax::IntAttribute::kNameFrom));
}

void AXNodeData::SetNameFrom(ax::NameFrom name_from) {
  if (HasIntAttribute(ax::IntAttribute::kNameFrom))
    RemoveIntAttribute(ax::IntAttribute::kNameFrom);
  if (name_from != ax::NameFrom::kNone) {
    AddIntAttribute(ax::IntAttribute::kNameFrom,
                    static_cast<int32_t>(name_from));
  }
}

ax::DescriptionFrom AXNodeData::GetDescriptionFrom() const {
  return static_cast<ax::DescriptionFrom>(
      GetIntAttribute(ax::IntAttribute::kDescriptionFrom));
}

void AXNodeData::SetDescriptionFrom(
    ax::DescriptionFrom description_from) {
  if (HasIntAttribute(ax::IntAttribute::kDescriptionFrom))
    RemoveIntAttribute(ax::IntAttribute::kDescriptionFrom);
  if (description_from != ax::DescriptionFrom::kNone) {
    AddIntAttribute(ax::IntAttribute::kDescriptionFrom,
                    static_cast<int32_t>(description_from));
  }
}

ax::TextPosition AXNodeData::GetTextPosition() const {
  return static_cast<ax::TextPosition>(
      GetIntAttribute(ax::IntAttribute::kTextPosition));
}

void AXNodeData::SetTextPosition(ax::TextPosition text_position) {
  if (HasIntAttribute(ax::IntAttribute::kTextPosition))
    RemoveIntAttribute(ax::IntAttribute::kTextPosition);
  if (text_position != ax::TextPosition::kNone) {
    AddIntAttribute(ax::IntAttribute::kTextPosition,
                    static_cast<int32_t>(text_position));
  }
}

ax::ImageAnnotationStatus AXNodeData::GetImageAnnotationStatus() const {
  return static_cast<ax::ImageAnnotationStatus>(
      GetIntAttribute(ax::IntAttribute::kImageAnnotationStatus));
}

void AXNodeData::SetImageAnnotationStatus(
    ax::ImageAnnotationStatus status) {
  if (HasIntAttribute(ax::IntAttribute::kImageAnnotationStatus))
    RemoveIntAttribute(ax::IntAttribute::kImageAnnotationStatus);
  if (status != ax::ImageAnnotationStatus::kNone) {
    AddIntAttribute(ax::IntAttribute::kImageAnnotationStatus,
                    static_cast<int32_t>(status));
  }
}

ax::Restriction AXNodeData::GetRestriction() const {
  return static_cast<ax::Restriction>(
      GetIntAttribute(ax::IntAttribute::kRestriction));
}

void AXNodeData::SetRestriction(ax::Restriction restriction) {
  if (HasIntAttribute(ax::IntAttribute::kRestriction))
    RemoveIntAttribute(ax::IntAttribute::kRestriction);
  if (restriction != ax::Restriction::kNone) {
    AddIntAttribute(ax::IntAttribute::kRestriction,
                    static_cast<int32_t>(restriction));
  }
}

ax::ListStyle AXNodeData::GetListStyle() const {
  return static_cast<ax::ListStyle>(
      GetIntAttribute(ax::IntAttribute::kListStyle));
}

void AXNodeData::SetListStyle(ax::ListStyle list_style) {
  if (HasIntAttribute(ax::IntAttribute::kListStyle))
    RemoveIntAttribute(ax::IntAttribute::kListStyle);
  if (list_style != ax::ListStyle::kNone) {
    AddIntAttribute(ax::IntAttribute::kListStyle,
                    static_cast<int32_t>(list_style));
  }
}

ax::TextAlign AXNodeData::GetTextAlign() const {
  return static_cast<ax::TextAlign>(
      GetIntAttribute(ax::IntAttribute::kTextAlign));
}

void AXNodeData::SetTextAlign(ax::TextAlign text_align) {
  if (HasIntAttribute(ax::IntAttribute::kTextAlign))
    RemoveIntAttribute(ax::IntAttribute::kTextAlign);
  AddIntAttribute(ax::IntAttribute::kTextAlign,
                  static_cast<int32_t>(text_align));
}

ax::WritingDirection AXNodeData::GetTextDirection() const {
  return static_cast<ax::WritingDirection>(
      GetIntAttribute(ax::IntAttribute::kTextDirection));
}

void AXNodeData::SetTextDirection(ax::WritingDirection text_direction) {
  if (HasIntAttribute(ax::IntAttribute::kTextDirection))
    RemoveIntAttribute(ax::IntAttribute::kTextDirection);
  if (text_direction != ax::WritingDirection::kNone) {
    AddIntAttribute(ax::IntAttribute::kTextDirection,
                    static_cast<int32_t>(text_direction));
  }
}

bool AXNodeData::IsActivatable() const {
  return IsTextField() || role == ax::Role::kListBox;
}

bool AXNodeData::IsButtonPressed() const {
  // Currently there is no internal representation for |aria-pressed|, and
  // we map |aria-pressed="true"| to ax::CheckedState::kTrue for a native
  // button or role="button".
  // https://www.w3.org/TR/wai-aria-1.1/#aria-pressed
  if (IsButton(role) && GetCheckedState() == ax::CheckedState::kTrue)
    return true;
  return false;
}

bool AXNodeData::IsClickable() const {
  // If it has a custom default action verb except for
  // ax::DefaultActionVerb::kClickAncestor, it's definitely clickable.
  // ax::DefaultActionVerb::kClickAncestor is used when an element with a
  // click listener is present in its ancestry chain.
  if (HasIntAttribute(ax::IntAttribute::kDefaultActionVerb) &&
      (GetDefaultActionVerb() != ax::DefaultActionVerb::kClickAncestor))
    return true;

  return ax::IsClickable(role);
}

bool AXNodeData::IsSelectable() const {
  // It's selectable if it has the attribute, whether it's true or false.
  return HasBoolAttribute(ax::BoolAttribute::kSelected) &&
         GetRestriction() != ax::Restriction::kDisabled;
}

bool AXNodeData::IsIgnored() const {
  return HasState(ax::State::kIgnored) ||
         role == ax::Role::kIgnored;
}

bool AXNodeData::IsInvisibleOrIgnored() const {
  return IsIgnored() || HasState(ax::State::kInvisible);
}

bool AXNodeData::IsInvisible() const {
  // A control is "invocable" if it initiates an action when activated but
  // does not maintain any state. A control that maintains state when activated
  // would be considered a toggle or expand-collapse element - these elements
  // are "clickable" but not "invocable". Similarly, if the action only involves
  // activating the control, such as when clicking a text field, the control is
  // not considered "invocable".
  return IsClickable() && !IsActivatable() && !SupportsExpandCollapse() &&
         !SupportsToggle(role);
}

bool AXNodeData::IsMenuButton() const {
  // According to the WAI-ARIA spec, a menu button is a native button or an ARIA
  // role="button" that opens a menu. Although ARIA does not include a role
  // specifically for menu buttons, screen readers identify buttons that have
  // aria-haspopup="true" or aria-haspopup="menu" as menu buttons, and Blink
  // maps both to HasPopup::kMenu.
  // https://www.w3.org/TR/wai-aria-practices/#menubutton
  // https://www.w3.org/TR/wai-aria-1.1/#aria-haspopup
  if (IsButton(role) && GetHasPopup() == ax::HasPopup::kMenu)
    return true;

  return false;
}

bool AXNodeData::IsTextField() const {
  return IsPlainTextField() || IsRichTextField();
}

bool AXNodeData::IsPasswordField() const {
  return IsTextField() && HasState(ax::State::kProtected);
}

bool AXNodeData::IsPlainTextField() const {
  // We need to check both the role and editable state, because some ARIA text
  // fields may in fact not be editable, whilst some editable fields might not
  // have the role.
  return !HasState(ax::State::kRichlyEditable) &&
         (role == ax::Role::kTextField ||
          role == ax::Role::kTextFieldWithComboBox ||
          role == ax::Role::kSearchBox ||
          GetBoolAttribute(ax::BoolAttribute::kEditableRoot));
}

bool AXNodeData::IsRichTextField() const {
  return GetBoolAttribute(ax::BoolAttribute::kEditableRoot) &&
         HasState(ax::State::kRichlyEditable);
}

bool AXNodeData::IsReadOnlyOrDisabled() const {
  switch (GetRestriction()) {
    case ax::Restriction::kReadOnly:
    case ax::Restriction::kDisabled:
      return true;
    case ax::Restriction::kNone: {
      if (HasState(ax::State::kEditable) ||
          HasState(ax::State::kRichlyEditable)) {
        return false;
      }

      // By default, when readonly is not supported, we assume the node is never
      // editable - then always readonly.
      return ShouldHaveReadonlyStateByDefault(role) ||
             !IsReadOnlySupported(role);
    }
  }
}

bool AXNodeData::IsRangeValueSupported() const {
  if (role == ax::Role::kSplitter) {
    // According to the ARIA spec, role="separator" acts as a splitter only
    // when focusable, and supports a range only in that case.
    return HasState(ax::State::kFocusable);
  }
  return ax::IsRangeValueSupported(role);
}

bool AXNodeData::SupportsExpandCollapse() const {
  if (GetHasPopup() != ax::HasPopup::kFalse ||
      HasState(ax::State::kExpanded) ||
      HasState(ax::State::kCollapsed))
    return true;

  return ax::SupportsExpandCollapse(role);
}

std::string AXNodeData::ToString() const {
  std::string result;

  result += "id=" + std::to_string(id);
  result += " ";
  result += ax::ToString(role);

  result += StateBitfieldToString(state);

  result += " " + relative_bounds.ToString();

  for (const std::pair<ax::IntAttribute, int32_t>& int_attribute :
       int_attributes) {
    std::string value = std::to_string(int_attribute.second);
    switch (int_attribute.first) {
      case ax::IntAttribute::kDefaultActionVerb:
        result += std::string(" action=") +
                  ax::ToString(static_cast<ax::DefaultActionVerb>(
                      int_attribute.second));
        break;
      case ax::IntAttribute::kScrollX:
        result += " scroll_x=" + value;
        break;
      case ax::IntAttribute::kScrollXMin:
        result += " scroll_x_min=" + value;
        break;
      case ax::IntAttribute::kScrollXMax:
        result += " scroll_x_max=" + value;
        break;
      case ax::IntAttribute::kScrollY:
        result += " scroll_y=" + value;
        break;
      case ax::IntAttribute::kScrollYMin:
        result += " scroll_y_min=" + value;
        break;
      case ax::IntAttribute::kScrollYMax:
        result += " scroll_y_max=" + value;
        break;
      case ax::IntAttribute::kHierarchicalLevel:
        result += " level=" + value;
        break;
      case ax::IntAttribute::kTextSelStart:
        result += " sel_start=" + value;
        break;
      case ax::IntAttribute::kTextSelEnd:
        result += " sel_end=" + value;
        break;
      case ax::IntAttribute::kAriaColumnCount:
        result += " aria_column_count=" + value;
        break;
      case ax::IntAttribute::kAriaCellColumnIndex:
        result += " aria_cell_column_index=" + value;
        break;
      case ax::IntAttribute::kAriaCellColumnSpan:
        result += " aria_cell_column_span=" + value;
        break;
      case ax::IntAttribute::kAriaRowCount:
        result += " aria_row_count=" + value;
        break;
      case ax::IntAttribute::kAriaCellRowIndex:
        result += " aria_cell_row_index=" + value;
        break;
      case ax::IntAttribute::kAriaCellRowSpan:
        result += " aria_cell_row_span=" + value;
        break;
      case ax::IntAttribute::kTableRowCount:
        result += " rows=" + value;
        break;
      case ax::IntAttribute::kTableColumnCount:
        result += " cols=" + value;
        break;
      case ax::IntAttribute::kTableCellColumnIndex:
        result += " col=" + value;
        break;
      case ax::IntAttribute::kTableCellRowIndex:
        result += " row=" + value;
        break;
      case ax::IntAttribute::kTableCellColumnSpan:
        result += " colspan=" + value;
        break;
      case ax::IntAttribute::kTableCellRowSpan:
        result += " rowspan=" + value;
        break;
      case ax::IntAttribute::kTableColumnHeaderId:
        result += " column_header_id=" + value;
        break;
      case ax::IntAttribute::kTableColumnIndex:
        result += " column_index=" + value;
        break;
      case ax::IntAttribute::kTableHeaderId:
        result += " header_id=" + value;
        break;
      case ax::IntAttribute::kTableRowHeaderId:
        result += " row_header_id=" + value;
        break;
      case ax::IntAttribute::kTableRowIndex:
        result += " row_index=" + value;
        break;
      case ax::IntAttribute::kSortDirection:
        switch (static_cast<ax::SortDirection>(int_attribute.second)) {
          case ax::SortDirection::kUnsorted:
            result += " sort_direction=none";
            break;
          case ax::SortDirection::kAscending:
            result += " sort_direction=ascending";
            break;
          case ax::SortDirection::kDescending:
            result += " sort_direction=descending";
            break;
          case ax::SortDirection::kOther:
            result += " sort_direction=other";
            break;
          default:
            break;
        }
        break;
      case ax::IntAttribute::kNameFrom:
        result += " name_from=";
        result += ax::ToString(
            static_cast<ax::NameFrom>(int_attribute.second));
        break;
      case ax::IntAttribute::kDescriptionFrom:
        result += " description_from=";
        result += ax::ToString(
            static_cast<ax::DescriptionFrom>(int_attribute.second));
        break;
      case ax::IntAttribute::kActivedescendantId:
        result += " activedescendant=" + value;
        break;
      case ax::IntAttribute::kErrormessageId:
        result += " errormessage=" + value;
        break;
      case ax::IntAttribute::kInPageLinkTargetId:
        result += " in_page_link_target_id=" + value;
        break;
      case ax::IntAttribute::kMemberOfId:
        result += " member_of_id=" + value;
        break;
      case ax::IntAttribute::kNextOnLineId:
        result += " next_on_line_id=" + value;
        break;
      case ax::IntAttribute::kPopupForId:
        result += " popup_for_id=" + value;
        break;
      case ax::IntAttribute::kPreviousOnLineId:
        result += " previous_on_line_id=" + value;
        break;
      case ax::IntAttribute::kColorValue:
        result += " color_value=&" + std::to_string(int_attribute.second);
        break;
      case ax::IntAttribute::kAriaCurrentState:
        switch (
            static_cast<ax::AriaCurrentState>(int_attribute.second)) {
          case ax::AriaCurrentState::kFalse:
            result += " aria_current_state=false";
            break;
          case ax::AriaCurrentState::kTrue:
            result += " aria_current_state=true";
            break;
          case ax::AriaCurrentState::kPage:
            result += " aria_current_state=page";
            break;
          case ax::AriaCurrentState::kStep:
            result += " aria_current_state=step";
            break;
          case ax::AriaCurrentState::kLocation:
            result += " aria_current_state=location";
            break;
          case ax::AriaCurrentState::kDate:
            result += " aria_current_state=date";
            break;
          case ax::AriaCurrentState::kTime:
            result += " aria_current_state=time";
            break;
          default:
            break;
        }
        break;
      case ax::IntAttribute::kBackgroundColor:
        result += " background_color=&" + std::to_string(int_attribute.second);
        break;
      case ax::IntAttribute::kColor:
        result += " color=&" + std::to_string(int_attribute.second);
        break;
      case ax::IntAttribute::kListStyle:
        switch (static_cast<ax::ListStyle>(int_attribute.second)) {
          case ax::ListStyle::kCircle:
            result += " list_style=circle";
            break;
          case ax::ListStyle::kDisc:
            result += " list_style=disc";
            break;
          case ax::ListStyle::kImage:
            result += " list_style=image";
            break;
          case ax::ListStyle::kNumeric:
            result += " list_style=numeric";
            break;
          case ax::ListStyle::kOther:
            result += " list_style=other";
            break;
          case ax::ListStyle::kSquare:
            result += " list_style=square";
            break;
          default:
            break;
        }
        break;
      case ax::IntAttribute::kTextAlign:
        result += " text_align=";
        result += ax::ToString(
            static_cast<ax::TextAlign>(int_attribute.second));
        break;
      case ax::IntAttribute::kTextDirection:
        switch (
            static_cast<ax::WritingDirection>(int_attribute.second)) {
          case ax::WritingDirection::kLtr:
            result += " text_direction=ltr";
            break;
          case ax::WritingDirection::kRtl:
            result += " text_direction=rtl";
            break;
          case ax::WritingDirection::kTtb:
            result += " text_direction=ttb";
            break;
          case ax::WritingDirection::kBtt:
            result += " text_direction=btt";
            break;
          default:
            break;
        }
        break;
      case ax::IntAttribute::kTextPosition:
        switch (static_cast<ax::TextPosition>(int_attribute.second)) {
          case ax::TextPosition::kNone:
            result += " text_position=none";
            break;
          case ax::TextPosition::kSubscript:
            result += " text_position=subscript";
            break;
          case ax::TextPosition::kSuperscript:
            result += " text_position=superscript";
            break;
          default:
            break;
        }
        break;
      case ax::IntAttribute::kTextStyle: {
        std::string text_style_value;
        if (HasTextStyle(ax::TextStyle::kBold))
          text_style_value += "bold,";
        if (HasTextStyle(ax::TextStyle::kItalic))
          text_style_value += "italic,";
        if (HasTextStyle(ax::TextStyle::kUnderline))
          text_style_value += "underline,";
        if (HasTextStyle(ax::TextStyle::kLineThrough))
          text_style_value += "line-through,";
        if (HasTextStyle(ax::TextStyle::kOverline))
          text_style_value += "overline,";
        result += text_style_value.substr(0, text_style_value.size() - 1);
        break;
      }
      case ax::IntAttribute::kTextOverlineStyle:
        result += std::string(" text_overline_style=") +
                  ax::ToString(static_cast<ax::TextDecorationStyle>(
                      int_attribute.second));
        break;
      case ax::IntAttribute::kTextStrikethroughStyle:
        result += std::string(" text_strikethrough_style=") +
                  ax::ToString(static_cast<ax::TextDecorationStyle>(
                      int_attribute.second));
        break;
      case ax::IntAttribute::kTextUnderlineStyle:
        result += std::string(" text_underline_style=") +
                  ax::ToString(static_cast<ax::TextDecorationStyle>(
                      int_attribute.second));
        break;
      case ax::IntAttribute::kSetSize:
        result += " setsize=" + value;
        break;
      case ax::IntAttribute::kPosInSet:
        result += " posinset=" + value;
        break;
      case ax::IntAttribute::kHasPopup:
        switch (static_cast<ax::HasPopup>(int_attribute.second)) {
          case ax::HasPopup::kTrue:
            result += " haspopup=true";
            break;
          case ax::HasPopup::kMenu:
            result += " haspopup=menu";
            break;
          case ax::HasPopup::kListbox:
            result += " haspopup=listbox";
            break;
          case ax::HasPopup::kTree:
            result += " haspopup=tree";
            break;
          case ax::HasPopup::kGrid:
            result += " haspopup=grid";
            break;
          case ax::HasPopup::kDialog:
            result += " haspopup=dialog";
            break;
          case ax::HasPopup::kFalse:
          default:
            break;
        }
        break;
      case ax::IntAttribute::kInvalidState:
        switch (static_cast<ax::InvalidState>(int_attribute.second)) {
          case ax::InvalidState::kFalse:
            result += " invalid_state=false";
            break;
          case ax::InvalidState::kTrue:
            result += " invalid_state=true";
            break;
          case ax::InvalidState::kOther:
            result += " invalid_state=other";
            break;
          default:
            break;
        }
        break;
      case ax::IntAttribute::kCheckedState:
        switch (static_cast<ax::CheckedState>(int_attribute.second)) {
          case ax::CheckedState::kFalse:
            result += " checked_state=false";
            break;
          case ax::CheckedState::kTrue:
            result += " checked_state=true";
            break;
          case ax::CheckedState::kMixed:
            result += " checked_state=mixed";
            break;
          default:
            break;
        }
        break;
      case ax::IntAttribute::kRestriction:
        switch (static_cast<ax::Restriction>(int_attribute.second)) {
          case ax::Restriction::kReadOnly:
            result += " restriction=readonly";
            break;
          case ax::Restriction::kDisabled:
            result += " restriction=disabled";
            break;
          default:
            break;
        }
        break;
      case ax::IntAttribute::kNextFocusId:
        result += " next_focus_id=" + value;
        break;
      case ax::IntAttribute::kPreviousFocusId:
        result += " previous_focus_id=" + value;
        break;
      case ax::IntAttribute::kImageAnnotationStatus:
        result += std::string(" image_annotation_status=") +
                  ax::ToString(static_cast<ax::ImageAnnotationStatus>(
                      int_attribute.second));
        break;
      case ax::IntAttribute::kDropeffect:
        result += " dropeffect=" + value;
        break;
      case ax::IntAttribute::kDOMNodeId:
        result += " dom_node_id=" + value;
        break;
      case ax::IntAttribute::kNone:
        break;
    }
  }

  for (const std::pair<ax::StringAttribute, std::string>&
           string_attribute : string_attributes) {
    std::string value = string_attribute.second;
    switch (string_attribute.first) {
      case ax::StringAttribute::kAccessKey:
        result += " access_key=" + value;
        break;
      case ax::StringAttribute::kAriaInvalidValue:
        result += " aria_invalid_value=" + value;
        break;
      case ax::StringAttribute::kAutoComplete:
        result += " autocomplete=" + value;
        break;
      case ax::StringAttribute::kChildTreeId:
        result += " child_tree_id=" + value.substr(0, 8);
        break;
      case ax::StringAttribute::kClassName:
        result += " class_name=" + value;
        break;
      case ax::StringAttribute::kDescription:
        result += " description=" + value;
        break;
      case ax::StringAttribute::kDisplay:
        result += " display=" + value;
        break;
      case ax::StringAttribute::kFontFamily:
        result += " font-family=" + value;
        break;
      case ax::StringAttribute::kHtmlTag:
        result += " html_tag=" + value;
        break;
      case ax::StringAttribute::kImageAnnotation:
        result += " image_annotation=" + value;
        break;
      case ax::StringAttribute::kImageDataUrl:
        result += " image_data_url=(" +
                  std::to_string(static_cast<int>(value.size())) +
                  " bytes)";
        break;
      case ax::StringAttribute::kInnerHtml:
        result += " inner_html=" + value;
        break;
      case ax::StringAttribute::kInputType:
        result += " input_type=" + value;
        break;
      case ax::StringAttribute::kKeyShortcuts:
        result += " key_shortcuts=" + value;
        break;
      case ax::StringAttribute::kLanguage:
        result += " language=" + value;
        break;
      case ax::StringAttribute::kLiveRelevant:
        result += " relevant=" + value;
        break;
      case ax::StringAttribute::kLiveStatus:
        result += " live=" + value;
        break;
      case ax::StringAttribute::kContainerLiveRelevant:
        result += " container_relevant=" + value;
        break;
      case ax::StringAttribute::kContainerLiveStatus:
        result += " container_live=" + value;
        break;
      case ax::StringAttribute::kPlaceholder:
        result += " placeholder=" + value;
        break;
      case ax::StringAttribute::kRole:
        result += " role=" + value;
        break;
      case ax::StringAttribute::kRoleDescription:
        result += " role_description=" + value;
        break;
      case ax::StringAttribute::kTooltip:
        result += " tooltip=" + value;
        break;
      case ax::StringAttribute::kUrl:
        result += " url=" + value;
        break;
      case ax::StringAttribute::kName:
        result += " name=" + value;
        break;
      case ax::StringAttribute::kValue:
        result += " value=" + value;
        break;
      case ax::StringAttribute::kNone:
        break;
    }
  }

  for (const std::pair<ax::FloatAttribute, float>& float_attribute :
       float_attributes) {
    std::string value = std::to_string(float_attribute.second);
    switch (float_attribute.first) {
      case ax::FloatAttribute::kValueForRange:
        result += " value_for_range=" + value;
        break;
      case ax::FloatAttribute::kMaxValueForRange:
        result += " max_value=" + value;
        break;
      case ax::FloatAttribute::kMinValueForRange:
        result += " min_value=" + value;
        break;
      case ax::FloatAttribute::kStepValueForRange:
        result += " step_value=" + value;
        break;
      case ax::FloatAttribute::kFontSize:
        result += " font_size=" + value;
        break;
      case ax::FloatAttribute::kFontWeight:
        result += " font_weight=" + value;
        break;
      case ax::FloatAttribute::kTextIndent:
        result += " text_indent=" + value;
        break;
      case ax::FloatAttribute::kNone:
        break;
    }
  }

  for (const std::pair<ax::BoolAttribute, bool>& bool_attribute :
       bool_attributes) {
    std::string value = bool_attribute.second ? "true" : "false";
    switch (bool_attribute.first) {
      case ax::BoolAttribute::kEditableRoot:
        result += " editable_root=" + value;
        break;
      case ax::BoolAttribute::kLiveAtomic:
        result += " atomic=" + value;
        break;
      case ax::BoolAttribute::kBusy:
        result += " busy=" + value;
        break;
      case ax::BoolAttribute::kContainerLiveAtomic:
        result += " container_atomic=" + value;
        break;
      case ax::BoolAttribute::kContainerLiveBusy:
        result += " container_busy=" + value;
        break;
      case ax::BoolAttribute::kUpdateLocationOnly:
        result += " update_location_only=" + value;
        break;
      case ax::BoolAttribute::kCanvasHasFallback:
        result += " has_fallback=" + value;
        break;
      case ax::BoolAttribute::kModal:
        result += " modal=" + value;
        break;
      case ax::BoolAttribute::kScrollable:
        result += " scrollable=" + value;
        break;
      case ax::BoolAttribute::kClickable:
        result += " clickable=" + value;
        break;
      case ax::BoolAttribute::kClipsChildren:
        result += " clips_children=" + value;
        break;
      case ax::BoolAttribute::kNotUserSelectableStyle:
        result += " not_user_selectable=" + value;
        break;
      case ax::BoolAttribute::kSelected:
        result += " selected=" + value;
        break;
      case ax::BoolAttribute::kSelectedFromFocus:
        result += " selected_from_focus=" + value;
        break;
      case ax::BoolAttribute::kSupportsTextLocation:
        result += " supports_text_location=" + value;
        break;
      case ax::BoolAttribute::kGrabbed:
        result += " grabbed=" + value;
        break;
      case ax::BoolAttribute::kIsLineBreakingObject:
        result += " is_line_breaking_object=" + value;
        break;
      case ax::BoolAttribute::kIsPageBreakingObject:
        result += " is_page_breaking_object=" + value;
        break;
      case ax::BoolAttribute::kHasAriaAttribute:
        result += " has_aria_attribute=" + value;
        break;
      case ax::BoolAttribute::kNone:
        break;
    }
  }

  for (const std::pair<ax::IntListAttribute, std::vector<int32_t>>&
           intlist_attribute : intlist_attributes) {
    const std::vector<int32_t>& values = intlist_attribute.second;
    switch (intlist_attribute.first) {
      case ax::IntListAttribute::kIndirectChildIds:
        result += " indirect_child_ids=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kControlsIds:
        result += " controls_ids=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kDescribedbyIds:
        result += " describedby_ids=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kDetailsIds:
        result += " details_ids=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kFlowtoIds:
        result += " flowto_ids=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kLabelledbyIds:
        result += " labelledby_ids=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kRadioGroupIds:
        result += " radio_group_ids=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kMarkerTypes: {
        std::string types_str;
        for (size_t i = 0; i < values.size(); ++i) {
          int32_t type = values[i];
          if (type == static_cast<int32_t>(ax::MarkerType::kNone))
            continue;

          if (i > 0)
            types_str += ',';

          if (type & static_cast<int32_t>(ax::MarkerType::kSpelling))
            types_str += "spelling&";
          if (type & static_cast<int32_t>(ax::MarkerType::kGrammar))
            types_str += "grammar&";
          if (type & static_cast<int32_t>(ax::MarkerType::kTextMatch))
            types_str += "text_match&";
          if (type &
              static_cast<int32_t>(ax::MarkerType::kActiveSuggestion))
            types_str += "active_suggestion&";
          if (type & static_cast<int32_t>(ax::MarkerType::kSuggestion))
            types_str += "suggestion&";

          if (!types_str.empty())
            types_str = types_str.substr(0, types_str.size() - 1);
        }

        if (!types_str.empty())
          result += " marker_types=" + types_str;
        break;
      }
      case ax::IntListAttribute::kMarkerStarts:
        result += " marker_starts=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kMarkerEnds:
        result += " marker_ends=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kCharacterOffsets:
        result += " character_offsets=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kCachedLineStarts:
        result += " cached_line_start_offsets=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kWordStarts:
        result += " word_starts=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kWordEnds:
        result += " word_ends=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kCustomActionIds:
        result += " custom_action_ids=" + IntVectorToString(values);
        break;
      case ax::IntListAttribute::kNone:
        break;
    }
  }

  for (const std::pair<ax::StringListAttribute,
                       std::vector<std::string>>& stringlist_attribute :
       stringlist_attributes) {
    const std::vector<std::string>& values = stringlist_attribute.second;
    switch (stringlist_attribute.first) {
      case ax::StringListAttribute::kNone:
        break;
      case ax::StringListAttribute::kCustomActionDescriptions:
        const char* const delim = ",";
        std::ostringstream imploded;
        std::copy(values.begin(), values.end(),
           std::ostream_iterator<std::string>(imploded, delim));
        result +=
            " custom_action_descriptions: " + imploded.str();
        break;
    }
  }

  if (actions)
    result += " actions=" + ActionsBitfieldToString(actions);

  if (!child_ids.empty())
    result += " child_ids=" + IntVectorToString(child_ids);

  return result;
}

std::string AXNodeData::DropeffectBitfieldToString() const {
  if (!HasIntAttribute(ax::IntAttribute::kDropeffect))
    return "";

  std::string str;
  for (int dropeffect_idx = static_cast<int>(ax::Dropeffect::kMinValue);
       dropeffect_idx <= static_cast<int>(ax::Dropeffect::kMaxValue);
       ++dropeffect_idx) {
    ax::Dropeffect dropeffect_enum =
        static_cast<ax::Dropeffect>(dropeffect_idx);
    if (HasDropeffect(dropeffect_enum))
      str += " " + std::string(ax::ToString(dropeffect_enum));
  }

  // Removing leading space in final string.
  return str.substr(1);
}

}  // namespace ax
