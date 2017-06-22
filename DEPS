# This file is automatically processed to create .DEPS.git which is the file
# that gclient uses under git.
#
# See http://code.google.com/p/chromium/wiki/UsingGit
#
# To test manually, run:
#   python tools/deps2git/deps2git.py -o .DEPS.git -w <gclientdir>
# where <gcliendir> is the absolute path to the directory containing the
# .gclient file (the parent of 'src').
#
# Then commit .DEPS.git locally (gclient doesn't like dirty trees) and run
#   gclient sync
# Verify the thing happened you wanted. Then revert your .DEPS.git change
# DO NOT CHECK IN CHANGES TO .DEPS.git upstream. It will be automatically
# updated by a bot when you modify this one.
#
# When adding a new dependency, please update the top-level .gitignore file
# to list the dependency's destination directory.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'fuchsia_git': 'https://fuchsia.googlesource.com',
  'skia_git': 'https://skia.googlesource.com',
  'github_git': 'https://github.com',
  'skia_revision': 'efa48d599de15d960d7335a4a93a6bbeb97d3c41',

  # When updating the Dart revision, ensure that all entries that are
  # dependencies of Dart are also updated to match the entries in the
  # Dart SDK's DEPS file for that revision of Dart. The DEPS file for
  # Dart is: https://github.com/dart-lang/sdk/blob/master/DEPS
  'dart_revision': '65a5707189dec6c5e0174cbc821acfb90f0c9b3f',
  # Dart calls the next one "boringssl_gen_rev"
  'dart_boringssl_gen_revision': '753224969dbe43dad29343146529727b5066c0f3',
  # Dart calls the next one "boringssl_rev"
  'dart_boringssl_revision': 'd519bf6be0b447fb80fbc539d4bff4479b5482a2',
  # Dart calls the next one "observatory_pub_packages_rev"
  'dart_observatory_packages_revision': '26aad88f1c1915d39bbcbff3cad589e2402fdcf1',
  # Dart calls the next one "args_tag"
  'dart_args_revision':
  '0.13.7',
  # Dart calls the next one "async_tag"
  'dart_async_revision':
  'daf66909019d2aaec1721fc39d94ea648a9fdc1d',
  # Dart calls the next one "collection_tag"
  'dart_collection_revision':
  '1.13.0',
  # Dart calls the next one "charcode_tag"
  'dart_charcode_revision':
  'v1.1.1',
  # Dart calls the next one "package_config_tag"
  'dart_package_config_revision':
  '1.0.0',
  # Dart calls the next one "convert_tag"
  'dart_convert_revision':
  '2.0.1',
  # Dart calls the next one "crypto_tag"
  'dart_crypto_revision':
  '2.0.1',
  # Dart calls the next one "csslib_tag"
  'dart_csslib_revision':
  '0.13.3+1',
  # Dart calls the next one "dart2js_info_tag"
  "dart_dart2js_info_revision":
  "0.5.4+2",
  # Dart calls the next one "isolate_tag"
  'dart_isolate_revision':
  '1.0.0',
  # Dart calls the next one "glob_tag"
  'dart_glob_revision':
  '1.1.3',
  # Dart calls the next one "html_tag"
  'dart_html_revision':
  '0.13.1',
  # Dart calls the next one "logging_tag"
  'dart_logging_revision':
  "0.11.3+1",
  # Dart calls the next one "path_tag"
  'dart_path_revision':
  "1.4.1",
  # Dart calls the next one "plugin_tag"
  'dart_plugin_revision':
  '0.2.0',
  # Dart calls the next one "pub_semver_tag"
  "dart_pub_semver_revision":
  "1.3.2",
  # Dart calls the next one "source_span_tag"
  'dart_source_span_revision':
  '1.3.1',
  # Dart calls the next one "string_scanner_tag"
  'dart_string_scanner_revision':
  '1.0.1',
  # Dart calls the next one "typed_data_tag"
  'dart_typed_data_revision':
  '1.1.3',
  # Dart calls the next one "utf_tag"
  'dart_utf_revision':
  '0.9.0+3',
  # Dart calls the next one "watcher_tag"
  'dart_watcher_revision':
  '0.9.7+3',
  # Dart calls the next one "yaml_tag"
  'dart_yaml_revision':
  '2.1.12',
  # Dart calls the next one "root_certificates_rev"
  'dart_root_certificates_revision': 'a4c7c6f23a664a37bc1b6f15a819e3f2a292791a',

  # Build bot tooling for iOS
  'ios_tools_revision': '69b7c1b160e7107a6a98d948363772dc9caea46f',

  'buildtools_revision': 'c8db819853bcf8ce1635a8b7a395820f39b5a9fc',
}

