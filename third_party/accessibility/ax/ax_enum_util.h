// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ACCESSIBILITY_AX_AX_ENUM_UTIL_H_
#define ACCESSIBILITY_AX_AX_ENUM_UTIL_H_

#include <string>

#include "ax_base_export.h"
#include "ax_enums.h"

namespace ax {

// ax::Event
AX_BASE_EXPORT const char* ToString(ax::Event event);
AX_BASE_EXPORT ax::Event ParseEvent(const char* event);

// ax::Role
AX_BASE_EXPORT const char* ToString(ax::Role role);
AX_BASE_EXPORT ax::Role ParseRole(const char* role);

// ax::State
AX_BASE_EXPORT const char* ToString(ax::State state);
AX_BASE_EXPORT ax::State ParseState(const char* state);

// ax::Action
AX_BASE_EXPORT const char* ToString(ax::Action action);
AX_BASE_EXPORT ax::Action ParseAction(const char* action);

// ax::ActionFlags
AX_BASE_EXPORT const char* ToString(ax::ActionFlags action_flags);
AX_BASE_EXPORT ax::ActionFlags ParseActionFlags(const char* action_flags);

// ax::DefaultActionVerb
AX_BASE_EXPORT const char* ToString(ax::DefaultActionVerb default_action_verb);

// Returns a localized string that corresponds to the name of the given action.
AX_BASE_EXPORT std::string ToLocalizedString(ax::DefaultActionVerb action_verb);

AX_BASE_EXPORT ax::DefaultActionVerb ParseDefaultActionVerb(
    const char* default_action_verb);

// ax::Mutation
AX_BASE_EXPORT const char* ToString(ax::Mutation mutation);
AX_BASE_EXPORT ax::Mutation ParseMutation(const char* mutation);

// ax::StringAttribute
AX_BASE_EXPORT const char* ToString(ax::StringAttribute string_attribute);
AX_BASE_EXPORT ax::StringAttribute ParseStringAttribute(
    const char* string_attribute);

// ax::IntAttribute
AX_BASE_EXPORT const char* ToString(ax::IntAttribute int_attribute);
AX_BASE_EXPORT ax::IntAttribute ParseIntAttribute(const char* int_attribute);

// ax::FloatAttribute
AX_BASE_EXPORT const char* ToString(ax::FloatAttribute float_attribute);
AX_BASE_EXPORT ax::FloatAttribute ParseFloatAttribute(
    const char* float_attribute);

// ax::BoolAttribute
AX_BASE_EXPORT const char* ToString(ax::BoolAttribute bool_attribute);
AX_BASE_EXPORT ax::BoolAttribute ParseBoolAttribute(const char* bool_attribute);

// ax::IntListAttribute
AX_BASE_EXPORT const char* ToString(ax::IntListAttribute int_list_attribute);
AX_BASE_EXPORT ax::IntListAttribute ParseIntListAttribute(
    const char* int_list_attribute);

// ax::StringListAttribute
AX_BASE_EXPORT const char* ToString(
    ax::StringListAttribute string_list_attribute);
AX_BASE_EXPORT ax::StringListAttribute ParseStringListAttribute(
    const char* string_list_attribute);

// ax::ListStyle
AX_BASE_EXPORT const char* ToString(ax::ListStyle list_style);
AX_BASE_EXPORT ax::ListStyle ParseListStyle(const char* list_style);

// ax::MarkerType
AX_BASE_EXPORT const char* ToString(ax::MarkerType marker_type);
AX_BASE_EXPORT ax::MarkerType ParseMarkerType(const char* marker_type);

// ax::MoveDirection
AX_BASE_EXPORT const char* ToString(ax::MoveDirection move_direction);
AX_BASE_EXPORT ax::MoveDirection ParseMoveDirection(const char* move_direction);

// ax::Command
AX_BASE_EXPORT const char* ToString(ax::Command command);
AX_BASE_EXPORT ax::Command ParseCommand(const char* command);

// ax::TextBoundary
AX_BASE_EXPORT const char* ToString(ax::TextBoundary text_boundary);
AX_BASE_EXPORT ax::TextBoundary ParseTextBoundary(const char* text_boundary);

// ax:mojom::TextDecorationStyle
AX_BASE_EXPORT const char* ToString(
    ax::TextDecorationStyle text_decoration_style);
AX_BASE_EXPORT ax::TextDecorationStyle ParseTextDecorationStyle(
    const char* text_decoration_style);

// ax::TextAlign
AX_BASE_EXPORT const char* ToString(ax::TextAlign text_align);
AX_BASE_EXPORT ax::TextAlign ParseTextAlign(const char* text_align);

// ax::WritingDirection
AX_BASE_EXPORT const char* ToString(ax::WritingDirection text_direction);
AX_BASE_EXPORT ax::WritingDirection ParseTextDirection(
    const char* text_direction);

// ax::TextPosition
AX_BASE_EXPORT const char* ToString(ax::TextPosition text_position);
AX_BASE_EXPORT ax::TextPosition ParseTextPosition(const char* text_position);

// ax::TextStyle
AX_BASE_EXPORT const char* ToString(ax::TextStyle text_style);
AX_BASE_EXPORT ax::TextStyle ParseTextStyle(const char* text_style);

// ax::AriaCurrentState
AX_BASE_EXPORT const char* ToString(ax::AriaCurrentState aria_current_state);
AX_BASE_EXPORT ax::AriaCurrentState ParseAriaCurrentState(
    const char* aria_current_state);

// ax::HasPopup
AX_BASE_EXPORT const char* ToString(ax::HasPopup has_popup);
AX_BASE_EXPORT ax::HasPopup ParseHasPopup(const char* has_popup);

// ax::InvalidState
AX_BASE_EXPORT const char* ToString(ax::InvalidState invalid_state);
AX_BASE_EXPORT ax::InvalidState ParseInvalidState(const char* invalid_state);

// ax::Restriction
AX_BASE_EXPORT const char* ToString(ax::Restriction restriction);
AX_BASE_EXPORT ax::Restriction ParseRestriction(const char* restriction);

// ax::CheckedState
AX_BASE_EXPORT const char* ToString(ax::CheckedState checked_state);
AX_BASE_EXPORT ax::CheckedState ParseCheckedState(const char* checked_state);

// ax::SortDirection
AX_BASE_EXPORT const char* ToString(ax::SortDirection sort_direction);
AX_BASE_EXPORT ax::SortDirection ParseSortDirection(const char* sort_direction);

// ax::NameFrom
AX_BASE_EXPORT const char* ToString(ax::NameFrom name_from);
AX_BASE_EXPORT ax::NameFrom ParseNameFrom(const char* name_from);

// ax::DescriptionFrom
AX_BASE_EXPORT const char* ToString(ax::DescriptionFrom description_from);
AX_BASE_EXPORT ax::DescriptionFrom ParseDescriptionFrom(
    const char* description_from);

// ax::EventFrom
AX_BASE_EXPORT const char* ToString(ax::EventFrom event_from);
AX_BASE_EXPORT ax::EventFrom ParseEventFrom(const char* event_from);

// ax::Gesture
AX_BASE_EXPORT const char* ToString(ax::Gesture gesture);
AX_BASE_EXPORT ax::Gesture ParseGesture(const char* gesture);

// ax::TextAffinity
AX_BASE_EXPORT const char* ToString(ax::TextAffinity text_affinity);
AX_BASE_EXPORT ax::TextAffinity ParseTextAffinity(const char* text_affinity);

// ax::TreeOrder
AX_BASE_EXPORT const char* ToString(ax::TreeOrder tree_order);
AX_BASE_EXPORT ax::TreeOrder ParseTreeOrder(const char* tree_order);

// ax::ImageAnnotationStatus
AX_BASE_EXPORT const char* ToString(ax::ImageAnnotationStatus status);
AX_BASE_EXPORT ax::ImageAnnotationStatus ParseImageAnnotationStatus(
    const char* status);

// ax::Dropeffect
AX_BASE_EXPORT const char* ToString(ax::Dropeffect dropeffect);
AX_BASE_EXPORT ax::Dropeffect ParseDropeffect(const char* dropeffect);

}  // namespace ax

#endif  // ACCESSIBILITY_AX_AX_ENUM_UTIL_H_
