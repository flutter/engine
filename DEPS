# The dependencies referenced by the Flutter Engine.
#
# This file is referenced by the .gclient file at the root of the checkout.
# To preview changes to the dependencies, update this file and run
# `gclient sync`.
#
# When adding a new dependency, please update the top-level .gitignore file
# to list the dependency's destination directory.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'dart_git': 'https://dart.googlesource.com',
  'flutter_git': 'https://flutter.googlesource.com',
  'fuchsia_git': 'https://fuchsia.googlesource.com',
  'github_git': 'https://github.com',
  'skia_git': 'https://skia.googlesource.com',
  'llvm_git': 'https://llvm.googlesource.com',
  # OCMock is for testing only so there is no google clone
  'ocmock_git': 'https://github.com/erikdoe/ocmock.git',
  'skia_revision': '4e218a8fe79a335b65106ed6a71d4166335189c6',

  # WARNING: DO NOT EDIT canvaskit_cipd_instance MANUALLY
  # See `lib/web_ui/README.md` for how to roll CanvasKit to a new version.
  'canvaskit_cipd_instance': '61aeJQ9laGfEFF_Vlc_u0MCkqB6xb2hAYHRBxKH-Uw4C',

  # Do not download the Emscripten SDK by default.
  # This prevents us from downloading the Emscripten toolchain for builds
  # which do not build for the web. This toolchain is needed to build CanvasKit
  # for the web engine.
  'download_emsdk': False,

  # For experimental features some dependencies may only be avaialable in the master/main
  # channels. This variable is being set when CI is checking out the repository.
  'release_candidate': False,


  # As Dart does, we use Fuchsia's GN and Clang toolchain. These revision
  # should be kept up to date with the revisions pulled by Dart.
  # The list of revisions for these tools comes from Fuchsia, here:
  # https://fuchsia.googlesource.com/integration/+/HEAD/toolchain
  # If there are problems with the toolchain, contact fuchsia-toolchain@.
  'clang_version': 'git_revision:20d06c833d833ef6b2d0f519cc4a7998d49a2803',

  # When updating the Dart revision, ensure that all entries that are
  # dependencies of Dart are also updated to match the entries in the
  # Dart SDK's DEPS file for that revision of Dart. The DEPS file for
  # Dart is: https://github.com/dart-lang/sdk/blob/main/DEPS
  # You can use //tools/dart/create_updated_flutter_deps.py to produce
  # updated revision list of existing dependencies.
  'dart_revision': 'c766fffb626ecac3be237703d4c68178f308e5f3',

  # WARNING: DO NOT EDIT MANUALLY
  # The lines between blank lines above and below are generated by a script. See create_updated_flutter_deps.py
  'dart_binaryen_rev': '4471b81a0a0b94c75bad6e81d0413860944ecb1f',
  'dart_boringssl_gen_rev': 'ced85ef0a00bbca77ce5a91261a5f2ae61b1e62f',
  'dart_boringssl_rev': '87f316d7748268eb56f2dc147bd593254ae93198',
  'dart_browser_launcher_rev': 'bc2dc4ef53a6619032e9f01450848e9ed40a9437',
  'dart_clock_rev': '5abb4816bad6cf3bfab64d65d501752d19760bd0',
  'dart_collection_rev': '0d0e184a5f70ca4e23dab23df686127ec40bfb11',
  'dart_devtools_rev': 'bf15e7348d53dc83531d503be94e0c035b604984',
  'dart_libprotobuf_rev': '24487dd1045c7f3d64a21f38a3f0c06cc4cf2edb',
  'dart_perfetto_rev': 'b8da07095979310818f0efde2ef3c69ea70d62c5',
  'dart_protobuf_gn_rev': 'f872f05cb0378eef9a7a2609076929f0f35b4141',
  'dart_protobuf_rev': '75bc380a6e9601565606a13dee16c5ab2bf5019c',
  'dart_pub_rev': '048e3ad2b5e1b4ebe6883addbc95722be6904a7b',
  'dart_root_certificates_rev': '692f6d6488af68e0121317a9c2c9eb393eb0ee50',
  'dart_watcher_rev': '5968409e1d73e21e75fc22a6481d5efaef7f3558',
  'dart_webdev_rev': 'cfe97534b33d37c7e0bbac2a3576085f74f2eaa3',
  'dart_webkit_inspection_protocol_rev': '8401098ace995e4dcd9855a2741c2dacccaa780b',
  'dart_yaml_edit_rev': '6abc42a74b76700ad02fa1c1ebd94951b62626b9',
  'dart_zlib_rev': '27c2f474b71d0d20764f86f60ef8b00da1a16cda',

  'ocmock_rev': 'c4ec0e3a7a9f56cfdbd0aa01f4f97bb4b75c5ef8', # v3.7.1

  # Download a prebuilt Dart SDK by default
  'download_dart_sdk': True,

  # Checkout Android dependencies only on platforms where we build for Android targets.
  'download_android_deps': 'host_os == "mac" or (host_os == "linux" and host_cpu == "x64")',

  # Checkout Windows dependencies only if we are building on Windows.
  'download_windows_deps' : 'host_os == "win"',

  # Checkout Linux dependencies only when building on Linux.
  'download_linux_deps': 'host_os == "linux"',

  # Downloads the fuchsia SDK as listed in fuchsia_sdk_path var. This variable
  # is currently only used for the Fuchsia LSC process and is not intended for
  # local development.
  'download_fuchsia_sdk': False,
  'fuchsia_sdk_path': '',

  # An LLVM backend needs LLVM binaries and headers. To avoid build time
  # increases we can use prebuilts. We don't want to download this on every
  # CQ/CI bot nor do we want the average Dart developer to incur that cost.
  # So by default we will not download prebuilts. This varible is needed in
  # the flutter engine to ensure that Dart gn has access to it as well.
  "checkout_llvm": False,

  # Setup Git hooks by default.
  "setup_githooks": True,

  # Upstream URLs for third party dependencies, used in
  # determining common ancestor commit for vulnerability scanning
  # prefixed with 'upstream_' in order to be identified by parsing tool.
  # The vulnerabiity database being used in this scan can be browsed
  # using this UI https://osv.dev/list
  # If a new dependency needs to be added, the upstream (non-mirrored)
  # git URL for that dependency should be added to this list 
  # with the key-value pair being:
  # 'upstream_[dep name from last slash and before .git in URL]':'[git URL]'
  # example: 
  "upstream_abseil-cpp": "https://github.com/abseil/abseil-cpp.git",
  "upstream_angle": "https://github.com/google/angle.git",
  "upstream_archive": "https://github.com/brendan-duncan/archive.git",
  "upstream_args": "https://github.com/dart-lang/args.git",
  "upstream_async": "https://github.com/dart-lang/async.git",
  "upstream_bazel_worker": "https://github.com/dart-lang/bazel_worker.git",
  "upstream_benchmark": "https://github.com/google/benchmark.git",
  "upstream_boolean_selector": "https://github.com/dart-lang/boolean_selector.git",
  "upstream_boringssl_gen": "https://github.com/dart-lang/boringssl_gen.git",
  "upstream_boringssl": "https://github.com/openssl/openssl.git",
  "upstream_browser_launcher": "https://github.com/dart-lang/browser_launcher.git",
  "upstream_buildroot": "https://github.com/flutter/buildroot.git",
  "upstream_cli_util": "https://github.com/dart-lang/cli_util.git",
  "upstream_clock": "https://github.com/dart-lang/clock.git",
  "upstream_collection": "https://github.com/dart-lang/collection.git",
  "upstream_colorama": "https://github.com/tartley/colorama.git",
  "upstream_convert": "https://github.com/dart-lang/convert.git",
  "upstream_crypto": "https://github.com/dart-lang/crypto.git",
  "upstream_csslib": "https://github.com/dart-lang/csslib.git",
  "upstream_dart_style": "https://github.com/dart-lang/dart_style.git",
  "upstream_dartdoc": "https://github.com/dart-lang/dartdoc.git",
  "upstream_equatable": "https://github.com/felangel/equatable.git",
  "upstream_ffi": "https://github.com/dart-lang/ffi.git",
  "upstream_file": "https://github.com/google/file.dart.git",
  "upstream_fixnum": "https://github.com/dart-lang/fixnum.git",
  "upstream_flatbuffers": "https://github.com/google/flatbuffers.git",
  "upstream_fontconfig": "https://gitlab.freedesktop.org/fontconfig/fontconfig.git",
  "upstream_freetype2": "https://gitlab.freedesktop.org/freetype/freetype.git",
  "upstream_gcloud": "https://github.com/dart-lang/gcloud.git",
  "upstream_glfw": "https://github.com/glfw/glfw.git",
  "upstream_glob": "https://github.com/dart-lang/glob.git",
  "upstream_googleapis": "https://github.com/google/googleapis.dart.git",
  "upstream_googletest": "https://github.com/google/googletest.git",
  "upstream_gtest-parallel": "https://github.com/google/gtest-parallel.git",
  "upstream_harfbuzz": "https://github.com/harfbuzz/harfbuzz.git",
  "upstream_html": "https://github.com/dart-lang/html.git",
  "upstream_http_multi_server": "https://github.com/dart-lang/http_multi_server.git",
  "upstream_http_parser": "https://github.com/dart-lang/http_parser.git",
  "upstream_http": "https://github.com/dart-lang/http.git",
  "upstream_icu": "https://github.com/unicode-org/icu.git",
  "upstream_intl": "https://github.com/dart-lang/intl.git",
  "upstream_imgui": "https://github.com/ocornut/imgui.git",
  "upstream_inja": "https://github.com/pantor/inja.git",
  "upstream_json": "https://github.com/nlohmann/json.git",
  "upstream_json_rpc_2": "https://github.com/dart-lang/json_rpc_2.git",
  "upstream_libcxx": "https://github.com/llvm-mirror/libcxx.git",
  "upstream_libcxxabi": "https://github.com/llvm-mirror/libcxxabi.git",
  "upstream_libexpat": "https://github.com/libexpat/libexpat.git",
  "upstream_libjpeg-turbo": "https://github.com/libjpeg-turbo/libjpeg-turbo.git",
  "upstream_libpng": "https://github.com/glennrp/libpng.git",
  "upstream_libtess2": "https://github.com/memononen/libtess2.git",
  "upstream_libwebp": "https://chromium.googlesource.com/webm/libwebp.git",
  "upstream_libxml": "https://gitlab.gnome.org/GNOME/libxml2.git",
  "upstream_linter": "https://github.com/dart-lang/linter.git",
  "upstream_logging": "https://github.com/dart-lang/logging.git",
  "upstream_markdown": "https://github.com/dart-lang/markdown.git",
  "upstream_matcher": "https://github.com/dart-lang/matcher.git",
  "upstream_mime": "https://github.com/dart-lang/mime.git",
  "upstream_mockito": "https://github.com/dart-lang/mockito.git",
  "upstream_oauth2": "https://github.com/dart-lang/oauth2.git",
  "upstream_ocmock": "https://github.com/erikdoe/ocmock.git",
  "upstream_package_config": "https://github.com/dart-lang/package_config.git",
  "upstream_packages": "https://github.com/flutter/packages.git",
  "upstream_path": "https://github.com/dart-lang/path.git",
  "upstream_platform": "https://github.com/google/platform.dart.git",
  "upstream_pool": "https://github.com/dart-lang/pool.git",
  "upstream_process_runner": "https://github.com/google/process_runner.git",
  "upstream_process": "https://github.com/google/process.dart.git",
  "upstream_protobuf": "https://github.com/google/protobuf.dart.git",
  "upstream_pub_semver": "https://github.com/dart-lang/pub_semver.git",
  "upstream_pub": "https://github.com/dart-lang/pub.git",
  "upstream_pyyaml": "https://github.com/yaml/pyyaml.git",
  "upstream_quiver-dart": "https://github.com/google/quiver-dart.git",
  "upstream_rapidjson": "https://github.com/Tencent/rapidjson.git",
  "upstream_root_certificates": "https://github.com/dart-lang/root_certificates.git",
  "upstream_sdk": "https://github.com/dart-lang/sdk.git",
  "upstream_shaderc": "https://github.com/google/shaderc.git",
  "upstream_shelf": "https://github.com/dart-lang/shelf.git",
  "upstream_skia": "https://skia.googlesource.com/skia.git",
  "upstream_source_map_stack_trace": "https://github.com/dart-lang/source_map_stack_trace.git",
  "upstream_source_maps": "https://github.com/dart-lang/source_maps.git",
  "upstream_source_span": "https://github.com/dart-lang/source_span.git",
  "upstream_sqlite": "https://github.com/sqlite/sqlite.git",
  "upstream_sse": "https://github.com/dart-lang/sse.git",
  "upstream_stack_trace": "https://github.com/dart-lang/stack_trace.git",
  "upstream_stream_channel": "https://github.com/dart-lang/stream_channel.git",
  "upstream_string_scanner": "https://github.com/dart-lang/string_scanner.git",
  "upstream_SwiftShader": "https://swiftshader.googlesource.com/SwiftShader.git",
  "upstream_term_glyph": "https://github.com/dart-lang/term_glyph.git",
  "upstream_test_reflective_loader": "https://github.com/dart-lang/test_reflective_loader.git",
  "upstream_test": "https://github.com/dart-lang/test.git",
  "upstream_tinygltf": "https://github.com/syoyo/tinygltf.git",
  "upstream_typed_data": "https://github.com/dart-lang/typed_data.git",
  "upstream_usage": "https://github.com/dart-lang/usage.git",
  "upstream_vector_math": "https://github.com/google/vector_math.dart.git",
  "upstream_Vulkan-Headers": "https://github.com/KhronosGroup/Vulkan-Headers.git",
  "upstream_VulkanMemoryAllocator": "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git",
  "upstream_watcher": "https://github.com/dart-lang/watcher.git",
  "upstream_web_socket_channel": "https://github.com/dart-lang/web_socket_channel.git",
  "upstream_webdev": "https://github.com/dart-lang/webdev.git",
  "upstream_webkit_inspection_protocol": "https://github.com/google/webkit_inspection_protocol.dart.git",
  "upstream_wuffs-mirror-release-c": "https://github.com/google/wuffs-mirror-release-c.git",
  "upstream_yaml_edit": "https://github.com/dart-lang/yaml_edit.git",
  "upstream_yaml": "https://github.com/dart-lang/yaml.git",
  "upstream_yapf": "https://github.com/google/yapf.git",
  "upstream_zlib": "https://github.com/madler/zlib.git",
}

