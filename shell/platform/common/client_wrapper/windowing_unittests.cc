// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/common/client_wrapper/include/flutter/windowing.h"

#include "gtest/gtest.h"

namespace flutter {

using Positioner = WindowPositioner;
using Anchor = Positioner::Anchor;
using Constraint = Positioner::ConstraintAdjustment;
using Rectangle = WindowRectangle;
using Point = WindowPoint;
using Size = WindowSize;

std::ostream& operator<<(std::ostream& os, Point const& point) {
  return os << "(x: " << point.x << ", y: " << point.y << ")";
}

std::ostream& operator<<(std::ostream& os, Size const& size) {
  return os << "(width: " << size.width << ", height: " << size.height << ")";
}

std::ostream& operator<<(std::ostream& os, Rectangle const& rect) {
  return os << "(x: " << rect.top_left.x << ", y: " << rect.top_left.y
            << ", width: " << rect.size.width
            << ", height: " << rect.size.height << ")";
}

namespace {

struct WindowPlacementTest : testing::Test {
  struct ClientAnchorsToParentConfig {
    Rectangle const display_area{{0, 0}, {800, 600}};
    Size const parent_size{400, 300};
    Size const child_size{100, 50};
    Point const parent_position{
        (display_area.size.width - parent_size.width) / 2,
        (display_area.size.height - parent_size.height) / 2};
  } client_anchors_to_parent_config;

  Rectangle const display_area{{0, 0}, {640, 480}};
  Size const parent_size{600, 400};
  Size const child_size{300, 300};
  Rectangle const rectangle_away_from_rhs{{20, 20}, {20, 20}};
  Rectangle const rectangle_near_rhs{{590, 20}, {10, 20}};
  Rectangle const rectangle_away_from_bottom{{20, 20}, {20, 20}};
  Rectangle const rectangle_near_bottom{{20, 380}, {20, 20}};
  Rectangle const rectangle_near_both_sides{{0, 20}, {600, 20}};
  Rectangle const rectangle_near_both_sides_and_bottom{{0, 380}, {600, 20}};
  Rectangle const rectangle_near_all_sides{{0, 20}, {600, 380}};
  Rectangle const rectangle_near_both_bottom_right{{400, 380}, {200, 20}};
  Point const parent_position{
      (display_area.size.width - parent_size.width) / 2,
      (display_area.size.height - parent_size.height) / 2};

  Positioner positioner;

  auto anchor_rect() -> Rectangle {
    auto rectangle{positioner.anchor_rect.value()};
    return {rectangle.top_left + parent_position, rectangle.size};
  }

  auto parent_rect() -> Rectangle { return {parent_position, parent_size}; }

  auto on_top_edge() -> Point {
    return anchor_rect().top_left - Point{0, child_size.height};
  }

  auto on_right_edge() -> Point {
    auto const rect{anchor_rect()};
    return rect.top_left + Point{rect.size.width, 0};
  }

  auto on_left_edge() -> Point {
    return anchor_rect().top_left - Point{child_size.width, 0};
  }

  auto on_bottom_edge() -> Point {
    auto const rect{anchor_rect()};
    return rect.top_left + Point{0, rect.size.height};
  }
};

}  // namespace

TEST_F(WindowPlacementTest, ClientAnchorsToParentGivenRectAnchorRightOfParent) {
  auto const& display_area{client_anchors_to_parent_config.display_area};
  auto const& parent_size{client_anchors_to_parent_config.parent_size};
  auto const& child_size{client_anchors_to_parent_config.child_size};
  auto const& parent_position{client_anchors_to_parent_config.parent_position};

  auto const rect_size{10};
  Rectangle const overlapping_right{
      parent_position +
          Point{parent_size.width - rect_size / 2, parent_size.height / 2},
      {rect_size, rect_size}};

  Positioner const positioner{.anchor_rect = overlapping_right,
                              .parent_anchor = Anchor::top_right,
                              .child_anchor = Anchor::top_left,
                              .constraint_adjustment = static_cast<Constraint>(
                                  static_cast<int>(Constraint::slide_y) |
                                  static_cast<int>(Constraint::resize_x))};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, positioner.anchor_rect.value(),
      {parent_position, parent_size}, display_area)};

  auto const expected_position{
      parent_position + Point{parent_size.width, parent_size.height / 2}};

  EXPECT_EQ(child_rect.top_left, expected_position);
  EXPECT_EQ(child_rect.size, child_size);
}

