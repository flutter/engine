// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_CODEC_H_
#define FLUTTER_LIB_UI_PAINTING_CODEC_H_

#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/painting/frame_info.h"
#include "third_party/skia/include/codec/SkCodec.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkImage.h"

using tonic::DartPersistentValue;

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace blink {

// A handle to an SkCodec object.
//
// Doesn't mirror SkCodec's API but provides a simple sequential access API.
class Codec : public RefCountedDartWrappable<Codec> {
  DEFINE_WRAPPERTYPEINFO();

 public:
  virtual int frameCount() = 0;
  virtual int repetitionCount() = 0;
  virtual Dart_Handle getNextFrame(Dart_Handle callback_handle) = 0;
  void dispose();
  virtual void enableFrameCache() {}
  virtual void clearAndDisableFrameCache() {}

  static void RegisterNatives(tonic::DartLibraryNatives* natives);
};

class MultiFrameCodec : public Codec {
 public:
  int frameCount() { return frameInfos_.size(); }
  int repetitionCount() { return repetitionCount_; }
  Dart_Handle getNextFrame(Dart_Handle args);
  // Called to enable caching decoded frames in memory. Caching defaults to on,
  // so only has an effect if [clearAndDisableFrameCache] was previously called.
  void enableFrameCache();
  // Should be called to evict previously decoded frames in cases of memory
  // pressure. Multi frame codecs use much more CPU to render without frame
  // caching enabled.
  void clearAndDisableFrameCache();

 private:
  MultiFrameCodec(std::unique_ptr<SkCodec> codec);

  ~MultiFrameCodec() {}

  sk_sp<SkImage> GetNextFrameImage(fml::WeakPtr<GrContext> resourceContext);

  void GetNextFrameAndInvokeCallback(
      std::unique_ptr<DartPersistentValue> callback,
      fml::RefPtr<fml::TaskRunner> ui_task_runner,
      fml::WeakPtr<GrContext> resourceContext,
      fml::RefPtr<flow::SkiaUnrefQueue> unref_queue,
      size_t trace_id);
  void populateFrameCache();
  void updateOrInsertCacheEntry(int frameIndex, bool required);

  const std::unique_ptr<SkCodec> codec_;
  int repetitionCount_;
  int nextFrameIndex_;

  std::vector<SkCodec::FrameInfo> frameInfos_;
  bool cacheAllFrames_ = true;

  // A struct linking the bitmap of a frame to whether it's required to render
  // other dependent frames.
  struct DecodedFrame {
    SkBitmap bitmap_;
    bool required_;

    DecodedFrame(SkBitmap bitmap, bool required)
        : bitmap_(bitmap), required_(required) {}
  };

  // A cache of previously loaded bitmaps, indexed by the frame they belong to.
  // Holds all frames if cacheAllFrames_ is true, or just the frames that are
  // marked as required for reuse by [SkCodec::getFrameInfo()] otherwise.
  std::map<int, std::unique_ptr<DecodedFrame>> frameBitmaps_;

  FML_FRIEND_MAKE_REF_COUNTED(MultiFrameCodec);
  FML_FRIEND_REF_COUNTED_THREAD_SAFE(MultiFrameCodec);
};

class SingleFrameCodec : public Codec {
 public:
  int frameCount() { return 1; }
  int repetitionCount() { return 0; }
  Dart_Handle getNextFrame(Dart_Handle args);

 private:
  SingleFrameCodec(fml::RefPtr<FrameInfo> frame) : frame_(std::move(frame)) {}
  ~SingleFrameCodec() {}

  fml::RefPtr<FrameInfo> frame_;

  FML_FRIEND_MAKE_REF_COUNTED(SingleFrameCodec);
  FML_FRIEND_REF_COUNTED_THREAD_SAFE(SingleFrameCodec);
};

}  // namespace blink

#endif  // FLUTTER_LIB_UI_PAINTING_CODEC_H_
