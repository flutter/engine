// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "display_list/display_list_color_filter.h"
#include "impeller/aiks/color_filter_factory.h"
#include "impeller/entity/contents/color_source_contents.h"

namespace impeller {

//------------------------------------------------------------------------------
/// DlColorFilterFactory
///

class DlBlendColorFilterFactory : public ColorFilterFactory {
 public:
  // |ColorFilterFactory|
  ~DlBlendColorFilterFactory() override;

  explicit DlBlendColorFilterFactory(const flutter::DlColorFilter* filter);

 private:
  // |ColorFilterFactory|
  ColorSourceType GetType() const override;

  // |ColorFilterFactory|
  bool Equal(const ColorFilterFactory& other) const override;

  // |ColorFilterFactory|
  std::shared_ptr<ColorFilterContents> MakeContents(
      FilterInput::Ref input) const override;

  const flutter::DlBlendColorFilter* filter_;
};

class DlMatrixColorFilterFactory : public ColorFilterFactory {
 public:
  // |ColorFilterFactory|
  ~DlMatrixColorFilterFactory() override;

  explicit DlMatrixColorFilterFactory(const flutter::DlColorFilter* filter);

 private:
  // |ColorFilterFactory|
  ColorSourceType GetType() const override;

  // |ColorFilterFactory|
  bool Equal(const ColorFilterFactory& other) const override;

  // |ColorFilterFactory|
  std::shared_ptr<ColorFilterContents> MakeContents(
      FilterInput::Ref input) const override;

  const flutter::DlMatrixColorFilter* filter_;
};

class DlSrgbToLinearColorFilterFactory : public ColorFilterFactory {
 public:
  // |ColorFilterFactory|
  ~DlSrgbToLinearColorFilterFactory() override;

  explicit DlSrgbToLinearColorFilterFactory();

 private:
  // |ColorFilterFactory|
  ColorSourceType GetType() const override;

  // |ColorFilterFactory|
  bool Equal(const ColorFilterFactory& other) const override;

  // |ColorFilterFactory|
  std::shared_ptr<ColorFilterContents> MakeContents(
      FilterInput::Ref input) const override;
};

class DlLinearToSrgbColorFilterFactory : public ColorFilterFactory {
 public:
  // |ColorFilterFactory|
  ~DlLinearToSrgbColorFilterFactory() override;

  explicit DlLinearToSrgbColorFilterFactory();

 private:
  // |ColorFilterFactory|
  ColorSourceType GetType() const override;

  // |ColorFilterFactory|
  bool Equal(const ColorFilterFactory& other) const override;

  // |ColorFilterFactory|
  std::shared_ptr<ColorFilterContents> MakeContents(
      FilterInput::Ref input) const override;
};

}  // namespace impeller