gclient_gn_args_file = 'src/third_party/dart/build/config/gclient_args.gni'
gclient_gn_args = [
  'checkout_llvm'
]

# Only these hosts are allowed for dependencies in this DEPS file.
# If you need to add a new host, contact chrome infrastructure team.
allowed_hosts = [
  'boringssl.googlesource.com',
  'chrome-infra-packages.appspot.com',
  'chromium.googlesource.com',
  'dart.googlesource.com',
  'flutter.googlesource.com',
  'fuchsia.googlesource.com',
  'llvm.googlesource.com',
  'skia.googlesource.com',
  'swiftshader.googlesource.com',
]

deps = {
  'src': 'https://github.com/flutter/buildroot.git' + '@' + '287917d342224d34d6fbdf863aee8417a5f2e8b8',

   # Fuchsia compatibility
   #
   # The dependencies in this section should match the layout in the Fuchsia gn
   # build. Eventually, we'll manage these dependencies together with Fuchsia
   # and not have to specific hashes.

  'src/third_party/rapidjson':
   Var('fuchsia_git') + '/third_party/rapidjson' + '@' + 'ef3564c5c8824989393b87df25355baf35ff544b',

  'src/third_party/harfbuzz':
   Var('flutter_git') + '/third_party/harfbuzz' + '@' + 'd40d15e994ed60d32bcfc9ab87004dfb028dfbd6',

  'src/third_party/libcxx':
   Var('llvm_git') + '/llvm-project/libcxx' + '@' + '44079a4cc04cdeffb9cfe8067bfb3c276fb2bab0',

  'src/third_party/libcxxabi':
   Var('llvm_git') + '/llvm-project/libcxxabi' + '@' + '2ce528fb5e0f92e57c97ec3ff53b75359d33af12',

  'src/third_party/glfw':
   Var('fuchsia_git') + '/third_party/glfw' + '@' + 'dd8a678a66f1967372e5a5e3deac41ebf65ee127',

  'src/third_party/shaderc':
   Var('github_git') + '/google/shaderc.git' + '@' + '7ea834ecc59258a5c13c3d3e6fa0582bdde7c543',

  'src/third_party/vulkan-deps':
   Var('chromium_git') + '/vulkan-deps' + '@' + 'a56166d59ebdb0208a7301d267460773df1a1adc',

  'src/third_party/flatbuffers':
   Var('github_git') + '/google/flatbuffers.git' + '@' + '0a80646371179f8a7a5c1f42c31ee1d44dcf6709',

  'src/third_party/icu':
   Var('chromium_git') + '/chromium/deps/icu.git' + '@' + '1e49ac26ddc712b1ab702f69023cbc57e9ae6628',

  'src/third_party/khronos':
   Var('chromium_git') + '/chromium/src/third_party/khronos.git' + '@' + '676d544d2b8f48903b7da9fceffaa534a5613978',

   'src/third_party/gtest-parallel':
   Var('chromium_git') + '/external/github.com/google/gtest-parallel' + '@' + '38191e2733d7cbaeaef6a3f1a942ddeb38a2ad14',

  'src/third_party/benchmark':
   Var('github_git') + '/google/benchmark' + '@' + '431abd149fd76a072f821913c0340137cc755f36',

  'src/third_party/googletest':
   Var('github_git') + '/google/googletest' + '@' + '054a986a8513149e8374fc669a5fe40117ca6b41',

  'src/third_party/boringssl':
   Var('github_git') + '/dart-lang/boringssl_gen.git' + '@' + Var('dart_boringssl_gen_rev'),

  'src/third_party/yapf':
  Var('github_git') + '/google/yapf' + '@' + '212c5b5ad8e172d2d914ae454c121c89cccbcb35',

  'src/third_party/boringssl/src':
   'https://boringssl.googlesource.com/boringssl.git' + '@' + Var('dart_boringssl_rev'),

  'src/third_party/perfetto':
   Var('fuchsia_git') + "/third_party/android.googlesource.com/platform/external/perfetto"
   + '@' + Var('dart_perfetto_rev'),

  'src/third_party/protobuf':
   Var('fuchsia_git') + '/third_party/protobuf' + '@' + Var('dart_libprotobuf_rev'),

  'src/build/secondary/third_party/protobuf':
   Var('fuchsia_git') + '/protobuf-gn' + '@' + Var('dart_protobuf_gn_rev'),

  'src/third_party/dart':
   Var('dart_git') + '/sdk.git' + '@' + Var('dart_revision'),

  # WARNING: Unused Dart dependencies in the list below till "WARNING:" marker are removed automatically - see create_updated_flutter_deps.py.

  'src/third_party/dart/third_party/binaryen/src':
   Var('chromium_git') + '/external/github.com/WebAssembly/binaryen.git@4471b81a0a0b94c75bad6e81d0413860944ecb1f',

  'src/third_party/dart/third_party/devtools':
   {'packages': [{'version': 'git_revision:bf15e7348d53dc83531d503be94e0c035b604984', 'package': 'dart/third_party/flutter/devtools'}], 'dep_type': 'cipd'},

  'src/third_party/dart/third_party/pkg/args':
   Var('dart_git') + '/args.git@9305d5a2ca817a065a1ded3734655d99cd717f2c',

  'src/third_party/dart/third_party/pkg/async':
   Var('dart_git') + '/async.git@f454380c2691728530e0b2421af7676a9621481b',

  'src/third_party/dart/third_party/pkg/bazel_worker':
   Var('dart_git') + '/bazel_worker.git@53871c5fe36438b734a0f75c91055e75111a7210',

  'src/third_party/dart/third_party/pkg/boolean_selector':
   Var('dart_git') + '/boolean_selector.git@16e6ad3c8000e08a7f1c213aef2a692d872a16c7',

  'src/third_party/dart/third_party/pkg/browser_launcher':
   Var('dart_git') + '/browser_launcher.git' + '@' + Var('dart_browser_launcher_rev'),

  'src/third_party/dart/third_party/pkg/cli_util':
   Var('dart_git') + '/cli_util.git@91747f7dd9bdf2457f081d3cf035de5672dd4a02',

  'src/third_party/dart/third_party/pkg/clock':
   Var('dart_git') + '/clock.git' + '@' + Var('dart_clock_rev'),

  'src/third_party/dart/third_party/pkg/collection':
   Var('dart_git') + '/collection.git' + '@' + Var('dart_collection_rev'),

  'src/third_party/dart/third_party/pkg/convert':
   Var('dart_git') + '/convert.git@83886e3fd895211a98e49b7595a92796fa216641',

  'src/third_party/dart/third_party/pkg/crypto':
   Var('dart_git') + '/crypto.git@9efb888e9b06489bd7c861b1d01f4c4eebb05b60',

  'src/third_party/dart/third_party/pkg/csslib':
   Var('dart_git') + '/csslib.git@b671738dc6de7d212b5f943b4258f9bb099d3e83',

  'src/third_party/dart/third_party/pkg/dart_style':
   Var('dart_git') + '/dart_style.git@a3f1b041f3c5045d1fee7f1129075d0ad6a4a96e',

  'src/third_party/dart/third_party/pkg/dartdoc':
   Var('dart_git') + '/dartdoc.git@98fa859c8e7e9c0ee9a8873a09aed4d96aab4d3b',

  'src/third_party/dart/third_party/pkg/ffi':
   Var('dart_git') + '/ffi.git@32f5eefea59d4461e8ab40e83f157e49d5abe0da',

  'src/third_party/dart/third_party/pkg/file':
   Var('dart_git') + '/external/github.com/google/file.dart@72a67c33f90bfb7e10057e286e06d2fe3baa4d98',

  'src/third_party/dart/third_party/pkg/fixnum':
   Var('dart_git') + '/fixnum.git@f8379d94436f3a73f71ea078f3207897c6700498',

  'src/third_party/dart/third_party/pkg/glob':
   Var('dart_git') + '/glob.git@f378dc8734f9f2413e93951f95d278420e4a03e0',

  'src/third_party/dart/third_party/pkg/html':
   Var('dart_git') + '/html.git@08643e9e2baac0f20ce4a3b8baed820107972270',

  'src/third_party/dart/third_party/pkg/http':
   Var('dart_git') + '/http.git@805a14742e293fb51c0b89004b51add50d825792',

  'src/third_party/dart/third_party/pkg/http_multi_server':
   Var('dart_git') + '/http_multi_server.git@7bd190c70288361ed01ad31668e93544bcc5cf2b',

  'src/third_party/dart/third_party/pkg/http_parser':
   Var('dart_git') + '/http_parser.git@b3b283b9f139640b932d604e8815460bbd3ecab5',

  'src/third_party/dart/third_party/pkg/intl':
   Var('dart_git') + '/intl.git@fca552f2ec5d682b5fa36f02bdd72a5a4e2edcee',

  'src/third_party/dart/third_party/pkg/json_rpc_2':
   Var('dart_git') + '/json_rpc_2.git@0280ac6cb4f3905d81c47ba927123ba2b95f7940',

  'src/third_party/dart/third_party/pkg/linter':
   Var('dart_git') + '/linter.git@dcf3a079c573487b6ab64518604e5686be899601',

  'src/third_party/dart/third_party/pkg/logging':
   Var('dart_git') + '/logging.git@abef3717d958158eb8b0ddb2871f4b15a9804cd4',

  'src/third_party/dart/third_party/pkg/markdown':
   Var('dart_git') + '/markdown.git@ecbffa9bf9109d490b9388e9cb1f2bb801aee63c',

  'src/third_party/dart/third_party/pkg/matcher':
   Var('dart_git') + '/matcher.git@c1a07043de3da4968e1736fcde1c521802821859',

  'src/third_party/dart/third_party/pkg/mime':
   Var('dart_git') + '/mime.git@1a51be02b3cbc742c79d83575bd096ae8c295c1f',

  'src/third_party/dart/third_party/pkg/mockito':
   Var('dart_git') + '/mockito.git@ed5bd84f7bd7b651c96fbd7f449e92984a902502',

  'src/third_party/dart/third_party/pkg/package_config':
   Var('dart_git') + '/package_config.git@74ac1cb90ad7f4f05bd92c952eaf9756c516fca3',

  'src/third_party/dart/third_party/pkg/path':
   Var('dart_git') + '/path.git@24b58a2499440cc3d12d2b3c2f52b95f91ff1c0a',

  'src/third_party/dart/third_party/pkg/pool':
   Var('dart_git') + '/pool.git@694cfd867d8d84857fc42a8d07933013f0d25305',

  'src/third_party/dart/third_party/pkg/protobuf':
   Var('dart_git') + '/protobuf.git' + '@' + Var('dart_protobuf_rev'),

  'src/third_party/dart/third_party/pkg/pub':
   Var('dart_git') + '/pub.git' + '@' + Var('dart_pub_rev'),

  'src/third_party/dart/third_party/pkg/pub_semver':
   Var('dart_git') + '/pub_semver.git@c0e6ea74ccfbfdb2ef54c7cd9ad31455ca8e481f',

  'src/third_party/dart/third_party/pkg/shelf':
   Var('dart_git') + '/shelf.git@e3cfe796ad478cb4b61ad8e68ea29644c0bb1037',

  'src/third_party/dart/third_party/pkg/source_map_stack_trace':
   Var('dart_git') + '/source_map_stack_trace.git@45ea3688b6bec1eb565c38c4e28f37b01f8a0fa9',

  'src/third_party/dart/third_party/pkg/source_maps':
   Var('dart_git') + '/source_maps.git@a112e981a985a8af2047ee25a9cf56b0d5413b02',

  'src/third_party/dart/third_party/pkg/source_span':
   Var('dart_git') + '/source_span.git@3951ba50ec29b9870c3131c6ddcc88700d26f3ee',

  'src/third_party/dart/third_party/pkg/sse':
   Var('dart_git') + '/sse.git@8c03b73f90d951f5b33dd496593718107c79f97a',

  'src/third_party/dart/third_party/pkg/stack_trace':
   Var('dart_git') + '/stack_trace.git@6ceb191ace71c18ccf5648f6b2e8be52da39c56f',

  'src/third_party/dart/third_party/pkg/stream_channel':
   Var('dart_git') + '/stream_channel.git@a20ccd4142947637213393803289787a233402a1',

  'src/third_party/dart/third_party/pkg/string_scanner':
   Var('dart_git') + '/string_scanner.git@29e471ea98762db8052cca0e6a69ef9c7999abed',

  'src/third_party/dart/third_party/pkg/term_glyph':
   Var('dart_git') + '/term_glyph.git@d275a8f7482b6a5e4f15d0da6feb66c24f52eb94',

  'src/third_party/dart/third_party/pkg/test':
   Var('dart_git') + '/test.git@3ba78f15538399073d0b5aba4ec19b1378af1625',

  'src/third_party/dart/third_party/pkg/test_reflective_loader':
   Var('dart_git') + '/test_reflective_loader.git@c4c2d5c3f94a96f3fc79e9e28944fba391bc544c',

  'src/third_party/dart/third_party/pkg/tools':
   Var('dart_git') + '/tools.git@bed358ea8ca10551c710282be96cf6e95620fb24',

  'src/third_party/dart/third_party/pkg/typed_data':
   Var('dart_git') + '/typed_data.git@f858046fb420cf644e7d8cb86b7893f2830d8a6c',

  'src/third_party/dart/third_party/pkg/usage':
   Var('dart_git') + '/usage.git@399770f45ed2d384d79395c0341a04fb25609a4d',

  'src/third_party/dart/third_party/pkg/watcher':
   Var('dart_git') + '/watcher.git' + '@' + Var('dart_watcher_rev'),

  'src/third_party/dart/third_party/pkg/web_socket_channel':
   Var('dart_git') + '/web_socket_channel.git@e2fe7f665997ab8e28aa76a8b26483f9903221a8',

  'src/third_party/dart/third_party/pkg/webdev':
   Var('dart_git') + '/webdev.git' + '@' + Var('dart_webdev_rev'),

  'src/third_party/dart/third_party/pkg/webkit_inspection_protocol':
   Var('dart_git') + '/external/github.com/google/webkit_inspection_protocol.dart.git' + '@' + Var('dart_webkit_inspection_protocol_rev'),

  'src/third_party/dart/third_party/pkg/yaml':
   Var('dart_git') + '/yaml.git@0f80b12978f066b3547632dec4a273649138fae4',

  'src/third_party/dart/third_party/pkg/yaml_edit':
   Var('dart_git') + '/yaml_edit.git' + '@' + Var('dart_yaml_edit_rev'),

  'src/third_party/dart/tools/sdks':
   {'packages': [{'version': 'version:3.0.0-187.0.dev', 'package': 'dart/dart-sdk/${{platform}}'}], 'dep_type': 'cipd'},

  # WARNING: end of dart dependencies list that is cleaned up automatically - see create_updated_flutter_deps.py.

  # Prebuilt Dart SDK of the same revision as the Dart SDK source checkout
  'src/flutter/prebuilts/linux-x64/dart-sdk': {
    'packages': [
      {
        'package': 'flutter/dart-sdk/linux-amd64',
        'version': 'git_revision:'+Var('dart_revision')
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "linux" and download_dart_sdk'
  },
  'src/flutter/prebuilts/linux-arm64/dart-sdk': {
    'packages': [
      {
        'package': 'flutter/dart-sdk/linux-arm64',
        'version': 'git_revision:'+Var('dart_revision')
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "linux" and download_dart_sdk'
  },
  'src/flutter/prebuilts/macos-x64/dart-sdk': {
    'packages': [
      {
        'package': 'flutter/dart-sdk/mac-amd64',
        'version': 'git_revision:'+Var('dart_revision')
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "mac" and download_dart_sdk'
  },
  'src/flutter/prebuilts/macos-arm64/dart-sdk': {
    'packages': [
      {
        'package': 'flutter/dart-sdk/mac-arm64',
        'version': 'git_revision:'+Var('dart_revision')
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "mac" and download_dart_sdk'
  },
  'src/flutter/prebuilts/windows-x64/dart-sdk': {
    'packages': [
      {
        'package': 'flutter/dart-sdk/windows-amd64',
        'version': 'git_revision:'+Var('dart_revision')
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "win" and download_dart_sdk'
  },
  'src/flutter/prebuilts/windows-arm64/dart-sdk': {
    'packages': [
      {
        'package': 'flutter/dart-sdk/windows-arm64',
        'version': 'git_revision:'+Var('dart_revision')
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "win" and download_dart_sdk and not release_candidate'
  },

  'src/third_party/colorama/src':
   Var('chromium_git') + '/external/colorama.git' + '@' + '799604a1041e9b3bc5d2789ecbd7e8db2e18e6b8',

  'src/third_party/expat':
   Var('chromium_git') + '/external/github.com/libexpat/libexpat.git' + '@' + '654d2de0da85662fcc7644a7acd7c2dd2cfb21f0',

  'src/third_party/freetype2':
   Var('flutter_git') + '/third_party/freetype2' + '@' + '3bea2761290a1cbe7d8f75c1c5a7ad727f826a66',

  'src/third_party/root_certificates':
   Var('dart_git') + '/root_certificates.git' + '@' + Var('dart_root_certificates_rev'),

  'src/third_party/skia':
   Var('skia_git') + '/skia.git' + '@' +  Var('skia_revision'),

  'src/third_party/ocmock':
   Var('ocmock_git') + '@' +  Var('ocmock_rev'),

  'src/third_party/libjpeg-turbo':
   Var('fuchsia_git') + '/third_party/libjpeg-turbo' + '@' + '0fb821f3b2e570b2783a94ccd9a2fb1f4916ae9f',

  'src/third_party/libpng':
   Var('flutter_git') + '/third_party/libpng' + '@' + '9187b6e12756317f6d44fc669ac11dfc262bd192',

  'src/third_party/libwebp':
   Var('chromium_git') + '/webm/libwebp.git' + '@' + '7dfde712a477e420968732161539011e0fd446cf', # 1.2.0

  'src/third_party/wuffs':
   Var('skia_git') + '/external/github.com/google/wuffs-mirror-release-c.git' + '@' + '600cd96cf47788ee3a74b40a6028b035c9fd6a61',

  'src/third_party/fontconfig/src':
   Var('chromium_git') + '/external/fontconfig.git' + '@' + 'c336b8471877371f0190ba06f7547c54e2b890ba',

  'src/third_party/fontconfig':
   Var('flutter_git') + '/third_party/fontconfig' + '@' + '81c83d510ae3aa75589435ce32a5de05139aacb0',

  'src/third_party/libxml':
   Var('flutter_git') + '/third_party/libxml' + '@' + 'a143e452b5fc7d872813eeadc8db421694058098',

  'src/third_party/zlib':
   Var('chromium_git') + '/chromium/src/third_party/zlib.git' + '@' + Var('dart_zlib_rev'),

  'src/third_party/inja':
   Var('flutter_git') + '/third_party/inja' + '@' + '88bd6112575a80d004e551c98cf956f88ff4d445',

  'src/third_party/libtess2':
   Var('flutter_git') + '/third_party/libtess2' + '@' + '725e5e08ec8751477565f1d603fd7eb9058c277c',

  'src/third_party/sqlite':
   Var('flutter_git') + '/third_party/sqlite' + '@' + '0f61bd2023ba94423b4e4c8cfb1a23de1fe6a21c',

  'src/third_party/pyyaml':
   Var('fuchsia_git') + '/third_party/pyyaml.git' + '@' + '25e97546488eee166b1abb229a27856cecd8b7ac',

   # Upstream Khronos Vulkan Headers are part of vulkan-deps
   # Downstream Fuchsia Vulkan Headers (v1.2.198)
  'src/third_party/fuchsia-vulkan':
   Var('fuchsia_git') + '/third_party/Vulkan-Headers.git' + '@' + '32640ad82ef648768c706c9bf828b77123a09bc2',

   'src/third_party/swiftshader':
   Var('swiftshader_git') + '/SwiftShader.git' + '@' + '5f9ed9b16931c7155171d31f75004f73f0a3abc8',

   'src/third_party/angle':
   Var('chromium_git') + '/angle/angle.git' + '@' + '32175d8a6dc185dd850e5c9988f208085c9c0d80',

   'src/third_party/vulkan_memory_allocator':
   Var('chromium_git') + '/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator' + '@' + '7de5cc00de50e71a3aab22dea52fbb7ff4efceb6',

   'src/third_party/abseil-cpp':
   Var('flutter_git') + '/third_party/abseil-cpp.git' + '@' + '61833f2c057a2b1993d871e8c51156aed1dd4354',

   # Dart packages
  'src/third_party/pkg/archive':
  Var('github_git') + '/brendan-duncan/archive.git' + '@' + '9de7a0544457c6aba755ccb65abb41b0dc1db70d', # 3.1.2

  'src/third_party/pkg/equatable':
  Var('github_git') + '/felangel/equatable.git' + '@' + '0ba67c72db8bed75877fc1caafa74112ee0bd921', # 2.0.2

  'src/third_party/pkg/file':
  Var('dart_git') + '/external/github.com/google/file.dart.git' + '@' + 'b2e31cb6ef40b223701dbfa0b907fe58468484d7', # 6.1.4

  'src/third_party/pkg/flutter_packages':
  Var('github_git') + '/flutter/packages.git' + '@' + '25454e63851fe7933f04d025606e68c1eac4fe0f', # various

  'src/third_party/pkg/gcloud':
  Var('github_git') + '/dart-lang/gcloud.git' + '@' + 'a5276b85c4714378e84b1fb478b8feeeb686ac26', # 0.8.6-dev

  'src/third_party/pkg/googleapis':
  Var('github_git') + '/google/googleapis.dart.git' + '@' + '526011f56d98eab183cc6075ee1392e8303e43e2', # various

  'src/third_party/pkg/platform':
  Var('github_git') + '/google/platform.dart.git' + '@' + '1ffad63428bbd1b3ecaa15926bacfb724023648c', # 3.1.0

  'src/third_party/pkg/process':
  Var('github_git') + '/google/process.dart.git' + '@' + '0c9aeac86dcc4e3a6cf760b76fed507107e244d5', # 4.2.1

  'src/third_party/pkg/process_runner':
  Var('github_git') + '/google/process_runner.git' + '@' + 'd632ea0bfd814d779fcc53a361ed33eaf3620a0b', # 4.0.1

  'src/third_party/pkg/quiver':
  Var('github_git') + '/google/quiver-dart.git' + '@' + '90b92bee895e507d435012356a8b5c5f17eafa52', # 3.2.1

  'src/third_party/pkg/vector_math':
  Var('dart_git') + '/external/github.com/google/vector_math.dart.git' + '@' + '0a5fd95449083d404df9768bc1b321b88a7d2eef', # 2.1.0

  'src/third_party/imgui':
  Var('github_git') + '/ocornut/imgui.git' + '@' + '3ea0fad204e994d669f79ed29dcaf61cd5cb571d',

  'src/third_party/tinygltf':
  Var('github_git') + '/syoyo/tinygltf.git' + '@' + '9bb5806df4055ac973b970ba5b3e27ce27d98148',

  'src/third_party/json':
  Var('github_git') + '/nlohmann/json.git' + '@' + '17d9eacd248f58b73f4d1be518ef649fe2295642',

  'src/third_party/gradle': {
    'packages': [
      {
        'version': 'version:7.0.2',
        'package': 'flutter/gradle'
      }
    ],
    'condition': 'download_android_deps',
    'dep_type': 'cipd'
  },

  'src/third_party/android_tools/trace_to_text': {
    'packages': [
      {
        # 25.0 downloads for both mac-amd64 and mac-arm64
        # 26.1 is not found with either platform
        # 27.1 is the latest release of perfetto
        'version': 'git_tag:v25.0',
        'package': 'perfetto/trace_to_text/${{platform}}'
      }
    ],
    'condition': 'download_android_deps',
    'dep_type': 'cipd'
  },

   'src/third_party/android_tools/google-java-format': {
     'packages': [
       {
        'package': 'flutter/android/google-java-format',
        'version': 'version:1.7-1'
       }
     ],
     # We want to be able to format these as part of CI, and the CI step that
     # checks formatting runs without downloading the rest of the Android build
     # tooling. Therefore unlike all the other Android-related tools, we want to
     # download this every time.
     'dep_type': 'cipd',
   },

  'src/third_party/android_tools': {
     'packages': [
       {
        'package': 'flutter/android/sdk/all/${{platform}}',
        'version': 'version:33v6'
       }
     ],
     'condition': 'download_android_deps',
     'dep_type': 'cipd',
   },

  'src/third_party/android_embedding_dependencies': {
     'packages': [
       {
        'package': 'flutter/android/embedding_bundle',
        'version': 'last_updated:2021-11-23T12:31:07-0800'
       }
     ],
     'condition': 'download_android_deps',
     'dep_type': 'cipd',
   },

  'src/third_party/web_dependencies': {
     'packages': [
       {
         'package': 'flutter/web/canvaskit_bundle',
         'version': Var('canvaskit_cipd_instance')
       }
     ],
     'dep_type': 'cipd',
   },

  'src/third_party/java/openjdk': {
     'packages': [
       {
        'package': 'flutter/java/openjdk/${{platform}}',
        'version': 'version:11'
       }
     ],
     'condition': 'download_android_deps',
     'dep_type': 'cipd',
   },

  'src/flutter/third_party/gn': {
    'packages': [
      {
        'package': 'gn/gn/${{platform}}',
        'version': 'git_revision:b79031308cc878488202beb99883ec1f2efd9a6d'
      },
    ],
    'dep_type': 'cipd',
  },
  'src/flutter/third_party/ninja': {
    'packages': [
      {
        'package': 'infra/3pp/tools/ninja/${{platform}}',
        'version': 'version:2@1.11.1.chromium.4',
      }
    ],
    'dep_type': 'cipd',
  },

  'src/buildtools/emsdk': {
   'url': Var('skia_git') + '/external/github.com/emscripten-core/emsdk.git' + '@' + 'fc645b7626ebf86530dbd82fbece74d457e7ae07',
   'condition': 'download_emsdk',
  },

  # Clang on mac and linux are expected to typically be the same revision.
  # They are separated out so that the autoroller can more easily manage them.
  'src/buildtools/mac-x64/clang': {
    'packages': [
      {
        'package': 'fuchsia/third_party/clang/mac-amd64',
        'version': Var('clang_version'),
      }
    ],
    'condition': 'host_os == "mac"', # On ARM64 Macs too because Goma doesn't support the host-arm64 toolchain.
    'dep_type': 'cipd',
  },

  'src/buildtools/mac-arm64/clang': {
    'packages': [
      {
        'package': 'fuchsia/third_party/clang/mac-arm64',
        'version': Var('clang_version'),
      }
    ],
    'condition': 'host_os == "mac" and host_cpu == "arm64"',
    'dep_type': 'cipd',
  },

  'src/buildtools/linux-x64/clang': {
    'packages': [
      {
        'package': 'fuchsia/third_party/clang/linux-amd64',
        'version': Var('clang_version'),
      }
    ],
    'condition': 'host_os == "linux" and host_cpu == "x64"',
    'dep_type': 'cipd',
  },

  'src/buildtools/linux-arm64/clang': {
    'packages': [
      {
        'package': 'fuchsia/third_party/clang/linux-arm64',
        'version': Var('clang_version'),
      }
    ],
    'condition': 'host_os == "linux" and host_cpu == "arm64"',
    'dep_type': 'cipd',
  },

  'src/buildtools/windows-x64/clang': {
    'packages': [
      {
        'package': 'fuchsia/third_party/clang/windows-amd64',
        'version': Var('clang_version'),
      }
    ],
    'condition': 'download_windows_deps',
    'dep_type': 'cipd',
  },

   # Get the SDK from https://chrome-infra-packages.appspot.com/p/fuchsia/sdk/core at the 'latest' tag
   # Get the toolchain from https://chrome-infra-packages.appspot.com/p/fuchsia/clang at the 'goma' tag

   'src/fuchsia/sdk/mac': {
     'packages': [
       {
        'package': 'fuchsia/sdk/core/mac-amd64',
        'version': 'lfirU5p2GigxFWZVA4s5fWx9-lhElkibhnNu2A3LOAgC'
       }
     ],
     'condition': 'host_os == "mac" and not download_fuchsia_sdk',
     'dep_type': 'cipd',
   },
   'src/fuchsia/sdk/linux': {
     'packages': [
       {
        'package': 'fuchsia/sdk/core/linux-amd64',
        'version': 'YaWqKKuj-fAqfpKCmyScvIfdALnv_IXsw486iCGUgN8C'
       }
     ],
     'condition': 'host_os == "linux" and not download_fuchsia_sdk',
     'dep_type': 'cipd',
   },
}

recursedeps = [
  'src/third_party/vulkan-deps',
]

hooks = [
  {
    # Generate the Dart SDK's .dart_tool/package_confg.json file.
    'name': 'Generate .dart_tool/package_confg.json',
    'pattern': '.',
    'action': ['python3', 'src/third_party/dart/tools/generate_package_config.py'],
  },
  {
    # Generate the sdk/version file.
    'name': 'Generate sdk/version',
    'pattern': '.',
    'action': ['python3', 'src/third_party/dart/tools/generate_sdk_version_file.py'],
  },
  {
    # Update the Windows toolchain if necessary.
    'name': 'win_toolchain',
    'condition': 'download_windows_deps',
    'pattern': '.',
    'action': ['python3', 'src/build/vs_toolchain.py', 'update'],
  },
  {
    # Ensure that we don't accidentally reference any .pyc files whose
    # corresponding .py files have already been deleted.
    'name': 'remove_stale_pyc_files',
    'pattern': 'src/tools/.*\\.py',
    'action': [
        'python3',
        'src/tools/remove_stale_pyc_files.py',
        'src/tools',
    ],
  },
  {
    'name': 'dia_dll',
    'pattern': '.',
    'condition': 'download_windows_deps',
    'action': [
      'python3',
      'src/flutter/tools/dia_dll.py',
    ],
  },
  {
    'name': 'linux_sysroot_x64',
    'pattern': '.',
    'condition': 'download_linux_deps',
    'action': [
      'python3',
      'src/build/linux/sysroot_scripts/install-sysroot.py',
      '--arch=x64'],
  },
  {
    'name': 'linux_sysroot_arm64',
    'pattern': '.',
    'condition': 'download_linux_deps',
    'action': [
      'python3',
      'src/build/linux/sysroot_scripts/install-sysroot.py',
      '--arch=arm64'],
  },
  {
    'name': 'pub get --offline',
    'pattern': '.',
    'action': [
      'python3',
      'src/flutter/tools/pub_get_offline.py',
    ]
  },
  {
    'name': 'Download Fuchsia SDK',
    'pattern': '.',
    'condition': 'download_fuchsia_sdk',
    'action': [
      'python3',
      'src/flutter/tools/download_fuchsia_sdk.py',
      '--fail-loudly',
      '--verbose',
      '--host-os',
      Var('host_os'),
      '--fuchsia-sdk-path',
      Var('fuchsia_sdk_path'),
    ]
  },
  {
    'name': 'Activate Emscripten SDK',
    'pattern': '.',
    'condition': 'download_emsdk',
    'action': [
      'python3',
      'src/flutter/tools/activate_emsdk.py',
    ]
  },
  {
    'name': 'Setup githooks',
    'pattern': '.',
    'condition': 'setup_githooks',
    'action': [
      'python3',
      'src/flutter/tools/githooks/setup.py',
    ]
  }
]