TEST_F(WindowPlacementTest, ClientAnchorsToParentGivenRectAnchorAboveParent) {
  auto const& display_area{client_anchors_to_parent_config.display_area};
  auto const& parent_size{client_anchors_to_parent_config.parent_size};
  auto const& child_size{client_anchors_to_parent_config.child_size};
  auto const& parent_position{client_anchors_to_parent_config.parent_position};

  auto const rect_size{10};
  Rectangle const overlapping_above{
      parent_position + Point{parent_size.width / 2, -rect_size / 2},
      {rect_size, rect_size}};

  Positioner const positioner{.anchor_rect = overlapping_above,
                              .parent_anchor = Anchor::top_right,
                              .child_anchor = Anchor::bottom_right,
                              .constraint_adjustment = Constraint::slide_x};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, positioner.anchor_rect.value(),
      {parent_position, parent_size}, display_area)};

  auto const expected_position{parent_position +
                               Point{parent_size.width / 2 + rect_size, 0} -
                               static_cast<Point>(child_size)};

  EXPECT_EQ(child_rect.top_left, expected_position);
  EXPECT_EQ(child_rect.size, child_size);
}

TEST_F(WindowPlacementTest, ClientAnchorsToParentGivenOffsetRightOfParent) {
  auto const& display_area{client_anchors_to_parent_config.display_area};
  auto const& parent_size{client_anchors_to_parent_config.parent_size};
  auto const& child_size{client_anchors_to_parent_config.child_size};
  auto const& parent_position{client_anchors_to_parent_config.parent_position};

  auto const rect_size{10};
  Rectangle const mid_right{
      parent_position +
          Point{parent_size.width - rect_size, parent_size.height / 2},
      {rect_size, rect_size}};

  Positioner const positioner{.anchor_rect = mid_right,
                              .parent_anchor = Anchor::top_right,
                              .child_anchor = Anchor::top_left,
                              .offset = Point{rect_size, 0},
                              .constraint_adjustment = static_cast<Constraint>(
                                  static_cast<int>(Constraint::slide_y) |
                                  static_cast<int>(Constraint::resize_x))};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, positioner.anchor_rect.value(),
      {parent_position, parent_size}, display_area)};

  auto const expected_position{
      parent_position + Point{parent_size.width, parent_size.height / 2}};

  EXPECT_EQ(child_rect.top_left, expected_position);
  EXPECT_EQ(child_rect.size, child_size);
}

TEST_F(WindowPlacementTest, ClientAnchorsToParentGivenOffsetAboveParent) {
  auto const& display_area{client_anchors_to_parent_config.display_area};
  auto const& parent_size{client_anchors_to_parent_config.parent_size};
  auto const& child_size{client_anchors_to_parent_config.child_size};
  auto const& parent_position{client_anchors_to_parent_config.parent_position};

  auto const rect_size{10};
  Rectangle const mid_top{parent_position + Point{parent_size.width / 2, 0},
                          {rect_size, rect_size}};

  Positioner const positioner{.anchor_rect = mid_top,
                              .parent_anchor = Anchor::top_right,
                              .child_anchor = Anchor::bottom_right,
                              .offset = Point{0, -rect_size},
                              .constraint_adjustment = Constraint::slide_x};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, positioner.anchor_rect.value(),
      {parent_position, parent_size}, display_area)};

  auto const expected_position{parent_position +
                               Point{parent_size.width / 2 + rect_size, 0} -
                               static_cast<Point>(child_size)};

  EXPECT_EQ(child_rect.top_left, expected_position);
  EXPECT_EQ(child_rect.size, child_size);
}

TEST_F(WindowPlacementTest,
       ClientAnchorsToParentGivenRectAndOffsetBelowLeftParent) {
  auto const& display_area{client_anchors_to_parent_config.display_area};
  auto const& parent_size{client_anchors_to_parent_config.parent_size};
  auto const& child_size{client_anchors_to_parent_config.child_size};
  auto const& parent_position{client_anchors_to_parent_config.parent_position};

  auto const rect_size{10};
  Rectangle const below_left{
      parent_position + Point{-rect_size, parent_size.height},
      {rect_size, rect_size}};

  Positioner const positioner{.anchor_rect = below_left,
                              .parent_anchor = Anchor::bottom_left,
                              .child_anchor = Anchor::top_right,
                              .offset = Point{-rect_size, rect_size},
                              .constraint_adjustment = Constraint::resize_any};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, positioner.anchor_rect.value(),
      {parent_position, parent_size}, display_area)};

  auto const expected_position{parent_position + Point{0, parent_size.height} -
                               Point{child_size.width, 0}};

  EXPECT_EQ(child_rect.top_left, expected_position);
  EXPECT_EQ(child_rect.size, child_size);
}

