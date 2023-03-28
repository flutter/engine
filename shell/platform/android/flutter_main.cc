// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/shell/platform/android/flutter_main.h"

#include <android/log.h>

#include <optional>
#include <vector>

#include "flutter/fml/command_line.h"
#include "flutter/fml/file.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/message_loop.h"
#include "flutter/fml/native_library.h"
#include "flutter/fml/paths.h"
#include "flutter/fml/platform/android/jni_util.h"
#include "flutter/fml/platform/android/paths_android.h"
#include "flutter/fml/size.h"
#include "flutter/lib/ui/plugins/callback_cache.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/shell/common/shell.h"
#include "flutter/shell/common/switches.h"
#include "third_party/dart/runtime/include/dart_tools_api.h"
#include "third_party/skia/include/core/SkFontMgr.h"

#include "updater.h"

namespace flutter {

extern "C" {
#if FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DEBUG
// Used for debugging dart:* sources.
extern const uint8_t kPlatformStrongDill[];
extern const intptr_t kPlatformStrongDillSize;
#endif
}

namespace {

// This is only available on API 23+, so dynamically look it up.
// This method is only called once at shell creation.
// Do this in C++ because the API is available at level 23 here, but only 29+ in
// Java.
bool IsATraceEnabled() {
  auto libandroid = fml::NativeLibrary::Create("libandroid.so");
  FML_CHECK(libandroid);
  auto atrace_fn =
      libandroid->ResolveFunction<bool (*)(void)>("ATrace_isEnabled");
  if (atrace_fn) {
    return atrace_fn.value()();
  }
  return false;
}

fml::jni::ScopedJavaGlobalRef<jclass>* g_flutter_jni_class = nullptr;

}  // anonymous namespace

FlutterMain::FlutterMain(const flutter::Settings& settings)
    : settings_(settings), vm_service_uri_callback_() {}

FlutterMain::~FlutterMain() = default;

static std::unique_ptr<FlutterMain> g_flutter_main;

FlutterMain& FlutterMain::Get() {
  FML_CHECK(g_flutter_main) << "ensureInitializationComplete must have already "
                               "been called.";
  return *g_flutter_main;
}

const flutter::Settings& FlutterMain::GetSettings() const {
  return settings_;
}

// Old Android versions (e.g. the v16 ndk Flutter uses) don't always include a
// getauxval symbol, but the Rust ring crate assumes it exists:
// https://github.com/briansmith/ring/blob/fa25bf3a7403c9fe6458cb87bd8427be41225ca2/src/cpu/arm.rs#L22
// It uses it to determine if the CPU supports AES instructions.
// Making this a weak symbol allows the linker to use a real version instead
// if it can find one.
// BoringSSL just reads from procfs instead, which is what we would do if
// we needed to implement this ourselves.  Implementation looks straightforward:
// https://lwn.net/Articles/519085/
// https://github.com/google/boringssl/blob/6ab4f0ae7f2db96d240eb61a5a8b4724e5a09b2f/crypto/cpu_arm_linux.c
#if defined(__ANDROID__) && defined(__arm__)
extern "C" __attribute__((weak)) unsigned long getauxval(unsigned long type) {
  return 0;
}
#endif

// TODO: Move this out into a separate file?
void ConfigureShorebird(std::string android_cache_path,
                        flutter::Settings& settings,
                        std::string shorebirdYaml,
                        std::string version) {
  auto cache_dir =
      fml::paths::JoinPaths({android_cache_path, "shorebird_updater"});

  fml::CreateDirectory(fml::paths::GetCachesDirectory(), {"shorebird_updater"},
                       fml::FilePermission::kReadWrite);

  AppParameters app_parameters;
  app_parameters.release_version = version.c_str();
  app_parameters.cache_dir = cache_dir.c_str();

  // This seems to be ['libapp.so', 'real/path/to/libapp.so']
  // not sure why, but we need to use the real path (second entry).
  app_parameters.original_libapp_path =
      settings.application_library_path.back().c_str();
  // TODO: How do can we get the path to libflutter.so?
  // The Rust side doesn't actually use this yet.  The intent is so that
  // Rust could hash it, or otherwise know that it matches the version the patch
  // is intended for, but currently that's not implemented.
  app_parameters.vm_path = "libflutter.so";

  shorebird_init(&app_parameters, shorebirdYaml.c_str());

  FML_LOG(INFO) << "Starting Shorebird update";
  shorebird_update();

  char* c_active_path = shorebird_active_path();
  if (c_active_path != NULL) {
    std::string active_path = c_active_path;
    shorebird_free_string(c_active_path);
    FML_LOG(INFO) << "Shorebird updater: active path: " << active_path;

    settings.application_library_path.clear();
    settings.application_library_path.emplace_back(active_path);

    char* c_patch_number = shorebird_active_patch_number();
    if (c_patch_number != NULL) {
      std::string patch_number = c_patch_number;
      shorebird_free_string(c_patch_number);
      FML_LOG(INFO) << "Shorebird updater: active patch number: "
                    << patch_number;
    }
  } else {
    FML_LOG(INFO) << "Shorebird updater: no active patch.";
  }
}

void FlutterMain::Init(JNIEnv* env,
                       jclass clazz,
                       jobject context,
                       jobjectArray jargs,
                       jstring kernelPath,
                       jstring appStoragePath,
                       jstring engineCachesPath,
                       jstring shorebirdYaml,
                       jstring version,
                       jlong initTimeMillis) {
  std::vector<std::string> args;
  args.push_back("flutter");
  for (auto& arg : fml::jni::StringArrayToVector(env, jargs)) {
    args.push_back(std::move(arg));
  }
  auto command_line = fml::CommandLineFromIterators(args.begin(), args.end());

  auto settings = SettingsFromCommandLine(command_line);

  // Turn systracing on if ATrace_isEnabled is true and the user did not already
  // request systracing
  if (!settings.trace_systrace) {
    settings.trace_systrace = IsATraceEnabled();
    if (settings.trace_systrace) {
      __android_log_print(
          ANDROID_LOG_INFO, "Flutter",
          "ATrace was enabled at startup. Flutter and Dart "
          "tracing will be forwarded to systrace and will not show up in "
          "Dart DevTools.");
    }
  }

#if FLUTTER_RELEASE
  // On most platforms the timeline is always disabled in release mode.
  // On Android, enable it in release mode only when using systrace.
  settings.enable_timeline_event_handler = settings.trace_systrace;
#endif  // FLUTTER_RELEASE

  // Restore the callback cache.
  // TODO(chinmaygarde): Route all cache file access through FML and remove this
  // setter.
  flutter::DartCallbackCache::SetCachePath(
      fml::jni::JavaStringToString(env, appStoragePath));

  auto android_cache_path = fml::jni::JavaStringToString(env, engineCachesPath);
  fml::paths::InitializeAndroidCachesPath(android_cache_path);

#if FLUTTER_RELEASE
  std::string shorebird_yaml = fml::jni::JavaStringToString(env, shorebirdYaml);
  std::string version_string = fml::jni::JavaStringToString(env, version);
  ConfigureShorebird(android_cache_path, settings, shorebird_yaml,
                     version_string);
#endif

  flutter::DartCallbackCache::LoadCacheFromDisk();

  if (!flutter::DartVM::IsRunningPrecompiledCode() && kernelPath) {
    // Check to see if the appropriate kernel files are present and configure
    // settings accordingly.
    auto application_kernel_path =
        fml::jni::JavaStringToString(env, kernelPath);

    if (fml::IsFile(application_kernel_path)) {
      settings.application_kernel_asset = application_kernel_path;
    }
  }

  settings.task_observer_add = [](intptr_t key, const fml::closure& callback) {
    fml::MessageLoop::GetCurrent().AddTaskObserver(key, callback);
  };

  settings.task_observer_remove = [](intptr_t key) {
    fml::MessageLoop::GetCurrent().RemoveTaskObserver(key);
  };

  settings.log_message_callback = [](const std::string& tag,
                                     const std::string& message) {
    __android_log_print(ANDROID_LOG_INFO, tag.c_str(), "%.*s",
                        static_cast<int>(message.size()), message.c_str());
  };

#if FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DEBUG
  // There are no ownership concerns here as all mappings are owned by the
  // embedder and not the engine.
  auto make_mapping_callback = [](const uint8_t* mapping, size_t size) {
    return [mapping, size]() {
      return std::make_unique<fml::NonOwnedMapping>(mapping, size);
    };
  };

  settings.dart_library_sources_kernel =
      make_mapping_callback(kPlatformStrongDill, kPlatformStrongDillSize);
#endif  // FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DEBUG

  // Not thread safe. Will be removed when FlutterMain is refactored to no
  // longer be a singleton.
  g_flutter_main.reset(new FlutterMain(settings));

  g_flutter_main->SetupDartVMServiceUriCallback(env);
}

void FlutterMain::SetupDartVMServiceUriCallback(JNIEnv* env) {
  g_flutter_jni_class = new fml::jni::ScopedJavaGlobalRef<jclass>(
      env, env->FindClass("io/flutter/embedding/engine/FlutterJNI"));
  if (g_flutter_jni_class->is_null()) {
    return;
  }
  jfieldID uri_field = env->GetStaticFieldID(
      g_flutter_jni_class->obj(), "vmServiceUri", "Ljava/lang/String;");
  if (uri_field == nullptr) {
    return;
  }

  auto set_uri = [env, uri_field](const std::string& uri) {
    fml::jni::ScopedJavaLocalRef<jstring> java_uri =
        fml::jni::StringToJavaString(env, uri);
    env->SetStaticObjectField(g_flutter_jni_class->obj(), uri_field,
                              java_uri.obj());
  };

  fml::MessageLoop::EnsureInitializedForCurrentThread();
  fml::RefPtr<fml::TaskRunner> platform_runner =
      fml::MessageLoop::GetCurrent().GetTaskRunner();

  vm_service_uri_callback_ = DartServiceIsolate::AddServerStatusCallback(
      [platform_runner, set_uri](const std::string& uri) {
        platform_runner->PostTask([uri, set_uri] { set_uri(uri); });
      });
}

static void PrefetchDefaultFontManager(JNIEnv* env, jclass jcaller) {
  // Initialize a singleton owned by Skia.
  SkFontMgr::RefDefault();
}

bool FlutterMain::Register(JNIEnv* env) {
  static const JNINativeMethod methods[] = {
      {
          .name = "nativeInit",
          .signature = "(Landroid/content/Context;[Ljava/lang/String;Ljava/"
                       "lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/"
                       "lang/String;Ljava/lang/String;J)V",
          .fnPtr = reinterpret_cast<void*>(&Init),
      },
      {
          .name = "nativePrefetchDefaultFontManager",
          .signature = "()V",
          .fnPtr = reinterpret_cast<void*>(&PrefetchDefaultFontManager),
      },
  };

  jclass clazz = env->FindClass("io/flutter/embedding/engine/FlutterJNI");

  if (clazz == nullptr) {
    return false;
  }

  return env->RegisterNatives(clazz, methods, fml::size(methods)) == 0;
}

}  // namespace flutter
