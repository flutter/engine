// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_VERSIONS_H_
#define FLUTTER_LIB_UI_VERSIONS_H_

#include <string>
#include <vector>

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace blink {

class Versions final {
 public:
  Versions(const char* dart_version_,
           const char* skia_version_,
           const char* flutter_engine_version_);
  
  Versions(const Versions& other);

  ~Versions();

  // returns a vector with 3 versions.
  // dart, skia and flutter engine versions in this order.
  std::vector<std::string> GetVersionsList();
  
  static void RegisterNatives(tonic::DartLibraryNatives* natives);

private:
  std::string dart_version;
  std::string skia_version;
  std::string flutter_engine_version;
};

} // namespace blink

#endif  // FLUTTER_LIB_UI_VERSIONS_H_
