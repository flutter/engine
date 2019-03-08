// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/cpp/text_input_model.h"

#include <iostream>

// TODO(awdavies): Need to fix this regarding issue #47.
static constexpr char kComposingBaseKey[] = "composingBase";

static constexpr char kComposingExtentKey[] = "composingExtent";

static constexpr char kSelectionAffinityKey[] = "selectionAffinity";
static constexpr char kAffinityDownstream[] = "TextAffinity.downstream";

static constexpr char kSelectionBaseKey[] = "selectionBase";
static constexpr char kSelectionExtentKey[] = "selectionExtent";

static constexpr char kSelectionIsDirectionalKey[] = "selectionIsDirectional";

static constexpr char kTextKey[] = "text";

// Input client configuration keys.
static constexpr char kTextInputAction[] = "inputAction";
static constexpr char kTextInputType[] = "inputType";
static constexpr char kTextInputTypeName[] = "name";

namespace shell {

TextInputModel::TextInputModel(int client_id, const Json::Value& config)
    : text_(""),
      client_id_(client_id),
      selection_base_(text_.begin()),
      selection_extent_(text_.begin()) {
  input_action_ = config[kTextInputAction].asString();
  Json::Value input_type_info = config[kTextInputType];
  input_type_ = input_type_info[kTextInputTypeName].asString();
}

TextInputModel::~TextInputModel() {}

bool TextInputModel::SetEditingState(size_t selection_base,
                                     size_t selection_extent,
                                     const std::string& text) {
  if (selection_base > selection_extent) {
    return false;
  }
  // Only checks extent since it is implicitly greater-than-or-equal-to base.
  if (selection_extent > text.size()) {
    return false;
  }
  text_ = std::string(text);
  selection_base_ = text_.begin() + selection_base;
  selection_extent_ = text_.begin() + selection_extent;
  return true;
}

void TextInputModel::DeleteSelected() {
  selection_base_ = text_.erase(selection_base_, selection_extent_);
  // Moves extent back to base, so that it is a single cursor placement again.
  selection_extent_ = selection_base_;
}

void TextInputModel::AddCharacter(char c) {
  if (selection_base_ != selection_extent_) {
    DeleteSelected();
  }
  selection_extent_ = text_.insert(selection_extent_, c);
  selection_extent_++;
  selection_base_ = selection_extent_;
}

bool TextInputModel::Backspace() {
  if (selection_base_ != selection_extent_) {
    DeleteSelected();
    return true;
  }
  if (selection_base_ != text_.begin()) {
    selection_base_ = text_.erase(selection_base_ - 1, selection_base_);
    selection_extent_ = selection_base_;
    return true;
  }
  return false;  // No edits happened.
}

bool TextInputModel::Delete() {
  if (selection_base_ != selection_extent_) {
    DeleteSelected();
    return true;
  }
  if (selection_base_ != text_.end()) {
    selection_base_ = text_.erase(selection_base_, selection_base_ + 1);
    selection_extent_ = selection_base_;
    return true;
  }
  return false;
}

void TextInputModel::MoveCursorToBeginning() {
  selection_base_ = text_.begin();
  selection_extent_ = text_.begin();
}

void TextInputModel::MoveCursorToEnd() {
  selection_base_ = text_.end();
  selection_extent_ = text_.end();
}

bool TextInputModel::MoveCursorForward() {
  // If about to move set to the end of the highlight (when not selecting).
  if (selection_base_ != selection_extent_) {
    selection_base_ = selection_extent_;
    return true;
  }
  // If not at the end, move the extent forward.
  if (selection_extent_ != text_.end()) {
    selection_extent_++;
    selection_base_++;
    return true;
  }
  return false;
}

bool TextInputModel::MoveCursorBack() {
  // If about to move set to the beginning of the highlight
  // (when not selecting).
  if (selection_base_ != selection_extent_) {
    selection_extent_ = selection_base_;
    return true;
  }
  // If not at the start, move the beginning backward.
  if (selection_base_ != text_.begin()) {
    selection_base_--;
    selection_extent_--;
    return true;
  }
  return false;
}

Json::Value TextInputModel::GetState() const {
  // TODO(awdavies): Most of these are hard-coded for now.
  Json::Value editing_state;
  editing_state[kComposingBaseKey] = -1;
  editing_state[kComposingExtentKey] = -1;
  editing_state[kSelectionAffinityKey] = kAffinityDownstream;
  editing_state[kSelectionBaseKey] =
      static_cast<int>(selection_base_ - text_.begin());
  editing_state[kSelectionExtentKey] =
      static_cast<int>(selection_extent_ - text_.begin());
  editing_state[kSelectionIsDirectionalKey] = false;
  editing_state[kTextKey] = text_;

  // TODO(stuartmorgan): Move client_id out up to the plugin so that this
  // function just returns the editing state.
  Json::Value args = Json::arrayValue;
  args.append(client_id_);
  args.append(editing_state);
  return args;
}

}  // namespace shell
