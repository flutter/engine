// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SHELL_HOLDER_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SHELL_HOLDER_H_

#include <memory>

#include "flutter/assets/asset_manager.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/unique_fd.h"
#include "flutter/lib/ui/window/viewport_metrics.h"
#include "flutter/runtime/platform_data.h"
#include "flutter/shell/common/run_configuration.h"
#include "flutter/shell/common/shell.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/shell/platform/android/jni/platform_view_android_jni.h"
#include "flutter/shell/platform/android/platform_view_android.h"

namespace flutter {

//----------------------------------------------------------------------------
/// @brief      This is the Android owner of the core engine Shell.
///
/// @details    This is the top orchestrator class on the C++ side for the
///             Android embedding. It corresponds to a FlutterEngine on the
///             Java side. This class is in C++ because the Shell is in
///             C++ and an Android orchestrator needs to exist to
///             compose it with other Android specific C++ components such as
///             the PlatformViewAndroid. This composition of many-to-one
///             C++ components would be difficult to do through JNI whereas
///             a FlutterEngine and AndroidShellHolder has a 1:1 relationship.
///
///             Technically, the FlutterJNI class owns this AndroidShellHolder
///             class instance, but the FlutterJNI class is meant to be mostly
///             static and has minimal state to perform the C++ pointer <->
///             Java class instance translation.
///
class AndroidShellHolder {
 public:
  AndroidShellHolder(flutter::Settings settings,
                     std::shared_ptr<PlatformViewAndroidJNI> jni_facade,
                     bool is_background_view);

  ~AndroidShellHolder();

  bool IsValid() const;

  //----------------------------------------------------------------------------
  /// @brief      This is a factory for a derived AndroidShellHolder from an
  ///             existing AndroidShellHolder.
  ///
  /// @details    Creates one Shell from another Shell where the created
  ///             Shell takes the opportunity to share any internal components
  ///             it can. This results is a Shell that has a smaller startup
  ///             time cost and a smaller memory footprint than an Shell created
  ///             with a Create function.
  ///
  ///             The new Shell is returned in a new AndroidShellHolder
  ///             instance.
  ///
  ///             The new Shell's flutter::Settings cannot be changed from that
  ///             of the initial Shell. The RunConfiguration subcomponent can
  ///             be changed however in the spawned Shell to run a different
  ///             entrypoint than the existing shell.
  ///
  ///             Since the AndroidShellHolder both binds downwards to a Shell
  ///             and also upwards to JNI callbacks that the PlatformViewAndroid
  ///             makes, the JNI instance holding this AndroidShellHolder should
  ///             be created first to supply the jni_facade callback.
  ///
  /// @param[in]  jni_facade this argument should be the JNI callback facade of
  ///             a new JNI instance meant to hold this AndroidShellHolder.
  ///
  /// @returns    A new AndroidShellHolder containing a new Shell. Returns
  ///             nullptr when a new Shell can't be created.
  ///
  std::unique_ptr<AndroidShellHolder> Spawn(
      std::shared_ptr<PlatformViewAndroidJNI> jni_facade,
      const std::string& entrypoint,
      const std::string& libraryUrl) const;

  void Launch(std::shared_ptr<AssetManager> asset_manager,
              const std::string& entrypoint,
              const std::string& libraryUrl);

  const flutter::Settings& GetSettings() const;

  fml::WeakPtr<PlatformViewAndroid> GetPlatformView();

  Rasterizer::Screenshot Screenshot(Rasterizer::ScreenshotType type,
                                    bool base64_encode);

  void UpdateAssetManager(fml::RefPtr<flutter::AssetManager> asset_manager);

  void NotifyLowMemoryWarning();

  void ClearDirectByteBufferDecodingPreference(const std::string& channel);

  void SetDirectByteBufferDecodingPreference(
      const std::string& channel,
      bool wants_direct_byte_buffer_for_decoding);

 private:
  const flutter::Settings settings_;
  const std::shared_ptr<PlatformViewAndroidJNI> jni_facade_;
  fml::WeakPtr<PlatformViewAndroid> platform_view_;
  std::shared_ptr<ThreadHost> thread_host_;
  std::unique_ptr<Shell> shell_;
  bool is_valid_ = false;
  uint64_t next_pointer_flow_id_ = 0;
  std::shared_ptr<AssetManager> asset_manager_;

  //----------------------------------------------------------------------------
  /// @brief      Constructor with its components injected.
  ///
  /// @details    This is similar to the standard constructor, except its
  ///             members were constructed elsewhere and injected.
  ///
  ///             All injected components must be non-null and valid.
  ///
  ///             Used when constructing the Shell from the inside out when
  ///             spawning from an existing Shell.
  ///
  AndroidShellHolder(const flutter::Settings& settings,
                     const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade,
                     const std::shared_ptr<ThreadHost>& thread_host,
                     std::unique_ptr<Shell> shell,
                     const fml::WeakPtr<PlatformViewAndroid>& platform_view);
  static void ThreadDestructCallback(void* value);
  std::optional<RunConfiguration> BuildRunConfiguration(
      std::shared_ptr<flutter::AssetManager> asset_manager,
      const std::string& entrypoint,
      const std::string& libraryUrl) const;

  FML_DISALLOW_COPY_AND_ASSIGN(AndroidShellHolder);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SHELL_HOLDER_H_
