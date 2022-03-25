// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYER_SNAPSHOT_STORE_H_
#define FLUTTER_FLOW_LAYER_SNAPSHOT_STORE_H_

#include <vector>

#include "flutter/fml/logging.h"
#include "flutter/fml/time/time_delta.h"

#include "third_party/skia/include/core/SkImageEncoder.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkSerialProcs.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/core/SkSurfaceCharacterization.h"
#include "third_party/skia/include/utils/SkBase64.h"

namespace flutter {

class LayerSnapshotData {
 public:
  LayerSnapshotData(int64_t layer_unique_id,
                    fml::TimeDelta duration,
                    sk_sp<SkData> base64_png_snapshot);

  ~LayerSnapshotData() = default;

  int64_t GetLayerUniqueId() const { return layer_unique_id_; }

  fml::TimeDelta GetDuration() const { return duration_; }

  sk_sp<SkData> GetBase64PNGSnapshot() const { return base64_png_snapshot_; }

 private:
  const int64_t layer_unique_id_;
  const fml::TimeDelta duration_;
  const sk_sp<SkData> base64_png_snapshot_;
};

class LayerSnapshotStore {
 public:
  typedef std::vector<LayerSnapshotData> Snapshots;

  LayerSnapshotStore() = default;

  ~LayerSnapshotStore() = default;

  void Clear();

  void Add(int64_t layer_unique_id,
           fml::TimeDelta duration,
           sk_sp<SkData> base64_png_snapshot);

  // make this class iterable
  Snapshots::iterator begin() { return layer_snapshots_.begin(); }
  Snapshots::iterator end() { return layer_snapshots_.end(); }

 private:
  Snapshots layer_snapshots_;

  FML_DISALLOW_COPY_AND_ASSIGN(LayerSnapshotStore);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYER_SNAPSHOT_STORE_H_
