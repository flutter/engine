// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_GLUE_DATA_PIPE_FLUSHER_H_
#define FLUTTER_GLUE_DATA_PIPE_FLUSHER_H_

#include "lib/ftl/macros.h"
#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/environment/async_waiter.h"
#include "mojo/public/cpp/system/data_pipe.h"

namespace glue {

class DataPipeFlusher {
 public:
  using CompletionCallback =
      std::function<void(DataPipeFlusher* flusher, bool result)>;

  class Allocation {
   public:
    Allocation(uint8_t* data, uint32_t length, bool copy);

    Allocation(Allocation&& allocation);

    ~Allocation();

    bool IsReady() const;

   private:
    uint8_t* data_;
    uint32_t length_;
    uint32_t written_;
    bool ready_;

    friend class DataPipeFlusher;

    bool WriteCompleted() const;

    bool AdvanceWrite(uint32_t size);

    struct WriteHead {
      uint8_t* data;
      uint32_t length;
    };

    WriteHead NextWriteHead();

    FTL_DISALLOW_COPY_AND_ASSIGN(Allocation);
  };

  DataPipeFlusher(mojo::ScopedDataPipeProducerHandle producer,
                  Allocation allocation,
                  CompletionCallback callback);

  ~DataPipeFlusher();

  void Start();

 private:
  mojo::ScopedDataPipeProducerHandle producer_;
  Allocation allocation_;
  std::unique_ptr<mojo::AsyncWaiter> handle_waiter_;
  CompletionCallback callback_;
  base::WeakPtrFactory<DataPipeFlusher> weak_ptr_factory_;

  void WaitForWrite();
  void TryWrite(MojoResult wait_result);
  void FulfillCallback(bool result);

  FTL_DISALLOW_COPY_AND_ASSIGN(DataPipeFlusher);
};

}  // namespace glue

#endif  // FLUTTER_GLUE_DATA_PIPE_FLUSHER_H_
