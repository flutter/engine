// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_DART_EMBEDDER_OBSERVATORY_ARCHIVE_H_
#define MOJO_DART_EMBEDDER_OBSERVATORY_ARCHIVE_H_

namespace dart {
namespace observatory {

// These two symbols are defined in |observatory_archive.cc| which is generated
// by the |//dart/runtime/observatory:archive_observatory| rule. Both of these
// symbols will be part of the data segment and therefore are read only.
extern unsigned int observatory_assets_archive_len;
extern const uint8_t* observatory_assets_archive;

}  // namespace observatory
}  // namespace dart

#endif  // MOJO_DART_EMBEDDER_OBSERVATORY_ARCHIVE_H_
