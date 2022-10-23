#ifndef FLUTTER_PARAGRAPH_BUILDER_CKJ_H
#define FLUTTER_PARAGRAPH_BUILDER_CKJ_H

#include "txt/paragraph_builder.h"
#include "txt/styled_runs.h"

namespace txt {

class ParagraphBuilderCJK : public ParagraphBuilder {
 public:
  ParagraphBuilderCJK(const ParagraphStyle& style,
                      std::shared_ptr<FontCollection> font_collection);

  virtual ~ParagraphBuilderCJK();

  virtual void PushStyle(const TextStyle& style) override;
  virtual void Pop() override;
  virtual const TextStyle& PeekStyle() override;
  virtual void AddText(const std::u16string& text) override;
  virtual void AddPlaceholder(PlaceholderRun& span) override;
  virtual std::unique_ptr<Paragraph> Build() override;

 private:
  std::vector<uint16_t> text_;
  std::vector<PlaceholderRun> inline_placeholders_;
  std::unordered_set<size_t> obj_replacement_char_indexes_;
  std::vector<size_t> style_stack_;
  std::shared_ptr<FontCollection> font_collection_;
  StyledRuns runs_;
  ParagraphStyle paragraph_style_;
  size_t paragraph_style_index_;

  void SetParagraphStyle(const ParagraphStyle& style);

  size_t PeekStyleIndex() const;
};

}  // namespace txt

#endif  // FLUTTER_PARAGRAPH_BUILDER_CKJ_H
