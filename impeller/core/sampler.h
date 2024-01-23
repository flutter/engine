// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_CORE_SAMPLER_H_
#define FLUTTER_IMPELLER_CORE_SAMPLER_H_

#include <unordered_map>

#include "impeller/base/comparable.h"
#include "impeller/core/sampler_descriptor.h"

namespace impeller {

class Sampler {
 public:
  virtual ~Sampler();

  virtual bool IsValid() const = 0;

  const SamplerDescriptor& GetDescriptor() const;

 protected:
  SamplerDescriptor desc_;

  explicit Sampler(SamplerDescriptor desc);

 private:
  Sampler(const Sampler&) = delete;

  Sampler& operator=(const Sampler&) = delete;
};

/// @brief A sampler subclass to be returned in the result of a lost device
//         or other failure to allocate a sampler object.
class InvalidSampler : public Sampler {
 public:
  explicit InvalidSampler(SamplerDescriptor desc) : Sampler(std::move(desc)) {}

  ~InvalidSampler() = default;

  bool IsValid() const override { return false; }
};

using SamplerMap = std::unordered_map<SamplerDescriptor,
                                      std::unique_ptr<const Sampler>,
                                      ComparableHash<SamplerDescriptor>,
                                      ComparableEqual<SamplerDescriptor>>;

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_CORE_SAMPLER_H_
