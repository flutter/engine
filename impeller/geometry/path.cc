// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/geometry/path.h"

#include <optional>
#include <variant>

#include "flutter/fml/logging.h"
#include "impeller/geometry/path_component.h"
#include "impeller/geometry/point.h"

namespace impeller {

Path::Path() : copy_on_shared_write_data_(new Data()) {
  AddContourComponent({});
};

Path::Path(const Path& other) {
  copy_on_shared_write_data_ = other.copy_on_shared_write_data_;
  copy_on_shared_write_data_->locked = true;
}

Path::Path(Path&& other) {
  copy_on_shared_write_data_ = other.copy_on_shared_write_data_;
  other.copy_on_shared_write_data_.reset(new Data());
}

void Path::operator=(const Path& other) {
  copy_on_shared_write_data_ = other.copy_on_shared_write_data_;
  copy_on_shared_write_data_->locked = true;
}

void Path::operator=(Path&& other) {
  copy_on_shared_write_data_ = other.copy_on_shared_write_data_;
  other.copy_on_shared_write_data_.reset(new Data());
}

Path::~Path() = default;

Path::Data& Path::GetModifiableData() {
  if (copy_on_shared_write_data_->locked) {
    copy_on_shared_write_data_.reset(new Data(*copy_on_shared_write_data_));
    copy_on_shared_write_data_->locked = false;
  }
  return *copy_on_shared_write_data_;
}

void Path::Reserve(size_t point_size, size_t verb_size) {
  auto& data = GetModifiableData();
  data.points.reserve(point_size);
  data.components.reserve(verb_size);
}

std::tuple<size_t, size_t> Path::Polyline::GetContourPointBounds(
    size_t contour_index) const {
  if (contour_index >= contours.size()) {
    return {points->size(), points->size()};
  }
  const size_t start_index = contours.at(contour_index).start_index;
  const size_t end_index = (contour_index >= contours.size() - 1)
                               ? points->size()
                               : contours.at(contour_index + 1).start_index;
  return std::make_tuple(start_index, end_index);
}

size_t Path::GetComponentCount(std::optional<ComponentType> type) const {
  auto& data = GetImmutableData();
  if (!type.has_value()) {
    return data.components.size();
  }
  auto type_value = type.value();
  if (type_value == ComponentType::kContour) {
    return data.contours.size();
  }
  size_t count = 0u;
  for (const auto& component : data.components) {
    if (component.type == type_value) {
      count++;
    }
  }
  return count;
}

void Path::SetFillType(FillType fill) {
  GetModifiableData().fill = fill;
}

FillType Path::GetFillType() const {
  return GetImmutableData().fill;
}

bool Path::IsConvex() const {
  return GetImmutableData().convexity == Convexity::kConvex;
}

bool Path::IsEmpty() const {
  return GetImmutableData().points.empty();
}

void Path::SetConvexity(Convexity value) {
  GetModifiableData().convexity = value;
}

void Path::Shift(Point shift) {
  auto& data = GetModifiableData();
  for (auto& point : data.points) {
    point += shift;
  }
  for (auto& contour : data.contours) {
    contour.destination += shift;
  }
}

Path& Path::AddLinearComponent(const Point& p1, const Point& p2) {
  auto& data = GetModifiableData();
  auto& points = data.points;
  auto index = points.size();
  points.emplace_back(p1);
  points.emplace_back(p2);
  data.components.emplace_back(ComponentType::kLinear, index);
  return *this;
}

Path& Path::AddQuadraticComponent(const Point& p1,
                                  const Point& cp,
                                  const Point& p2) {
  auto& data = GetModifiableData();
  auto& points = data.points;
  auto index = points.size();
  points.emplace_back(p1);
  points.emplace_back(cp);
  points.emplace_back(p2);
  data.components.emplace_back(ComponentType::kQuadratic, index);
  return *this;
}

Path& Path::AddCubicComponent(const Point& p1,
                              const Point& cp1,
                              const Point& cp2,
                              const Point& p2) {
  auto& data = GetModifiableData();
  auto& points = data.points;
  auto index = points.size();
  points.emplace_back(p1);
  points.emplace_back(cp1);
  points.emplace_back(cp2);
  points.emplace_back(p2);
  data.components.emplace_back(ComponentType::kCubic, index);
  return *this;
}

Path& Path::AddContourComponent(const Point& destination, bool is_closed) {
  auto& data = GetModifiableData();
  auto& components = data.components;
  auto& contours = data.contours;
  if (components.size() > 0 &&
      components.back().type == ComponentType::kContour) {
    // Never insert contiguous contours.
    contours.back() = ContourComponent(destination, is_closed);
  } else {
    contours.emplace_back(ContourComponent(destination, is_closed));
    components.emplace_back(ComponentType::kContour, contours.size() - 1);
  }
  return *this;
}

void Path::SetContourClosed(bool is_closed) {
  GetModifiableData().contours.back().is_closed = is_closed;
}

void Path::EnumerateComponents(
    const Applier<LinearPathComponent>& linear_applier,
    const Applier<QuadraticPathComponent>& quad_applier,
    const Applier<CubicPathComponent>& cubic_applier,
    const Applier<ContourComponent>& contour_applier) const {
  auto& data = GetImmutableData();
  auto& points = data.points;
  size_t currentIndex = 0;
  for (const auto& component : data.components) {
    switch (component.type) {
      case ComponentType::kLinear:
        if (linear_applier) {
          linear_applier(currentIndex,
                         LinearPathComponent(points[component.index],
                                             points[component.index + 1]));
        }
        break;
      case ComponentType::kQuadratic:
        if (quad_applier) {
          quad_applier(currentIndex,
                       QuadraticPathComponent(points[component.index],
                                              points[component.index + 1],
                                              points[component.index + 2]));
        }
        break;
      case ComponentType::kCubic:
        if (cubic_applier) {
          cubic_applier(currentIndex,
                        CubicPathComponent(points[component.index],
                                           points[component.index + 1],
                                           points[component.index + 2],
                                           points[component.index + 3]));
        }
        break;
      case ComponentType::kContour:
        if (contour_applier) {
          contour_applier(currentIndex, data.contours[component.index]);
        }
        break;
    }
    currentIndex++;
  }
}

bool Path::GetLinearComponentAtIndex(size_t index,
                                     LinearPathComponent& linear) const {
  auto& data = GetImmutableData();
  auto& components = data.components;

  if (index >= components.size()) {
    return false;
  }

  if (components[index].type != ComponentType::kLinear) {
    return false;
  }

  auto& points = data.points;
  auto point_index = components[index].index;
  linear = LinearPathComponent(points[point_index], points[point_index + 1]);
  return true;
}

bool Path::GetQuadraticComponentAtIndex(
    size_t index,
    QuadraticPathComponent& quadratic) const {
  auto& data = GetImmutableData();
  auto& components = data.components;

  if (index >= components.size()) {
    return false;
  }

  if (components[index].type != ComponentType::kQuadratic) {
    return false;
  }

  auto& points = data.points;
  auto point_index = components[index].index;
  quadratic = QuadraticPathComponent(
      points[point_index], points[point_index + 1], points[point_index + 2]);
  return true;
}

bool Path::GetCubicComponentAtIndex(size_t index,
                                    CubicPathComponent& cubic) const {
  auto& data = GetImmutableData();
  auto& components = data.components;

  if (index >= components.size()) {
    return false;
  }

  if (components[index].type != ComponentType::kCubic) {
    return false;
  }

  auto& points = data.points;
  auto point_index = components[index].index;
  cubic = CubicPathComponent(points[point_index], points[point_index + 1],
                             points[point_index + 2], points[point_index + 3]);
  return true;
}

bool Path::GetContourComponentAtIndex(size_t index,
                                      ContourComponent& move) const {
  auto& data = GetImmutableData();
  auto& components = data.components;

  if (index >= components.size()) {
    return false;
  }

  if (components[index].type != ComponentType::kContour) {
    return false;
  }

  move = data.contours[components[index].index];
  return true;
}

Path::Polyline::Polyline(Path::Polyline::PointBufferPtr point_buffer,
                         Path::Polyline::ReclaimPointBufferCallback reclaim)
    : points(std::move(point_buffer)), reclaim_points_(std::move(reclaim)) {
  FML_DCHECK(points);
}

Path::Polyline::Polyline(Path::Polyline&& other) {
  points = std::move(other.points);
  reclaim_points_ = std::move(other.reclaim_points_);
  contours = std::move(other.contours);
}

Path::Polyline::~Polyline() {
  if (reclaim_points_) {
    points->clear();
    reclaim_points_(std::move(points));
  }
}

Path::Polyline Path::CreatePolyline(
    Scalar scale,
    Path::Polyline::PointBufferPtr point_buffer,
    Path::Polyline::ReclaimPointBufferCallback reclaim) const {
  Polyline polyline(std::move(point_buffer), std::move(reclaim));

  auto& data = GetImmutableData();
  auto& path_components = data.components;
  auto& path_points = data.points;

  auto get_path_component = [&path_components, &path_points](
                                size_t component_i) -> PathComponentVariant {
    if (component_i >= path_components.size()) {
      return std::monostate{};
    }
    const auto& component = path_components[component_i];
    switch (component.type) {
      case ComponentType::kLinear:
        return reinterpret_cast<const LinearPathComponent*>(
            &path_points[component.index]);
      case ComponentType::kQuadratic:
        return reinterpret_cast<const QuadraticPathComponent*>(
            &path_points[component.index]);
      case ComponentType::kCubic:
        return reinterpret_cast<const CubicPathComponent*>(
            &path_points[component.index]);
      case ComponentType::kContour:
        return std::monostate{};
    }
  };

  auto compute_contour_start_direction =
      [&get_path_component](size_t current_path_component_index) {
        size_t next_component_index = current_path_component_index + 1;
        while (!std::holds_alternative<std::monostate>(
            get_path_component(next_component_index))) {
          auto next_component = get_path_component(next_component_index);
          auto maybe_vector =
              std::visit(PathComponentStartDirectionVisitor(), next_component);
          if (maybe_vector.has_value()) {
            return maybe_vector.value();
          } else {
            next_component_index++;
          }
        }
        return Vector2(0, -1);
      };

  std::vector<PolylineContour::Component> poly_components;
  std::optional<size_t> previous_path_component_index;
  auto end_contour = [&polyline, &previous_path_component_index,
                      &get_path_component, &poly_components]() {
    // Whenever a contour has ended, extract the exact end direction from
    // the last component.
    if (polyline.contours.empty()) {
      return;
    }

    if (!previous_path_component_index.has_value()) {
      return;
    }

    auto& contour = polyline.contours.back();
    contour.end_direction = Vector2(0, 1);
    contour.components = poly_components;
    poly_components.clear();

    size_t previous_index = previous_path_component_index.value();
    while (!std::holds_alternative<std::monostate>(
        get_path_component(previous_index))) {
      auto previous_component = get_path_component(previous_index);
      auto maybe_vector =
          std::visit(PathComponentEndDirectionVisitor(), previous_component);
      if (maybe_vector.has_value()) {
        contour.end_direction = maybe_vector.value();
        break;
      } else {
        if (previous_index == 0) {
          break;
        }
        previous_index--;
      }
    }
  };

  for (size_t component_i = 0; component_i < path_components.size();
       component_i++) {
    const auto& path_component = path_components[component_i];
    switch (path_component.type) {
      case ComponentType::kLinear:
        poly_components.push_back({
            .component_start_index = polyline.points->size() - 1,
            .is_curve = false,
        });
        reinterpret_cast<const LinearPathComponent*>(
            &path_points[path_component.index])
            ->AppendPolylinePoints(*polyline.points);
        previous_path_component_index = component_i;
        break;
      case ComponentType::kQuadratic:
        poly_components.push_back({
            .component_start_index = polyline.points->size() - 1,
            .is_curve = true,
        });
        reinterpret_cast<const QuadraticPathComponent*>(
            &path_points[path_component.index])
            ->AppendPolylinePoints(scale, *polyline.points);
        previous_path_component_index = component_i;
        break;
      case ComponentType::kCubic:
        poly_components.push_back({
            .component_start_index = polyline.points->size() - 1,
            .is_curve = true,
        });
        reinterpret_cast<const CubicPathComponent*>(
            &path_points[path_component.index])
            ->AppendPolylinePoints(scale, *polyline.points);
        previous_path_component_index = component_i;
        break;
      case ComponentType::kContour:
        if (component_i == path_components.size() - 1) {
          // If the last component is a contour, that means it's an empty
          // contour, so skip it.
          continue;
        }
        end_contour();

        Vector2 start_direction = compute_contour_start_direction(component_i);
        const auto& contour = data.contours[path_component.index];
        polyline.contours.push_back({.start_index = polyline.points->size(),
                                     .is_closed = contour.is_closed,
                                     .start_direction = start_direction,
                                     .components = poly_components});

        polyline.points->push_back(contour.destination);
        break;
    }
  }
  end_contour();
  return polyline;
}

std::optional<Rect> Path::GetBoundingBox() const {
  return GetImmutableData().computed_bounds;
}

void Path::ComputeBounds() {
  auto& data = GetModifiableData();
  auto min_max = GetMinMaxCoveragePoints();
  if (!min_max.has_value()) {
    data.computed_bounds = std::nullopt;
    return;
  }
  auto min = min_max->first;
  auto max = min_max->second;
  const auto difference = max - min;
  data.computed_bounds =
      Rect::MakeXYWH(min.x, min.y, difference.x, difference.y);
}

std::optional<Rect> Path::GetTransformedBoundingBox(
    const Matrix& transform) const {
  auto bounds = GetBoundingBox();
  if (!bounds.has_value()) {
    return std::nullopt;
  }
  return bounds->TransformBounds(transform);
}

std::optional<std::pair<Point, Point>> Path::GetMinMaxCoveragePoints() const {
  auto data = GetImmutableData();
  auto& points = data.points;

  if (points.empty()) {
    return std::nullopt;
  }

  std::optional<Point> min, max;

  auto clamp = [&min, &max](const Point& point) {
    if (min.has_value()) {
      min = min->Min(point);
    } else {
      min = point;
    }

    if (max.has_value()) {
      max = max->Max(point);
    } else {
      max = point;
    }
  };

  for (const auto& component : data.components) {
    switch (component.type) {
      case ComponentType::kLinear: {
        auto* linear = reinterpret_cast<const LinearPathComponent*>(
            &points[component.index]);
        clamp(linear->p1);
        clamp(linear->p2);
        break;
      }
      case ComponentType::kQuadratic:
        for (const auto& extrema :
             reinterpret_cast<const QuadraticPathComponent*>(
                 &points[component.index])
                 ->Extrema()) {
          clamp(extrema);
        }
        break;
      case ComponentType::kCubic:
        for (const auto& extrema : reinterpret_cast<const CubicPathComponent*>(
                                       &points[component.index])
                                       ->Extrema()) {
          clamp(extrema);
        }
        break;
      case ComponentType::kContour:
        break;
    }
  }

  if (!min.has_value() || !max.has_value()) {
    return std::nullopt;
  }

  return std::make_pair(min.value(), max.value());
}

void Path::SetBounds(Rect rect) {
  GetModifiableData().computed_bounds = rect;
}

}  // namespace impeller
