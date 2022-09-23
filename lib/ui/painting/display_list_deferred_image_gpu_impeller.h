// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/display_list/display_list_image.h"
#include "flutter/flow/layers/layer_tree.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/fml/task_runner.h"
#include "flutter/lib/ui/snapshot_delegate.h"
#include "impeller/renderer/texture.h"

namespace flutter {

class DlDeferredImageGPUImpeller final : public DlImage {
 public:
  static sk_sp<DlDeferredImageGPUImpeller> Make(
      std::shared_ptr<LayerTree> layer_tree,
      const SkISize& size,
      fml::WeakPtr<SnapshotDelegate> snapshot_delegate,
      fml::RefPtr<fml::TaskRunner> raster_task_runner);

  static sk_sp<DlDeferredImageGPUImpeller> Make(
      sk_sp<DisplayList> display_list,
      const SkISize& size,
      fml::WeakPtr<SnapshotDelegate> snapshot_delegate,
      fml::RefPtr<fml::TaskRunner> raster_task_runner);

  // |DlImage|
  ~DlDeferredImageGPUImpeller() override;

  // |DlImage|
  sk_sp<SkImage> skia_image() const override;

  // |DlImage|
  std::shared_ptr<impeller::Texture> impeller_texture() const override;

  void set_texture(std::shared_ptr<impeller::Texture> texture);

  // |DlImage|
  bool isOpaque() const override;

  // |DlImage|
  bool isTextureBacked() const override;

  // |DlImage|
  SkISize dimensions() const override;

  // |DlImage|
  size_t GetApproximateByteSize() const override;

 private:
  SkISize size_;
  std::shared_ptr<impeller::Texture> texture_;

  explicit DlDeferredImageGPUImpeller(const SkISize& size);

  FML_DISALLOW_COPY_AND_ASSIGN(DlDeferredImageGPUImpeller);
};

}  // namespace flutter