TEST_F(WindowPlacementTest,
       AttachesToRightEdgeGivenAnchorRectAwayFromRightSide) {
  positioner = {.anchor_rect = rectangle_away_from_rhs,
                .parent_anchor = Anchor::top_right,
                .child_anchor = Anchor::top_left};

  auto const expected_position{on_right_edge()};

  auto const child_rect{
      internal::PlaceWindow(positioner, child_size, anchor_rect(),
                            {parent_position, parent_size}, display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest, AttachesToLeftEdgeGivenAnchorRectNearRightSide) {
  positioner = {.anchor_rect = rectangle_near_rhs,
                .parent_anchor = Anchor::top_left,
                .child_anchor = Anchor::top_right};

  auto const expected_position{on_left_edge()};

  auto const child_rect{
      internal::PlaceWindow(positioner, child_size, anchor_rect(),
                            {parent_position, parent_size}, display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest, AttachesToRightEdgeGivenAnchorRectNearBothSides) {
  positioner = {.anchor_rect = rectangle_near_both_sides,
                .parent_anchor = Anchor::top_right,
                .child_anchor = Anchor::top_left};

  auto const expected_position{on_right_edge()};

  auto const child_rect{
      internal::PlaceWindow(positioner, child_size, anchor_rect(),
                            {parent_position, parent_size}, display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest, AttachesToBottomEdgeGivenAnchorRectAwayFromBottom) {
  positioner = {.anchor_rect = rectangle_away_from_bottom,
                .parent_anchor = Anchor::bottom_left,
                .child_anchor = Anchor::top_left};

  auto const expected_position{on_bottom_edge()};

  auto const child_rect{
      internal::PlaceWindow(positioner, child_size, anchor_rect(),
                            {parent_position, parent_size}, display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest, AttachesToTopEdgeGivenAnchorRectNearBottom) {
  positioner = {.anchor_rect = rectangle_near_bottom,
                .parent_anchor = Anchor::top_left,
                .child_anchor = Anchor::bottom_left};

  auto const expected_position{on_top_edge()};

  auto const child_rect{
      internal::PlaceWindow(positioner, child_size, anchor_rect(),
                            {parent_position, parent_size}, display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest, AttachesToBottomEdgeGivenAnchorRectNearBothSides) {
  positioner = {.anchor_rect = rectangle_near_both_sides,
                .parent_anchor = Anchor::bottom_left,
                .child_anchor = Anchor::top_left};

  auto const expected_position{on_bottom_edge()};

  auto const child_rect{
      internal::PlaceWindow(positioner, child_size, anchor_rect(),
                            {parent_position, parent_size}, display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest,
       AttachesToTopEdgeGivenAnchorRectNearBothSidesAndBottom) {
  positioner = {.anchor_rect = rectangle_near_both_sides_and_bottom,
                .parent_anchor = Anchor::top_left,
                .child_anchor = Anchor::bottom_left};

  auto const expected_position{on_top_edge()};

  auto const child_rect{
      internal::PlaceWindow(positioner, child_size, anchor_rect(),
                            {parent_position, parent_size}, display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest, AttachesToRightEdgeGivenAnchorRectNearAllSides) {
  positioner = {.anchor_rect = rectangle_near_all_sides,
                .parent_anchor = Anchor::top_right,
                .child_anchor = Anchor::top_left};

  auto const expected_position{on_right_edge()};

  auto const child_rect{
      internal::PlaceWindow(positioner, child_size, anchor_rect(),
                            {parent_position, parent_size}, display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

namespace {
Anchor const all_anchors[]{
    Anchor::top_left,    Anchor::top,    Anchor::top_right,
    Anchor::left,        Anchor::center, Anchor::right,
    Anchor::bottom_left, Anchor::bottom, Anchor::bottom_right,
};

auto position_of(Anchor anchor, Rectangle rectangle) -> Point {
  switch (anchor) {
    case Anchor::top_left:
      return rectangle.top_left;
    case Anchor::top:
      return rectangle.top_left + Point{rectangle.size.width / 2, 0};
    case Anchor::top_right:
      return rectangle.top_left + Point{rectangle.size.width, 0};
    case Anchor::left:
      return rectangle.top_left + Point{0, rectangle.size.height / 2};
    case Anchor::center:
      return rectangle.top_left +
             Point{rectangle.size.width / 2, rectangle.size.height / 2};
    case Anchor::right:
      return rectangle.top_left +
             Point{rectangle.size.width, rectangle.size.height / 2};
    case Anchor::bottom_left:
      return rectangle.top_left + Point{0, rectangle.size.height};
    case Anchor::bottom:
      return rectangle.top_left +
             Point{rectangle.size.width / 2, rectangle.size.height};
    case Anchor::bottom_right:
      return rectangle.top_left + static_cast<Point>(rectangle.size);
    default:
      std::cerr << "Unknown anchor value: " << static_cast<int>(anchor) << '\n';
      std::abort();
  }
}
}  // namespace

TEST_F(WindowPlacementTest, CanAttachByEveryAnchorGivenNoConstraintAdjustment) {
  positioner.anchor_rect = Rectangle{{100, 50}, {20, 20}};
  positioner.constraint_adjustment = Constraint{};

  for (auto const rect_anchor : all_anchors) {
    positioner.parent_anchor = rect_anchor;

    auto const anchor_position{position_of(rect_anchor, anchor_rect())};

    for (auto const window_anchor : all_anchors) {
      positioner.child_anchor = window_anchor;

      auto const child_rect{internal::PlaceWindow(
          positioner, child_size, anchor_rect(), parent_rect(), display_area)};

      EXPECT_EQ(position_of(window_anchor, child_rect), anchor_position);
    }
  }
}

TEST_F(WindowPlacementTest,
       PlacementIsFlippedGivenAnchorRectNearRightSideAndOffset) {
  auto const x_offset{42};
  auto const y_offset{13};

  positioner.anchor_rect = rectangle_near_rhs;
  positioner.constraint_adjustment = Constraint::flip_x;
  positioner.offset = Point{x_offset, y_offset};
  positioner.child_anchor = Anchor::top_left;
  positioner.parent_anchor = Anchor::top_right;

  auto const expected_position{on_left_edge() + Point{-1 * x_offset, y_offset}};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest,
       PlacementIsFlippedGivenAnchorRectNearBottomAndOffset) {
  auto const x_offset{42};
  auto const y_offset{13};

  positioner.anchor_rect = rectangle_near_bottom;
  positioner.constraint_adjustment = Constraint::flip_y;
  positioner.offset = Point{x_offset, y_offset};
  positioner.child_anchor = Anchor::top_left;
  positioner.parent_anchor = Anchor::bottom_left;

  auto const expected_position{on_top_edge() + Point{x_offset, -1 * y_offset}};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest,
       PlacementIsFlippedBothWaysGivenAnchorRectNearBottomRightAndOffset) {
  auto const x_offset{42};
  auto const y_offset{13};

  positioner.anchor_rect = rectangle_near_both_bottom_right;
  positioner.constraint_adjustment = Constraint::flip_any;
  positioner.offset = Point{x_offset, y_offset};
  positioner.child_anchor = Anchor::top_left;
  positioner.parent_anchor = Anchor::bottom_right;

  auto const expected_position{anchor_rect().top_left -
                               static_cast<Point>(child_size) -
                               Point{x_offset, y_offset}};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest, PlacementCanSlideInXGivenAnchorRectNearRightSide) {
  positioner.anchor_rect = rectangle_near_rhs;
  positioner.constraint_adjustment = Constraint::slide_x;
  positioner.child_anchor = Anchor::top_left;
  positioner.parent_anchor = Anchor::top_right;

  Point const expected_position{
      (display_area.top_left.x + display_area.size.width) - child_size.width,
      anchor_rect().top_left.y};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest, PlacementCanSlideInXGivenAnchorRectNearLeftSide) {
  Rectangle const rectangle_near_left_side{{0, 20}, {20, 20}};

  positioner.anchor_rect = rectangle_near_left_side;
  positioner.constraint_adjustment = Constraint::slide_x;
  positioner.child_anchor = Anchor::top_right;
  positioner.parent_anchor = Anchor::top_left;

  Point const expected_position{display_area.top_left.x,
                                anchor_rect().top_left.y};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest, PlacementCanSlideInYGivenAnchorRectNearBottom) {
  positioner.anchor_rect = rectangle_near_bottom;
  positioner.constraint_adjustment = Constraint::slide_y;
  positioner.child_anchor = Anchor::top_left;
  positioner.parent_anchor = Anchor::bottom_left;

  Point const expected_position{
      anchor_rect().top_left.x,
      (display_area.top_left.y + display_area.size.height) - child_size.height};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest, PlacementCanSlideInYGivenAnchorRectNearTop) {
  positioner.anchor_rect = rectangle_near_all_sides;
  positioner.constraint_adjustment = Constraint::slide_y;
  positioner.child_anchor = Anchor::bottom_left;
  positioner.parent_anchor = Anchor::top_left;

  Point const expected_position{anchor_rect().top_left.x,
                                display_area.top_left.y};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest,
       PlacementCanSlideInXAndYGivenAnchorRectNearBottomRightAndOffset) {
  positioner.anchor_rect = rectangle_near_both_bottom_right;
  positioner.constraint_adjustment = Constraint::slide_any;
  positioner.child_anchor = Anchor::top_left;
  positioner.parent_anchor = Anchor::bottom_left;

  auto const expected_position{
      (display_area.top_left + static_cast<Point>(display_area.size)) -
      static_cast<Point>(child_size)};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
}

TEST_F(WindowPlacementTest, PlacementCanResizeInXGivenAnchorRectNearRightSide) {
  positioner.anchor_rect = rectangle_near_rhs;
  positioner.constraint_adjustment = Constraint::resize_x;
  positioner.child_anchor = Anchor::top_left;
  positioner.parent_anchor = Anchor::top_right;

  auto const expected_position{anchor_rect().top_left +
                               Point{anchor_rect().size.width, 0}};
  Size const expected_size{
      (display_area.top_left.x + display_area.size.width) -
          (anchor_rect().top_left.x + anchor_rect().size.width),
      child_size.height};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
  EXPECT_EQ(child_rect.size, expected_size);
}

TEST_F(WindowPlacementTest, PlacementCanResizeInXGivenAnchorRectNearLeftSide) {
  Rectangle const rectangle_near_left_side{{0, 20}, {20, 20}};

  positioner.anchor_rect = rectangle_near_left_side;
  positioner.constraint_adjustment = Constraint::resize_x;
  positioner.child_anchor = Anchor::top_right;
  positioner.parent_anchor = Anchor::top_left;

  Point const expected_position{display_area.top_left.x,
                                anchor_rect().top_left.y};
  Size const expected_size{anchor_rect().top_left.x - display_area.top_left.x,
                           child_size.height};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
  EXPECT_EQ(child_rect.size, expected_size);
}

TEST_F(WindowPlacementTest, PlacementCanResizeInYGivenAnchorRectNearBottom) {
  positioner.anchor_rect = rectangle_near_bottom;
  positioner.constraint_adjustment = Constraint::resize_y;
  positioner.child_anchor = Anchor::top_left;
  positioner.parent_anchor = Anchor::bottom_left;

  auto const expected_position{anchor_rect().top_left +
                               Point{0, anchor_rect().size.height}};
  Size const expected_size{
      child_size.width,
      (display_area.top_left.y + display_area.size.height) -
          (anchor_rect().top_left.y + anchor_rect().size.height)};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
  EXPECT_EQ(child_rect.size, expected_size);
}

TEST_F(WindowPlacementTest, PlacementCanResizeInYGivenAnchorRectNearTop) {
  positioner.anchor_rect = rectangle_near_all_sides;
  positioner.constraint_adjustment = Constraint::resize_y;
  positioner.child_anchor = Anchor::bottom_left;
  positioner.parent_anchor = Anchor::top_left;

  Point const expected_position{anchor_rect().top_left.x,
                                display_area.top_left.y};
  Size const expected_size{child_size.width,
                           anchor_rect().top_left.y - display_area.top_left.y};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
  EXPECT_EQ(child_rect.size, expected_size);
}

TEST_F(WindowPlacementTest,
       PlacementCanResizeInXAndYGivenAnchorRectNearBottomRightAndOffset) {
  positioner.anchor_rect = rectangle_near_both_bottom_right;
  positioner.constraint_adjustment = Constraint::resize_any;
  positioner.child_anchor = Anchor::top_left;
  positioner.parent_anchor = Anchor::bottom_right;

  auto const expected_position{anchor_rect().top_left +
                               static_cast<Point>(anchor_rect().size)};
  Size const expected_size{
      (display_area.top_left.x + display_area.size.width) - expected_position.x,
      (display_area.top_left.y + display_area.size.height) -
          expected_position.y};

  auto const child_rect{internal::PlaceWindow(
      positioner, child_size, anchor_rect(), parent_rect(), display_area)};

  EXPECT_EQ(child_rect.top_left, expected_position);
  EXPECT_EQ(child_rect.size, expected_size);
}

}  // namespace flutter
