// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ax_tree_id.h"

#include <algorithm>
#include <iostream>

#include "base/logging.h"

#include "base/no_destructor.h"

#include "ax_enums.h"

namespace ui {

AXTreeID::AXTreeID() : AXTreeID(ax::mojom::AXTreeIDType::kUnknown) {}

AXTreeID::AXTreeID(const AXTreeID& other) = default;

AXTreeID::AXTreeID(ax::mojom::AXTreeIDType type) : type_(type) {}

AXTreeID::AXTreeID(const std::string& string) {
  // TODO(chunhtai): either remove the ax tree id entirely or implement token
  // for real.
  if (string.empty()) {
    type_ = ax::mojom::AXTreeIDType::kUnknown;
  } else if (string == "kToken") {
    type_ = ax::mojom::AXTreeIDType::kToken;
  } else {
    BASE_UNREACHABLE();
  }
}

// static
AXTreeID AXTreeID::FromString(const std::string& string) {
  return AXTreeID(string);
}

// static
AXTreeID AXTreeID::CreateNewAXTreeID() {
  return AXTreeID(ax::mojom::AXTreeIDType::kToken);
}

AXTreeID& AXTreeID::operator=(const AXTreeID& other) = default;

std::string AXTreeID::ToString() const {
  // TODO(chunhtai): either remove the ax tree id entirely or implement token
  // for real.
  switch (type_) {
    case ax::mojom::AXTreeIDType::kUnknown:
      return "";
    case ax::mojom::AXTreeIDType::kToken:
      return "kToken";
  }

  BASE_UNREACHABLE();
  return std::string();
}

void swap(AXTreeID& first, AXTreeID& second) {
  std::swap(first.type_, second.type_);
}

bool AXTreeID::operator==(const AXTreeID& rhs) const {
  return type_ == rhs.type_;  //&& token_ == rhs.token_;
}

bool AXTreeID::operator!=(const AXTreeID& rhs) const {
  return !(*this == rhs);
}

bool AXTreeID::operator<(const AXTreeID& rhs) const {
  return type_ < rhs.type_;
}

bool AXTreeID::operator<=(const AXTreeID& rhs) const {
  return type_ <= rhs.type_;
}

bool AXTreeID::operator>(const AXTreeID& rhs) const {
  return !(*this <= rhs);
}

bool AXTreeID::operator>=(const AXTreeID& rhs) const {
  return !(*this < rhs);
}

size_t AXTreeIDHash::operator()(const ui::AXTreeID& tree_id) const {
  BASE_DCHECK(tree_id.type() == ax::mojom::AXTreeIDType::kToken);
  return 0;
}

std::ostream& operator<<(std::ostream& stream, const AXTreeID& value) {
  return stream << value.ToString();
}

const AXTreeID& AXTreeIDUnknown() {
  static const base::NoDestructor<AXTreeID> ax_tree_id_unknown(
      ax::mojom::AXTreeIDType::kUnknown);
  return *ax_tree_id_unknown;
}

}  // namespace ui
