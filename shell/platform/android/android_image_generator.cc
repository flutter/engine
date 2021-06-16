
#include "flutter/shell/platform/android/android_image_generator.h"

#include <android/bitmap.h>
#include <android/hardware_buffer.h>

#include "flutter/fml/platform/android/jni_util.h"

namespace flutter {

static fml::jni::ScopedJavaGlobalRef<jclass>* g_flutter_jni_class = nullptr;
static jmethodID g_decode_image_method = nullptr;

AndroidImageGenerator::~AndroidImageGenerator() = default;

AndroidImageGenerator::AndroidImageGenerator(sk_sp<SkData> data)
    : data_(data), image_info_(SkImageInfo::MakeUnknown()) {}

const SkImageInfo& AndroidImageGenerator::GetInfo() {
  header_decoded_latch_.Wait();
  return image_info_;
}

unsigned int AndroidImageGenerator::GetFrameCount() const {
  return 1;
}

unsigned int AndroidImageGenerator::GetPlayCount() const {
  return 1;
}

const ImageGenerator::FrameInfo AndroidImageGenerator::GetFrameInfo(
    unsigned int frame_index) const {
  return {.required_frame = std::nullopt,
          .duration = 0,
          .disposal_method = SkCodecAnimation::DisposalMethod::kKeep};
}

SkISize AndroidImageGenerator::GetScaledDimensions(float desired_scale) {
  return GetInfo().dimensions();
}

bool AndroidImageGenerator::GetPixels(const SkImageInfo& info,
                                      void* pixels,
                                      size_t row_bytes,
                                      unsigned int frame_index,
                                      std::optional<unsigned int> prior_frame) {
  fully_decoded_latch_.Wait();

  if (kRGBA_8888_SkColorType != info.colorType()) {
    return false;
  }

  switch (info.alphaType()) {
    case kOpaque_SkAlphaType:
      if (kOpaque_SkAlphaType != GetInfo().alphaType()) {
        return false;
      }
      break;
    case kPremul_SkAlphaType:
      break;
    default:
      return false;
  }

  // Since this decoder is only installed for API 28 and 29, there is no
  // supported way to work around this unfortunate copy. It's possible to
  // produce an `SkImage` direcly from an `AHardwareBuffer`, but
  // `AndroidBitmap_getHardwareBuffer` and `Bitmap.getHardwareBuffer` are only
  // available in API 30+ and 31+ respectively.
  memcpy(pixels, software_decoded_data_->data(), software_decoded_data_->size());
  return true;
}

void AndroidImageGenerator::DecodeImage() {
  DoDecodeImage();

  header_decoded_latch_.Signal();
  fully_decoded_latch_.Signal();
}

void AndroidImageGenerator::DoDecodeImage() {
  if (!g_flutter_jni_class || !g_decode_image_method) {
    return;
  }

  // Call FlutterJNI.decodeImage

  JNIEnv* env = fml::jni::AttachCurrentThread();

  fml::jni::ScopedJavaLocalRef<jobject> direct_buffer(
      env, env->NewDirectByteBuffer(const_cast<void*>(data_->data()),
                                    data_->size()));
  fml::jni::ScopedJavaGlobalRef<jobject>* bitmap =
      new fml::jni::ScopedJavaGlobalRef(
          env, env->CallStaticObjectMethod(g_flutter_jni_class->obj(),
                                           g_decode_image_method,
                                           direct_buffer.obj(), this));
  FML_CHECK(fml::jni::CheckException(env));

  AndroidBitmapInfo info;
  int status;
  if ((status = AndroidBitmap_getInfo(env, bitmap->obj(), &info)) < 0) {
    FML_DLOG(ERROR) << "Failed to get bitmap info, status=" << status;
    return;
  }
  FML_DCHECK(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888);

  // Lock the android buffer in a shared pointer

  void* pixel_lock;
  if ((status = AndroidBitmap_lockPixels(env, bitmap->obj(), &pixel_lock)) <
      0) {
    FML_DLOG(ERROR) << "Failed to lock pixels, error=" << status;
    return;
  }

  SkData::ReleaseProc on_release = [](const void* ptr, void* context) -> void {
    fml::jni::ScopedJavaGlobalRef<jobject>* bitmap =
        reinterpret_cast<fml::jni::ScopedJavaGlobalRef<jobject>*>(context);
    auto env = fml::jni::AttachCurrentThread();
    AndroidBitmap_unlockPixels(env, bitmap->obj());
  };

  software_decoded_data_ = SkData::MakeWithProc(
      pixel_lock, info.width * info.height * sizeof(uint32_t), on_release,
      bitmap);
}

bool AndroidImageGenerator::Register(JNIEnv* env) {
  g_flutter_jni_class = new fml::jni::ScopedJavaGlobalRef<jclass>(
      env, env->FindClass("io/flutter/embedding/engine/FlutterJNI"));
  if (g_flutter_jni_class->is_null()) {
    FML_LOG(ERROR) << "Failed to find FlutterJNI class";
    return false;
  }

  g_decode_image_method = env->GetStaticMethodID(
      g_flutter_jni_class->obj(), "decodeImage",
      "(Ljava/nio/ByteBuffer;J)Landroid/graphics/Bitmap;");

  if (g_decode_image_method == nullptr) {
    FML_LOG(ERROR) << "Failed to find FlutterJNI.decodeImage method";
    return false;
  }

  static const JNINativeMethod header_decoded_method = {
      .name = "nativeImageHeaderCallback",
      .signature = "(JII)V",
      .fnPtr = reinterpret_cast<void*>(
          &AndroidImageGenerator::NativeImageHeaderCallback),
  };
  if (env->RegisterNatives(g_flutter_jni_class->obj(), &header_decoded_method,
                           1) != 0) {
    FML_LOG(ERROR)
        << "Failed to register FlutterJNI.nativeImageHeaderCallback method";
    return false;
  }

  return true;
}

std::unique_ptr<ImageGenerator> AndroidImageGenerator::MakeFromData(
    sk_sp<SkData> data,
    fml::RefPtr<fml::TaskRunner> task_runner) {
  auto* generator = new AndroidImageGenerator(std::move(data));

  fml::TaskRunner::RunNowOrPostTask(
      task_runner, [generator]() { generator->DecodeImage(); });

  if (generator->IsValidImageData()) {
    return std::unique_ptr<AndroidImageGenerator>(generator);
  }
  return nullptr;
}

void AndroidImageGenerator::NativeImageHeaderCallback(long generator_address,
                                                      int width,
                                                      int height) {
  auto* generator = reinterpret_cast<AndroidImageGenerator*>(generator_address);

  generator->image_info_ = SkImageInfo::Make(
      width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
  generator->header_decoded_latch_.Signal();
}

bool AndroidImageGenerator::IsValidImageData() {
  // The generator kicks off an IO task to decode everything, and calls to
  // "GetInfo()" block until the header has been decoded. The decoder explicitly
  // marks the height as -1 if the image is invalid or if the SDK image decoder
  // is unavailable.
  return GetInfo().height() != -1;
}

}  // namespace flutter
