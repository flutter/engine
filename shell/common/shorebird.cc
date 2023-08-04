
#include "flutter/shell/common/shorebird.h"

#include <optional>
#include <vector>

#include "flutter/fml/command_line.h"
#include "flutter/fml/file.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/message_loop.h"
#include "flutter/fml/native_library.h"
#include "flutter/fml/paths.h"
#include "flutter/fml/size.h"
#include "flutter/lib/ui/plugins/callback_cache.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/shell/common/shell.h"
#include "flutter/shell/common/switches.h"
#include "third_party/dart/runtime/include/dart_tools_api.h"
#include "third_party/skia/include/core/SkFontMgr.h"

#include "third_party/updater/library/include/updater.h"

// Namespaced to avoid Google style warnings.
namespace flutter {

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

void ConfigureShorebird(std::string cache_path,
                        flutter::Settings& settings,
                        const std::string& shorebird_yaml,
                        const std::string& version,
                        const std::string& version_code) {
  auto cache_dir =
      fml::paths::JoinPaths({std::move(cache_path), "shorebird_updater"});

  fml::CreateDirectory(fml::paths::GetCachesDirectory(), {"shorebird_updater"},
                       fml::FilePermission::kReadWrite);

  // Using a block to make AppParameters lifetime explicit.
  {
    AppParameters app_parameters;
    // Combine version and version_code into a single string.
    // We could also pass these separately through to the updater if needed.
    auto release_version = version + "+" + version_code;
    app_parameters.release_version = release_version.c_str();
    app_parameters.cache_dir = cache_dir.c_str();

    // https://stackoverflow.com/questions/26032039/convert-vectorstring-into-char-c
    std::vector<const char*> c_paths{};
    for (const auto& string : settings.application_library_path) {
      c_paths.push_back(string.c_str());
    }
    // Do not modify application_library_path or c_strings will invalidate.

    app_parameters.original_libapp_paths = c_paths.data();
    app_parameters.original_libapp_paths_size = c_paths.size();

    // shorebird_init copies from app_parameters and shorebirdYaml.
    shorebird_init(&app_parameters, shorebird_yaml.c_str());
  }

  // We've decided not to support synchronous updates on launch for now.
  // It's a terrible user experience (having the app hang on launch) and
  // instead we will provide examples of how to build a custom update UI
  // within Dart, including updating as part of login, etc.
  // https://github.com/shorebirdtech/shorebird/issues/950

  char* c_active_path = shorebird_next_boot_patch_path();
  if (c_active_path != NULL) {
    std::string active_path = c_active_path;
    shorebird_free_string(c_active_path);
    FML_LOG(INFO) << "Shorebird updater: active path: " << active_path;
    size_t c_patch_number = shorebird_next_boot_patch_number();
    // FIXME: use a constant for this.
    // 0 means no patch is available.
    if (c_patch_number != 0) {
      FML_LOG(INFO) << "Shorebird updater: active patch number: "
                    << c_patch_number;
    }

    settings.application_library_path.clear();
    settings.application_library_path.emplace_back(active_path);
    // Once start_update_thread is called, the next_boot_patch* functions may
    // change their return values if the shorebird_report_launch_failed
    // function is called.
    shorebird_report_launch_start();
  } else {
    FML_LOG(INFO) << "Shorebird updater: no active patch.";
  }

  if (shorebird_should_auto_update()) {
    FML_LOG(INFO) << "Starting Shorebird update";
    shorebird_start_update_thread();
  } else {
    FML_LOG(INFO)
        << "Shorebird auto_update disabled, not checking for updates.";
  }
}

}  // namespace flutter