# Only these hosts are allowed for dependencies in this DEPS file.
# If you need to add a new host, contact chrome infrastructure team.
allowed_hosts = [
  'chromium.googlesource.com',
  'fuchsia.googlesource.com',
  'github.com',
  'skia.googlesource.com',
]

deps = {
  'src': 'https://github.com/flutter/buildroot.git' + '@' + 'a37901d9b7b2051d315e0b1c63427e2cf46bacae',

   # Fuchsia compatibility
   #
   # The dependencies in this section should match the layout in the Fuchsia gn
   # build. Eventually, we'll manage these dependencies together with Fuchsia
   # and not have to specific specific hashes.

  'src/lib/ftl':
   Var('fuchsia_git') + '/ftl' + '@' + '81aa1ca480c99caffbc2965deb0a6f7ac7f59f1c',

  'src/lib/tonic':
   Var('fuchsia_git') + '/tonic' + '@' + '5b3d521980ca00274ad7e67f9f8b203cd4b20039',

  'src/lib/zip':
   Var('fuchsia_git') + '/zip' + '@' + '92dc87ca645fe8e9f5151ef6dac86d8311a7222f',

  'src/third_party/gtest':
   Var('fuchsia_git') + '/third_party/gtest' + '@' + 'c00f82917331efbbd27124b537e4ccc915a02b72',

  'src/third_party/rapidjson':
   Var('fuchsia_git') + '/third_party/rapidjson' + '@' + '9defbb0209a534ffeb3a2b79d5ee440a77407292',

   # Chromium-style
   #
   # As part of integrating with Fuchsia, we should eventually remove all these
   # Chromium-style dependencies.

  'src/buildtools':
   Var('fuchsia_git') + '/buildtools' + '@' +  Var('buildtools_revision'),

   'src/ios_tools':
   Var('chromium_git') + '/chromium/src/ios.git' + '@' + Var('ios_tools_revision'),

  'src/third_party/icu':
   Var('chromium_git') + '/chromium/deps/icu.git' + '@' + 'c3f79166089e5360c09e3053fce50e6e296c3204',

  'src/dart':
    Var('chromium_git') + '/external/github.com/dart-lang/sdk.git' + '@' + Var('dart_revision'),

  'src/third_party/boringssl':
    Var('github_git') + '/dart-lang/boringssl_gen.git' + '@' + Var('dart_boringssl_gen_revision'),

  'src/third_party/boringssl/src':
   'https://boringssl.googlesource.com/boringssl.git' + '@' + Var('dart_boringssl_revision'),

  'src/dart/third_party/observatory_pub_packages':
   Var('chromium_git') +
   '/external/github.com/dart-lang/observatory_pub_packages' + '@' +
   Var('dart_observatory_packages_revision'),

  'src/dart/third_party/pkg/args':
  Var('chromium_git') +
  '/external/github.com/dart-lang/args' + '@' +
  Var('dart_args_revision'),

  'src/dart/third_party/pkg/async':
  Var('chromium_git') +
  '/external/github.com/dart-lang/async' + '@' +
  Var('dart_async_revision'),

  'src/dart/third_party/pkg/charcode':
  Var('chromium_git') +
  '/external/github.com/dart-lang/charcode' + '@' +
  Var('dart_charcode_revision'),

  'src/dart/third_party/pkg/collection':
  Var('chromium_git') +
  '/external/github.com/dart-lang/collection' + '@' +
  Var('dart_collection_revision'),

  'src/dart/third_party/pkg/convert':
  Var('chromium_git') +
  '/external/github.com/dart-lang/convert' + '@' +
  Var('dart_convert_revision'),

  'src/dart/third_party/pkg/crypto':
  Var('chromium_git') +
  '/external/github.com/dart-lang/crypto' + '@' +
  Var('dart_crypto_revision'),

  'src/dart/third_party/pkg/csslib':
  Var('chromium_git') +
  '/external/github.com/dart-lang/csslib' + '@' +
  Var('dart_csslib_revision'),

  'src/dart/third_party/pkg/dart2js_info':
  Var('chromium_git') +
  '/external/github.com/dart-lang/dart2js_info' + '@' +
  Var('dart_dart2js_info_revision'),

  'src/dart/third_party/pkg/isolate':
  Var('chromium_git') +
  '/external/github.com/dart-lang/isolate' + '@' +
  Var('dart_isolate_revision'),

  'src/dart/third_party/pkg/glob':
  Var('chromium_git') +
  '/external/github.com/dart-lang/glob' + '@' +
  Var('dart_glob_revision'),

  'src/dart/third_party/pkg/html':
  Var('chromium_git') +
  '/external/github.com/dart-lang/html' + '@' +
  Var('dart_html_revision'),

  'src/dart/third_party/pkg/logging':
  Var('chromium_git') +
  '/external/github.com/dart-lang/logging' + '@' +
  Var('dart_logging_revision'),

  'src/dart/third_party/pkg_tested/package_config':
  Var('chromium_git') +
  '/external/github.com/dart-lang/package_config' + '@' +
  Var('dart_package_config_revision'),

  'src/dart/third_party/pkg/path':
  Var('chromium_git') +
  '/external/github.com/dart-lang/path' + '@' +
  Var('dart_path_revision'),

  'src/dart/third_party/pkg/plugin':
  Var('chromium_git') +
  '/external/github.com/dart-lang/plugin' + '@' +
  Var('dart_plugin_revision'),

  'src/dart/third_party/pkg/pub_semver':
  Var('chromium_git') +
  '/external/github.com/dart-lang/pub_semver' + '@' +
  Var('dart_pub_semver_revision'),

  'src/dart/third_party/pkg/source_span':
  Var('chromium_git') +
  '/external/github.com/dart-lang/source_span' + '@' +
  Var('dart_source_span_revision'),

  'src/dart/third_party/pkg/string_scanner':
  Var('chromium_git') +
  '/external/github.com/dart-lang/string_scanner' + '@' +
  Var('dart_string_scanner_revision'),

  'src/dart/third_party/pkg/typed_data':
  Var('chromium_git') +
  '/external/github.com/dart-lang/typed_data' + '@' +
  Var('dart_typed_data_revision'),

  'src/dart/third_party/pkg/utf':
  Var('chromium_git') +
  '/external/github.com/dart-lang/utf' + '@' +
  Var('dart_utf_revision'),

  'src/dart/third_party/pkg/watcher':
  Var('chromium_git') +
  '/external/github.com/dart-lang/watcher' + '@' +
  Var('dart_watcher_revision'),

  'src/dart/third_party/pkg/yaml':
  Var('chromium_git') +
  '/external/github.com/dart-lang/yaml' + '@' +
  Var('dart_yaml_revision'),

  'src/third_party/root_certificates':
   Var('chromium_git') +
   '/external/github.com/dart-lang/root_certificates' + '@' +
   Var('dart_root_certificates_revision'),

  'src/third_party/skia':
   Var('skia_git') + '/skia.git' + '@' +  Var('skia_revision'),

  'src/third_party/libjpeg-turbo':
   Var('skia_git') + '/third_party/libjpeg-turbo.git' + '@' + 'debddedc75850bcdeb8a57258572f48b802a4bb3',

  'src/third_party/gyp':
   Var('chromium_git') + '/external/gyp.git' + '@' + '6ee91ad8659871916f9aa840d42e1513befdf638',

   # Headers for Vulkan 1.0
   'src/third_party/vulkan':
   Var('github_git') + '/KhronosGroup/Vulkan-Docs.git' + '@' + 'e29c2489e238509c41aeb8c7bce9d669a496344b',
}

