// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_H_
#define FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_H_

#include <sstream>

#include "flutter/lib/ui/dart_wrapper.h"
#include "third_party/tonic/typed_data/typed_list.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace flutter {

class DisplayList : public RefCountedDartWrappable<DisplayList> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(DisplayList);

 public:
  static fml::RefPtr<DisplayList> Create(tonic::Uint8List& ops, int numOps, tonic::DartByteData& data, int dataBytes);

  Dart_Handle toImage(uint32_t width,
                      uint32_t height,
                      Dart_Handle raw_image_callback);

  static Dart_Handle RasterizeToImage(std::shared_ptr<std::vector<uint8_t>> ops,
                                      std::shared_ptr<std::vector<float>> data,
                                      uint32_t width,
                                      uint32_t height,
                                      Dart_Handle raw_image_callback);

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

  std::shared_ptr<std::vector<uint8_t>> ops_vector() { return ops_vector_; }
  std::shared_ptr<std::vector<float>> data_vector() { return data_vector_; }

 private:
  explicit DisplayList(std::shared_ptr<std::vector<uint8_t>> ops_vector,
                       std::shared_ptr<std::vector<float>> data_vector);
  // explicit DisplayList();

  std::shared_ptr<std::vector<uint8_t>> ops_vector_;
  std::shared_ptr<std::vector<float>> data_vector_;
};

// class DisplayList : public RefCountedDartWrappable<DisplayList> {
//   DEFINE_WRAPPERTYPEINFO();
//   FML_FRIEND_MAKE_REF_COUNTED(DisplayList);

//  public:

//   ~DisplayList() override;
//   static fml::RefPtr<DisplayList> Create(Dart_Handle dart_handle,
//                                      flutter::SkiaGPUObject<SkPicture> picture);

//   Dart_Handle toImage(uint32_t width,
//                       uint32_t height,
//                       Dart_Handle raw_image_callback);

//   void dispose();

//   size_t GetAllocationSize() const override;

//   static void RegisterNatives(tonic::DartLibraryNatives* natives);

//   static Dart_Handle RasterizeToImage(sk_sp<SkPicture> picture,
//                                       uint32_t width,
//                                       uint32_t height,
//                                       Dart_Handle raw_image_callback);

//  private:
//   Picture(flutter::SkiaGPUObject<SkPicture> picture);
// };

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_H_
