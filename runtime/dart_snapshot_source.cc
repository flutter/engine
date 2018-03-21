// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/dart_snapshot_source.h"

#include <utility>

#include "flutter/fml/mapping.h"

namespace blink {

class NativeLibrarySnapshotSource final : public DartSnapshotSource {
 public:
  NativeLibrarySnapshotSource(fxl::RefPtr<fml::NativeLibrary> library,
                              const char* symbol_name)
      : library_(std::move(library)) {
    if (library_) {
      symbol_ = library_->ResolveSymbol(symbol_name);
    }
  }

  const uint8_t* GetSnapshotPointer() const override { return symbol_; }

  size_t GetSnapshotSize() const override { return 0; }

 private:
  fxl::RefPtr<fml::NativeLibrary> library_;
  const uint8_t* symbol_ = nullptr;

  FXL_DISALLOW_COPY_AND_ASSIGN(NativeLibrarySnapshotSource);
};

class FileSnapshotSource final : public DartSnapshotSource {
 public:
  FileSnapshotSource(const char* path, bool executable)
      : mapping_(path, executable) {
    if (mapping_.GetSize() > 0) {
      symbol_ = mapping_.GetMapping();
    }
  }

  const uint8_t* GetSnapshotPointer() const override { return symbol_; }

  size_t GetSnapshotSize() const override { return mapping_.GetSize(); }

 private:
  fml::FileMapping mapping_;
  const uint8_t* symbol_ = nullptr;

  FXL_DISALLOW_COPY_AND_ASSIGN(FileSnapshotSource);
};

std::unique_ptr<DartSnapshotSource>
DartSnapshotSource::CreateWithSymbolInLibrary(
    fxl::RefPtr<fml::NativeLibrary> library,
    const char* symbol_name) {
  auto source = std::make_unique<NativeLibrarySnapshotSource>(
      std::move(library), symbol_name);
  return source->GetSnapshotPointer() == nullptr ? nullptr : std::move(source);
}

std::unique_ptr<DartSnapshotSource>
DartSnapshotSource::CreateWithContentsOfFile(const char* file_path,
                                             bool executable) {
  auto source = std::make_unique<FileSnapshotSource>(file_path, executable);
  return source->GetSnapshotPointer() == nullptr ? nullptr : std::move(source);
}

DartSnapshotSource::~DartSnapshotSource() = default;

}  // namespace blink
