// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/threading/worker_pool.h"
#include "sky/engine/core/loader/CanvasImageDecoder.h"
#include "sky/engine/core/painting/CanvasImage.h"
#include "sky/engine/platform/SharedBuffer.h"
#include "sky/engine/platform/image-decoders/ImageDecoder.h"

namespace blink {

class ImageDecoderJob : public base::RefCountedThreadSafe<ImageDecoderJob> {
 public:
  explicit ImageDecoderJob(PassRefPtr<SharedBuffer> buffer);
  ~ImageDecoderJob();
  void DecodeImage();
  PassRefPtr<SkImage> decoded_image() { return decoded_image_; }
 private:
  RefPtr<SharedBuffer> buffer_;
  RefPtr<SkImage> decoded_image_;
};

PassRefPtr<CanvasImageDecoder> CanvasImageDecoder::create(
    PassOwnPtr<ImageDecoderCallback> callback) {
  return adoptRef(new CanvasImageDecoder(callback));
}

CanvasImageDecoder::CanvasImageDecoder(PassOwnPtr<ImageDecoderCallback> callback)
    : callback_(callback), weak_factory_(this) {
  CHECK(callback_);
  buffer_ = SharedBuffer::create();
}

CanvasImageDecoder::~CanvasImageDecoder() {
}

void CanvasImageDecoder::initWithConsumer(mojo::ScopedDataPipeConsumerHandle handle) {
  CHECK(!drainer_);
  if (!handle.is_valid()) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE, base::Bind(&CanvasImageDecoder::RejectCallback,
                              weak_factory_.GetWeakPtr()));
    return;
  }

  drainer_ = adoptPtr(new mojo::common::DataPipeDrainer(this, handle.Pass()));
}

void CanvasImageDecoder::initWithList(const Uint8List& list) {
  CHECK(!drainer_);

  OnDataAvailable(list.data(), list.num_elements());
  OnDataComplete();
}

void CanvasImageDecoder::OnDataAvailable(const void* data, size_t num_bytes) {
  buffer_->append(static_cast<const char*>(data), num_bytes);
}

void CanvasImageDecoder::OnDataComplete() {
  job_ = new ImageDecoderJob(buffer_.release());
  base::WorkerPool::PostTaskAndReply(FROM_HERE,
                                     base::Bind(&ImageDecoderJob::DecodeImage, job_.get()),
                                     base::Bind(&CanvasImageDecoder::OnDecodeComplete, weak_factory_.GetWeakPtr()),
                                     /* task_is_slow */ true);
}

ImageDecoderJob::ImageDecoderJob(PassRefPtr<SharedBuffer> buffer)
    : buffer_(buffer) {
  CHECK(buffer_->hasOneRef());
}

ImageDecoderJob::~ImageDecoderJob() {
}

void ImageDecoderJob::DecodeImage() {
  OwnPtr<ImageDecoder> decoder =
      ImageDecoder::create(*buffer_.get(), ImageSource::AlphaPremultiplied,
                           ImageSource::GammaAndColorProfileIgnored);
  // decoder can be null if the buffer we was empty and we couldn't even guess
  // what type of image to decode.
  if (!decoder) {
    return;
  }
  decoder->setData(buffer_.get(), true);
  if (decoder->failed() || decoder->frameCount() == 0) {
    return;
  }

  ImageFrame* imageFrame = decoder->frameBufferAtIndex(0);
  decoded_image_ = adoptRef(SkImage::NewFromBitmap(imageFrame->getSkBitmap()));
}

void CanvasImageDecoder::OnDecodeComplete() {
  RefPtr<CanvasImage> resultImage = CanvasImage::create();
  resultImage->setImage(job_->decoded_image());
  callback_->handleEvent(resultImage.get());
}

void CanvasImageDecoder::RejectCallback() {
  callback_->handleEvent(nullptr);
}

}  // namespace blink
