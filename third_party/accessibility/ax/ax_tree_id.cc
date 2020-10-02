// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ax_tree_id.h"

#include <algorithm>
#include <iostream>

#include "flutter/fml/logging.h"

// #include "base/check.h"
#include "base/no_destructor.h"
// #include "base/FML_DCHECK(false).h"
// #include "base/util/values/values_util.h"
// #include "base/values.h"
#include "ax_enums.h"

namespace ax {

AXTreeID::AXTreeID() : AXTreeID(ax::AXTreeIDType::kUnknown) {}

AXTreeID::AXTreeID(const AXTreeID& other) = default;

AXTreeID::AXTreeID(ax::AXTreeIDType type) : type_(type) {
  // if (type_ == ax::AXTreeIDType::kToken)
  //   token_ = base::UnguessableToken::Create();
}

AXTreeID::AXTreeID(const std::string& string) {
  if (string.empty()) {
    type_ = ax::AXTreeIDType::kUnknown;
  } else {
    // type_ = ax::AXTreeIDType::kToken;
    // std::optional<base::UnguessableToken> token =
    //     util::ValueToUnguessableToken(base::Value(string));
    // CHECK(token);
    // token_ = *token;
  }
}

// static
AXTreeID AXTreeID::FromString(const std::string& string) {
  return AXTreeID(string);
}

// static
// AXTreeID AXTreeID::FromToken(const base::UnguessableToken& token) {
//   return AXTreeID(token.ToString());
// }

// static
AXTreeID AXTreeID::CreateNewAXTreeID() {
  return AXTreeID(ax::AXTreeIDType::kToken);
}

AXTreeID& AXTreeID::operator=(const AXTreeID& other) = default;

std::string AXTreeID::ToString() const {
  switch (type_) {
    case ax::AXTreeIDType::kUnknown:
      return "";
    case ax::AXTreeIDType::kToken:
      return "";
      // return util::UnguessableTokenToValue(*token_).GetString();
  }

  FML_DCHECK(false);
  return std::string();
}

void swap(AXTreeID& first, AXTreeID& second) {
  std::swap(first.type_, second.type_);
  // std::swap(first.token_, second.token_);
}

bool AXTreeID::operator==(const AXTreeID& rhs) const {
  return type_ == rhs.type_ ;//&& token_ == rhs.token_;
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

size_t AXTreeIDHash::operator()(const ax::AXTreeID& tree_id) const {
  FML_DCHECK(tree_id.type() == ax::AXTreeIDType::kToken);
  // return base::UnguessableTokenHash()(tree_id.token().value());
  return 0;
}

std::ostream& operator<<(std::ostream& stream, const AXTreeID& value) {
  return stream << value.ToString();
}

const AXTreeID& AXTreeIDUnknown() {
  static const base::NoDestructor<AXTreeID> ax_tree_id_unknown(
      ax::AXTreeIDType::kUnknown);
  return *ax_tree_id_unknown;
}

}  // namespace ax