recursedeps = [
  'src/buildtools',
]

deps_os = {
  'android': {
    'src/third_party/colorama/src':
     Var('chromium_git') + '/external/colorama.git' + '@' + '799604a1041e9b3bc5d2789ecbd7e8db2e18e6b8',

    'src/third_party/freetype2':
       Var('fuchsia_git') + '/third_party/freetype2' + '@' + 'e23a030e9b43c648249477fdf7bf5305d2cc8f59',
  },
}


hooks = [
  {
    # This clobbers when necessary (based on get_landmines.py). It must be the
    # first hook so that other things that get/generate into the output
    # directory will not subsequently be clobbered.
    'name': 'landmines',
    'pattern': '.',
    'action': [
        'python',
        'src/build/landmines.py',
    ],
  },
  {
    # Update the Windows toolchain if necessary.
    'name': 'win_toolchain',
    'pattern': '.',
    'action': ['python', 'src/build/vs_toolchain.py', 'update'],
  },
  {
    'name': 'download_android_tools',
    'pattern': '.',
    'action': [
        'python',
        'src/tools/android/download_android_tools.py',
    ],
  },
  {
    'name': 'buildtools',
    'pattern': '.',
    'action': [
      'python',
      'src/tools/buildtools/update.py',
      '--ninja',
      '--gn',
      '--toolchain'
    ],
  },
  {
    # Pull dart sdk if needed
    'name': 'dart',
    'pattern': '.',
    'action': ['python', 'src/tools/dart/update.py'],
  },
  {
    # Ensure that we don't accidentally reference any .pyc files whose
    # corresponding .py files have already been deleted.
    'name': 'remove_stale_pyc_files',
    'pattern': 'src/tools/.*\\.py',
    'action': [
        'python',
        'src/tools/remove_stale_pyc_files.py',
        'src/tools',
    ],
  },
]
