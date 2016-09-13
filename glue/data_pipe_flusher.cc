// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/glue/data_pipe_flusher.h"

#include "base/bind.h"
#include "lib/ftl/logging.h"

namespace glue {

DataPipeFlusher::Allocation::Allocation(uint8_t* data,
                                        uint32_t length,
                                        bool copy)
    : data_(nullptr), length_(length), written_(0), ready_(false) {
  if (data == nullptr || length == 0) {
    return;
  }

  if (copy) {
    data_ = reinterpret_cast<uint8_t*>(malloc(length));
    if (data_ == nullptr) {
      // Allocation failure. Bail without readiness.
      return;
    }
    memcpy(data_, data, length);
  } else {
    data_ = data;
  }

  ready_ = true;
}

DataPipeFlusher::Allocation::Allocation(Allocation&& other)
    : data_(other.data_),
      length_(other.length_),
      written_(other.written_),
      ready_(other.ready_) {
  other.data_ = nullptr;
  other.length_ = 0;
  other.written_ = 0;
  other.ready_ = false;
}

DataPipeFlusher::Allocation::~Allocation() {
  free(data_);
}

bool DataPipeFlusher::Allocation::IsReady() const {
  return ready_;
}

bool DataPipeFlusher::Allocation::WriteCompleted() const {
  return written_ == length_;
}

bool DataPipeFlusher::Allocation::AdvanceWrite(uint32_t size) {
  if (size + written_ > length_) {
    return false;
  }

  written_ += size;
  return true;
}

DataPipeFlusher::Allocation::WriteHead
DataPipeFlusher::Allocation::NextWriteHead() {
  return {
      data_ + written_,    // data
      length_ - written_,  // length
  };
}

DataPipeFlusher::DataPipeFlusher(mojo::ScopedDataPipeProducerHandle producer,
                                 Allocation allocation,
                                 CompletionCallback callback)
    : producer_(producer.Pass()),
      allocation_(std::move(allocation)),
      callback_(std::move(callback)),
      weak_ptr_factory_(this) {}

DataPipeFlusher::~DataPipeFlusher() = default;

void DataPipeFlusher::Start() {
  if (!allocation_.IsReady()) {
    FulfillCallback(false);
    return;
  }

  TryWrite(MOJO_RESULT_OK);
}

void DataPipeFlusher::TryWrite(MojoResult) {
  handle_waiter_.reset();

  auto write_head = allocation_.NextWriteHead();

  MojoResult write_result =
      MojoWriteData(producer_.get().value(), write_head.data,
                    &write_head.length,  // in-out
                    MOJO_WRITE_DATA_FLAG_NONE);

  switch (write_result) {
    case MOJO_RESULT_OK:
      allocation_.AdvanceWrite(write_head.length);
    case MOJO_RESULT_BUSY:
    case MOJO_RESULT_SHOULD_WAIT:
      if (allocation_.WriteCompleted()) {
        FulfillCallback(true);
      } else {
        WaitForWrite();
      }
      break;
    default:
      FulfillCallback(false);
      break;
  }
}

void DataPipeFlusher::WaitForWrite() {
  auto callback =
      base::Bind(&DataPipeFlusher::TryWrite, weak_ptr_factory_.GetWeakPtr());

  handle_waiter_.reset(
      new mojo::AsyncWaiter(producer_.get(),              // handle
                            MOJO_HANDLE_SIGNAL_WRITABLE,  // signals
                            callback)                     // callback
      );
}

void DataPipeFlusher::FulfillCallback(bool result) {
  if (callback_) {
    auto copied_callback = callback_;
    callback_ = nullptr;
    copied_callback(this, result);
  }
}

}  // namespace glue
