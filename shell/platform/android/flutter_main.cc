// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/flutter_main.h"

#include <vector>

#include "base/at_exit.h"
#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/threading/simple_thread.h"
#include "dart/runtime/include/dart_tools_api.h"
#include "flutter/fml/platform/android/jni_util.h"
#include "flutter/runtime/start_up.h"
#include "flutter/shell/common/shell.h"
#include "lib/ftl/arraysize.h"
#include "lib/ftl/command_line.h"
#include "lib/ftl/macros.h"

namespace shell {

static void Init(JNIEnv* env,
                 jclass clazz,
                 jobject context,
                 jobjectArray jargs) {
  // REMOVE
  base::android::ScopedJavaLocalRef<jobject> scoped_context_og(
      env, env->NewLocalRef(context));
  base::android::InitApplicationContext(env, scoped_context_og);
  // END REMOVE

  fml::jni::ScopedJavaLocalRef<jobject> scoped_context(
      env, env->NewLocalRef(context));
  fml::jni::InitAndroidApplicationContext(scoped_context);

  // Prepare command line arguments and initialize the shell.
  std::vector<std::string> args;
  args.push_back("sky_shell");
  base::android::AppendJavaStringArrayToStringVector(env, jargs, &args);
  Shell::InitStandalone(
      ftl::CommandLineFromIterators(args.begin(), args.end()));
}

static void RecordStartTimestamp(JNIEnv* env,
                                 jclass jcaller,
                                 jlong initTimeMillis) {
  int64_t initTimeMicros =
      static_cast<int64_t>(initTimeMillis) * static_cast<int64_t>(1000);
  blink::engine_main_enter_ts = Dart_TimelineGetMicros() - initTimeMicros;
}

bool RegisterFlutterMain(JNIEnv* env) {
  static const JNINativeMethod methods[] = {
      {
          .name = "nativeInit",
          .signature = "(Landroid/content/Context;[Ljava/lang/String;)V",
          .fnPtr = reinterpret_cast<void*>(&Init),
      },
      {
          .name = "nativeRecordStartTimestamp",
          .signature = "(J)V",
          .fnPtr = reinterpret_cast<void*>(&RecordStartTimestamp),
      },
  };

  jclass clazz = env->FindClass("io/flutter/view/FlutterMain");

  if (clazz == nullptr) {
    return false;
  }

  return env->RegisterNatives(clazz, methods, arraysize(methods)) == 0;
}

}  // namespace shell
