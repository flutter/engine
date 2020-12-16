// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ax_enum_util.h"

#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "ax_enums.h"
#include "ax_node_data.h"

namespace ax {

// Templatized function that tests that for a mojom enum
// such as ax::Role, ax::Event, etc. we can
// call ToString() on the enum to get a string, and then
// ParseEnumName() on the string to get back the original
// value. Also tests what happens when we call ToString
// or ParseEnumName on a bogus value.
template <typename T>
void TestEnumStringConversion(
    T(ParseFunction)(const char*),
    int32_t(step)(int32_t) = [](int32_t val) { return val + 1; }) {
  // Check every valid enum value.
  for (int i = static_cast<int>(T::kMinValue);
       i <= static_cast<int>(T::kMaxValue); i = step(i)) {
    T src = static_cast<T>(i);
    std::string str = ToString(src);
    auto dst = ParseFunction(str.c_str());
    EXPECT_EQ(src, dst);
  }

  // Parse a bogus string.
  EXPECT_EQ(T::kNone, ParseFunction("bogus"));

  // Convert a bogus value to a string.
  int out_of_range_value = static_cast<int>(T::kMaxValue) + 1;
  EXPECT_STREQ("", ToString(static_cast<T>(out_of_range_value)));
}

// Templatized function that tries calling a setter on AXNodeData
// such as AddIntAttribute, AddFloatAttribute - with each possible
// enum value.
//
// This variant is for cases where the value type is an object.
template <typename T, typename U>
void TestAXNodeDataSetter(void (AXNodeData::*Setter)(T, const U&),
                          const U& value) {
  AXNodeData node_data;
  for (int i = static_cast<int>(T::kMinValue) + 1;
       i <= static_cast<int>(T::kMaxValue); ++i) {
    T attr = static_cast<T>(i);
    ((node_data).*(Setter))(attr, value);
  }
  EXPECT_TRUE(!node_data.ToString().empty());
}

// Same as TextAXNodeData, above, but This variant is for
// cases where the value type is POD.
template <typename T, typename U>
void TestAXNodeDataSetter(void (AXNodeData::*Setter)(T, U), U value) {
  AXNodeData node_data;
  for (int i = static_cast<int>(T::kMinValue) + 1;
       i <= static_cast<int>(T::kMaxValue); ++i) {
    T attr = static_cast<T>(i);
    ((node_data).*(Setter))(attr, value);
  }
  EXPECT_TRUE(!node_data.ToString().empty());
}

TEST(AXEnumUtilTest, Event) {
  TestEnumStringConversion<ax::Event>(ParseEvent);
}

TEST(AXEnumUtilTest, Role) {
  TestEnumStringConversion<ax::Role>(ParseRole);
}

TEST(AXEnumUtilTest, State) {
  TestEnumStringConversion<ax::State>(ParseState);
}

TEST(AXEnumUtilTest, Action) {
  TestEnumStringConversion<ax::Action>(ParseAction);
}

TEST(AXEnumUtilTest, ActionFlags) {
  TestEnumStringConversion<ax::ActionFlags>(ParseActionFlags);
}

TEST(AXEnumUtilTest, DefaultActionVerb) {
  TestEnumStringConversion<ax::DefaultActionVerb>(ParseDefaultActionVerb);
}

TEST(AXEnumUtilTest, Mutation) {
  TestEnumStringConversion<ax::Mutation>(ParseMutation);
}

TEST(AXEnumUtilTest, StringAttribute) {
  TestEnumStringConversion<ax::StringAttribute>(ParseStringAttribute);
  TestAXNodeDataSetter<ax::StringAttribute>(&AXNodeData::AddStringAttribute,
                                            std::string());
}

TEST(AXEnumUtilTest, IntAttribute) {
  TestEnumStringConversion<ax::IntAttribute>(ParseIntAttribute);
  TestAXNodeDataSetter<ax::IntAttribute>(&AXNodeData::AddIntAttribute, 0);
}

TEST(AXEnumUtilTest, FloatAttribute) {
  TestEnumStringConversion<ax::FloatAttribute>(ParseFloatAttribute);
  TestAXNodeDataSetter<ax::FloatAttribute>(&AXNodeData::AddFloatAttribute,
                                           0.0f);
}

TEST(AXEnumUtilTest, BoolAttribute) {
  TestEnumStringConversion<ax::BoolAttribute>(ParseBoolAttribute);
  TestAXNodeDataSetter<ax::BoolAttribute>(&AXNodeData::AddBoolAttribute, false);
}

TEST(AXEnumUtilTest, IntListAttribute) {
  TestEnumStringConversion<ax::IntListAttribute>(ParseIntListAttribute);
  TestAXNodeDataSetter<ax::IntListAttribute>(&AXNodeData::AddIntListAttribute,
                                             std::vector<int32_t>());
}

TEST(AXEnumUtilTest, StringListAttribute) {
  TestEnumStringConversion<ax::StringListAttribute>(ParseStringListAttribute);
  TestAXNodeDataSetter<ax::StringListAttribute>(
      &AXNodeData::AddStringListAttribute, std::vector<std::string>());
}

TEST(AXEnumUtilTest, MarkerType) {
  TestEnumStringConversion<ax::MarkerType>(ParseMarkerType, [](int32_t val) {
    return val == 0 ? 1 :
                    // 8 (Composition) is
                    // explicitly skipped in
                    // ax_enums.mojom.
               val == 4 ? 16 : val * 2;
  });
}

TEST(AXEnumUtilTest, Text_Decoration_Style) {
  TestEnumStringConversion<ax::TextDecorationStyle>(ParseTextDecorationStyle);
}

TEST(AXEnumUtilTest, ListStyle) {
  TestEnumStringConversion<ax::ListStyle>(ParseListStyle);
}

TEST(AXEnumUtilTest, MoveDirection) {
  TestEnumStringConversion<ax::MoveDirection>(ParseMoveDirection);
}

TEST(AXEnumUtilTest, Command) {
  TestEnumStringConversion<ax::Command>(ParseCommand);
}

TEST(AXEnumUtilTest, TextAlign) {
  TestEnumStringConversion<ax::TextAlign>(ParseTextAlign);
}

TEST(AXEnumUtilTest, TextBoundary) {
  TestEnumStringConversion<ax::TextBoundary>(ParseTextBoundary);
}

TEST(AXEnumUtilTest, TextDirection) {
  TestEnumStringConversion<ax::WritingDirection>(ParseTextDirection);
}

TEST(AXEnumUtilTest, TextPosition) {
  TestEnumStringConversion<ax::TextPosition>(ParseTextPosition);
}

TEST(AXEnumUtilTest, TextStyle) {
  TestEnumStringConversion<ax::TextStyle>(ParseTextStyle);
}

TEST(AXEnumUtilTest, AriaCurrentState) {
  TestEnumStringConversion<ax::AriaCurrentState>(ParseAriaCurrentState);
}

TEST(AXEnumUtilTest, HasPopup) {
  TestEnumStringConversion<ax::HasPopup>(ParseHasPopup);
}

TEST(AXEnumUtilTest, InvalidState) {
  TestEnumStringConversion<ax::InvalidState>(ParseInvalidState);
}

TEST(AXEnumUtilTest, Restriction) {
  TestEnumStringConversion<ax::Restriction>(ParseRestriction);
}

TEST(AXEnumUtilTest, CheckedState) {
  TestEnumStringConversion<ax::CheckedState>(ParseCheckedState);
}

TEST(AXEnumUtilTest, SortDirection) {
  TestEnumStringConversion<ax::SortDirection>(ParseSortDirection);
}

TEST(AXEnumUtilTest, NameFrom) {
  TestEnumStringConversion<ax::NameFrom>(ParseNameFrom);
}

TEST(AXEnumUtilTest, DescriptionFrom) {
  TestEnumStringConversion<ax::DescriptionFrom>(ParseDescriptionFrom);
}

TEST(AXEnumUtilTest, EventFrom) {
  TestEnumStringConversion<ax::EventFrom>(ParseEventFrom);
}

TEST(AXEnumUtilTest, Gesture) {
  TestEnumStringConversion<ax::Gesture>(ParseGesture);
}

TEST(AXEnumUtilTest, TextAffinity) {
  TestEnumStringConversion<ax::TextAffinity>(ParseTextAffinity);
}

TEST(AXEnumUtilTest, TreeOrder) {
  TestEnumStringConversion<ax::TreeOrder>(ParseTreeOrder);
}

TEST(AXEnumUtilTest, ImageAnnotationStatus) {
  TestEnumStringConversion<ax::ImageAnnotationStatus>(
      ParseImageAnnotationStatus);
}

TEST(AXEnumUtilTest, Dropeffect) {
  TestEnumStringConversion<ax::Dropeffect>(ParseDropeffect);
}

}  // namespace ax
