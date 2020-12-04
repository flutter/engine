// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "compute_attributes.h"

#include <cstddef>

#include "ax/ax_enums.h"
#include "ax/ax_node_data.h"
#include "ax_platform_node_delegate.h"

namespace ax {
namespace {

std::optional<int32_t> GetCellAttribute(
    const ax::AXPlatformNodeDelegate* delegate,
    ax::IntAttribute attribute) {
  switch (attribute) {
    case ax::IntAttribute::kAriaCellColumnIndex:
      return delegate->GetTableCellAriaColIndex();
    case ax::IntAttribute::kAriaCellRowIndex:
      return delegate->GetTableCellAriaRowIndex();
    case ax::IntAttribute::kTableCellColumnIndex:
      return delegate->GetTableCellColIndex();
    case ax::IntAttribute::kTableCellRowIndex:
      return delegate->GetTableCellRowIndex();
    case ax::IntAttribute::kTableCellColumnSpan:
      return delegate->GetTableCellColSpan();
    case ax::IntAttribute::kTableCellRowSpan:
      return delegate->GetTableCellRowSpan();
    default:
      return std::nullopt;
  }
}

std::optional<int32_t> GetRowAttribute(
    const ax::AXPlatformNodeDelegate* delegate,
    ax::IntAttribute attribute) {
  if (attribute == ax::IntAttribute::kTableRowIndex) {
    return delegate->GetTableRowRowIndex();
  }
  return std::nullopt;
}

std::optional<int32_t> GetTableAttribute(
    const ax::AXPlatformNodeDelegate* delegate,
    ax::IntAttribute attribute) {
  switch (attribute) {
    case ax::IntAttribute::kTableColumnCount:
      return delegate->GetTableColCount();
    case ax::IntAttribute::kTableRowCount:
      return delegate->GetTableRowCount();
    case ax::IntAttribute::kAriaColumnCount:
      return delegate->GetTableAriaColCount();
    case ax::IntAttribute::kAriaRowCount:
      return delegate->GetTableAriaRowCount();
    default:
      return std::nullopt;
  }
}

std::optional<int> GetOrderedSetItemAttribute(
    const ax::AXPlatformNodeDelegate* delegate,
    ax::IntAttribute attribute) {
  switch (attribute) {
    case ax::IntAttribute::kPosInSet:
      return delegate->GetPosInSet();
    case ax::IntAttribute::kSetSize:
      return delegate->GetSetSize();
    default:
      return std::nullopt;
  }
}

std::optional<int> GetOrderedSetAttribute(
    const ax::AXPlatformNodeDelegate* delegate,
    ax::IntAttribute attribute) {
  switch (attribute) {
    case ax::IntAttribute::kSetSize:
      return delegate->GetSetSize();
    default:
      return std::nullopt;
  }
}

std::optional<int32_t> GetFromData(const ax::AXPlatformNodeDelegate* delegate,
                                    ax::IntAttribute attribute) {
  int32_t value;
  if (delegate->GetData().GetIntAttribute(attribute, &value)) {
    return value;
  }
  return std::nullopt;
}

}  // namespace

std::optional<int32_t> ComputeAttribute(
    const ax::AXPlatformNodeDelegate* delegate,
    ax::IntAttribute attribute) {
  std::optional<int32_t> maybe_value = std::nullopt;
  // Table-related nodes.
  if (delegate->IsTableCellOrHeader())
    maybe_value = GetCellAttribute(delegate, attribute);
  else if (delegate->IsTableRow())
    maybe_value = GetRowAttribute(delegate, attribute);
  else if (delegate->IsTable())
    maybe_value = GetTableAttribute(delegate, attribute);
  // Ordered-set-related nodes.
  else if (delegate->IsOrderedSetItem())
    maybe_value = GetOrderedSetItemAttribute(delegate, attribute);
  else if (delegate->IsOrderedSet())
    maybe_value = GetOrderedSetAttribute(delegate, attribute);

  if (!maybe_value.has_value()) {
    return GetFromData(delegate, attribute);
  }
  return maybe_value;
}

}  // namespace ax
