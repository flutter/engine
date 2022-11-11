#include "paragraph_builder_ckj.h"
#include "paragraph_cjk.h"

namespace txt {

ParagraphBuilderCJK::ParagraphBuilderCJK(
    const txt::ParagraphStyle& style,
    std::shared_ptr<FontCollection> font_collection)
    : font_collection_(std::move(font_collection)) {
  SetParagraphStyle(style);
}

ParagraphBuilderCJK::~ParagraphBuilderCJK() = default;

void ParagraphBuilderCJK::SetParagraphStyle(const ParagraphStyle& style) {
  paragraph_style_ = style;
  paragraph_style_index_ = runs_.AddStyle(style.GetTextStyle());
  runs_.StartRun(paragraph_style_index_, text_.size());
}

void ParagraphBuilderCJK::PushStyle(const TextStyle& style) {
  size_t style_index = runs_.AddStyle(style);
  style_stack_.push_back(style_index);
  runs_.StartRun(style_index, text_.size());
}

void ParagraphBuilderCJK::Pop() {
  if (style_stack_.empty()) {
    return;
  }
  style_stack_.pop_back();
  runs_.StartRun(PeekStyleIndex(), text_.size());
}

size_t ParagraphBuilderCJK::PeekStyleIndex() const {
  return style_stack_.size() ? style_stack_.back() : paragraph_style_index_;
}

const TextStyle& ParagraphBuilderCJK::PeekStyle() {
  return runs_.GetStyle(PeekStyleIndex());
}

void ParagraphBuilderCJK::AddText(const std::u16string& text) {
  text_.insert(text_.end(), text.begin(), text.end());
}

void ParagraphBuilderCJK::AddPlaceholder(PlaceholderRun& span) {
  obj_replacement_char_indexes_.insert(text_.size());
  runs_.StartRun(PeekStyleIndex(), text_.size());
  AddText(std::u16string(1ull, 0xFFFC));
  runs_.StartRun(PeekStyleIndex(), text_.size());
  inline_placeholders_.push_back(span);
}

std::unique_ptr<Paragraph> ParagraphBuilderCJK::Build() {
  runs_.EndRunIfNeeded(text_.size());
  auto p = std::make_unique<ParagraphCJK>(
      std::move(text_), paragraph_style_, std::move(runs_), font_collection_,
      std::move(inline_placeholders_),
      std::move(obj_replacement_char_indexes_));
  return p;
}

}  // namespace txt
