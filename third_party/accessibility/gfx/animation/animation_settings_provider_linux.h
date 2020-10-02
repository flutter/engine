// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_ANIMATION_ANIMATION_SETTINGS_PROVIDER_LINUX_H_
#define UI_GFX_ANIMATION_ANIMATION_SETTINGS_PROVIDER_LINUX_H_

#include "base/macros.h"
#include "ui/gfx/animation/animation_export.h"

namespace gfx {

class ANIMATION_EXPORT AnimationSettingsProviderLinux {
 public:
  virtual ~AnimationSettingsProviderLinux();

  // Indicates if animations are enabled by the toolkit.
  virtual bool AnimationsEnabled() const = 0;

  static AnimationSettingsProviderLinux* GetInstance();

 protected:
  AnimationSettingsProviderLinux();

 private:
  static AnimationSettingsProviderLinux* instance_;

  DISALLOW_COPY_AND_ASSIGN(AnimationSettingsProviderLinux);
};

}  // namespace gfx

#endif  // UI_GFX_ANIMATION_ANIMATION_SETTINGS_PROVIDER_LINUX_H_
