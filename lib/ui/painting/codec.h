// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_CODEC_H_
#define FLUTTER_LIB_UI_PAINTING_CODEC_H_

#include "lib/tonic/dart_wrappable.h"
#include "lib/tonic/dart_library_natives.h"

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace blink {

// A handle to an SkCodec object.
//
// Doesn't mirror SkCodec's API but provides a simple sequential access API.
class Codec final : public fxl::RefCountedThreadSafe<Codec>,
                    public tonic::DartWrappable {
  DEFINE_WRAPPERTYPEINFO();
  FRIEND_MAKE_REF_COUNTED(Codec);

 public:
  ~Codec() override;
  static fxl::RefPtr<Codec> Create() {
    return fxl::MakeRefCounted<Codec>();
  }

  int framesCount() { return 0; }
  int repetitionCount() { return 0; }

  void dispose() {}

  static void RegisterNatives(tonic::DartLibraryNatives* natives);
 private:
  Codec();
};
}  // namespace blink

#endif  // FLUTTER_LIB_UI_PAINTING_CODEC_H_
