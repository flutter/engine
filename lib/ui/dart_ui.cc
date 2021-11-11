// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/dart_ui.h"

#include "flutter/fml/build_config.h"
#include "flutter/lib/ui/compositing/scene.h"
#include "flutter/lib/ui/compositing/scene_builder.h"
#include "flutter/lib/ui/dart_runtime_hooks.h"
#include "flutter/lib/ui/isolate_name_server/isolate_name_server_natives.h"
#include "flutter/lib/ui/painting/canvas.h"
#include "flutter/lib/ui/painting/codec.h"
#include "flutter/lib/ui/painting/color_filter.h"
#include "flutter/lib/ui/painting/engine_layer.h"
#include "flutter/lib/ui/painting/fragment_program.h"
#include "flutter/lib/ui/painting/gradient.h"
#include "flutter/lib/ui/painting/image.h"
#include "flutter/lib/ui/painting/image_descriptor.h"
#include "flutter/lib/ui/painting/image_filter.h"
#include "flutter/lib/ui/painting/image_shader.h"
#include "flutter/lib/ui/painting/immutable_buffer.h"
#include "flutter/lib/ui/painting/path.h"
#include "flutter/lib/ui/painting/path_measure.h"
#include "flutter/lib/ui/painting/picture.h"
#include "flutter/lib/ui/painting/picture_recorder.h"
#include "flutter/lib/ui/painting/vertices.h"
#include "flutter/lib/ui/semantics/semantics_update.h"
#include "flutter/lib/ui/semantics/semantics_update_builder.h"
#include "flutter/lib/ui/semantics/string_attribute.h"
#include "flutter/lib/ui/text/font_collection.h"
#include "flutter/lib/ui/text/paragraph.h"
#include "flutter/lib/ui/text/paragraph_builder.h"
#include "flutter/lib/ui/window/platform_configuration.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/logging/dart_error.h"

using tonic::ToDart;

namespace flutter {
namespace {

static tonic::DartLibraryNatives* g_natives;

Dart_NativeFunction GetNativeFunction(Dart_Handle name,
                                      int argument_count,
                                      bool* auto_setup_scope) {
  return g_natives->GetNativeFunction(name, argument_count, auto_setup_scope);
}

const uint8_t* GetSymbol(Dart_NativeFunction native_function) {
  return g_natives->GetSymbol(native_function);
}

}  // namespace

void DartUI::InitForGlobal() {
  if (!g_natives) {
    g_natives = new tonic::DartLibraryNatives();
    Canvas::RegisterNatives(g_natives);
    CanvasGradient::RegisterNatives(g_natives);
    CanvasImage::RegisterNatives(g_natives);
    CanvasPath::RegisterNatives(g_natives);
    CanvasPathMeasure::RegisterNatives(g_natives);
    Codec::RegisterNatives(g_natives);
    ColorFilter::RegisterNatives(g_natives);
    DartRuntimeHooks::RegisterNatives(g_natives);
    EngineLayer::RegisterNatives(g_natives);
    FontCollection::RegisterNatives(g_natives);
    FragmentProgram::RegisterNatives(g_natives);
    ImageDescriptor::RegisterNatives(g_natives);
    ImageFilter::RegisterNatives(g_natives);
    ImageShader::RegisterNatives(g_natives);
    ImmutableBuffer::RegisterNatives(g_natives);
    IsolateNameServerNatives::RegisterNatives(g_natives);
    NativeStringAttribute::RegisterNatives(g_natives);
    Paragraph::RegisterNatives(g_natives);
    ParagraphBuilder::RegisterNatives(g_natives);
    Picture::RegisterNatives(g_natives);
    PictureRecorder::RegisterNatives(g_natives);
    Scene::RegisterNatives(g_natives);
    SceneBuilder::RegisterNatives(g_natives);
    SemanticsUpdate::RegisterNatives(g_natives);
    SemanticsUpdateBuilder::RegisterNatives(g_natives);
    Vertices::RegisterNatives(g_natives);
    PlatformConfiguration::RegisterNatives(g_natives);
  }
}

typedef CanvasImage Image;
typedef CanvasPathMeasure PathMeasure;
typedef CanvasGradient Gradient;
typedef CanvasPath Path;

// Functions.
#define FFI_FUNCTION_LIST(V)                               \
  /* Constructors */                                       \
  V(Canvas::CreateOrThrow, 6)                              \
  V(ColorFilter::CreateOrThrow, 1)                         \
  V(FragmentProgram::CreateOrThrow, 1)                     \
  V(Gradient::CreateOrThrow, 1)                            \
  V(ImageFilter::CreateOrThrow, 1)                         \
  V(ImageShader::CreateOrThrow, 1)                         \
  V(ParagraphBuilder::CreateOrThrow, 9)                    \
  V(PathMeasure::CreateOrThrow, 3)                         \
  V(Path::CreateOrThrow, 1)                                \
  V(PictureRecorder::CreateOrThrow, 1)                     \
  V(SceneBuilder::CreateOrThrow, 1)                        \
  V(SemanticsUpdateBuilder::CreateOrThrow, 1)              \
  /* Other */                                              \
  V(ImageDescriptor::initEncodedHandle, 3)                 \
  V(ImmutableBuffer::initHandle, 3)                        \
  V(ImageDescriptor::initRaw, 6)                           \
  V(IsolateNameServerNatives::LookupPortByName, 1)         \
  V(IsolateNameServerNatives::RegisterPortWithName, 2)     \
  V(IsolateNameServerNatives::RemovePortNameMapping, 1)    \
  V(NativeStringAttribute::initLocaleStringAttribute, 4)   \
  V(NativeStringAttribute::initSpellOutStringAttribute, 3) \
  V(Vertices::init, 6)                                     \
  V(FontCollection::LoadFontFromListOrThrowHandle, 3)

// Instance methods.
#define FFI_METHOD_LIST(V)                             \
  V(Image, dispose, 1)                                 \
  V(Image, width, 1)                                   \
  V(Image, height, 1)                                  \
  V(Image, toByteData, 3)                              \
  V(Canvas, clipPath, 3)                               \
  V(Canvas, clipRect, 7)                               \
  V(Canvas, clipRRect, 3)                              \
  V(Canvas, drawArcHandle, 10)                         \
  V(Canvas, drawAtlasHandle, 10)                       \
  V(Canvas, drawCircleHandle, 6)                       \
  V(Canvas, drawColor, 3)                              \
  V(Canvas, drawDRRectHandle, 5)                       \
  V(Canvas, drawImageHandle, 7)                        \
  V(Canvas, drawImageNineHandle, 13)                   \
  V(Canvas, drawImageRectHandle, 13)                   \
  V(Canvas, drawLineHandle, 7)                         \
  V(Canvas, drawOvalHandle, 7)                         \
  V(Canvas, drawPaintHandle, 3)                        \
  V(Canvas, drawPathHandle, 4)                         \
  V(Canvas, drawPicture, 2)                            \
  V(Canvas, drawPointsHandle, 5)                       \
  V(Canvas, drawRRectHandle, 4)                        \
  V(Canvas, drawRectHandle, 7)                         \
  V(Canvas, drawShadow, 5)                             \
  V(Canvas, drawVerticesHandle, 5)                     \
  V(Canvas, getSaveCount, 1)                           \
  V(Canvas, restore, 1)                                \
  V(Canvas, rotate, 2)                                 \
  V(Canvas, save, 1)                                   \
  V(Canvas, saveLayerHandle, 7)                        \
  V(Canvas, saveLayerWithoutBoundsHandle, 3)           \
  V(Canvas, scale, 3)                                  \
  V(Canvas, skew, 3)                                   \
  V(Canvas, transform, 2)                              \
  V(Canvas, translate, 3)                              \
  V(Codec, dispose, 1)                                 \
  V(Codec, frameCount, 1)                              \
  V(Codec, getNextFrame, 2)                            \
  V(Codec, repetitionCount, 1)                         \
  V(ColorFilter, initLinearToSrgbGamma, 1)             \
  V(ColorFilter, initMatrix, 2)                        \
  V(ColorFilter, initMode, 3)                          \
  V(ColorFilter, initSrgbToLinearGamma, 1)             \
  V(EngineLayer, dispose, 1)                           \
  V(FragmentProgram, init, 3)                          \
  V(FragmentProgram, shader, 3)                        \
  V(Gradient, initLinear, 6)                           \
  V(Gradient, initRadial, 8)                           \
  V(Gradient, initSweep, 9)                            \
  V(Gradient, initTwoPointConical, 11)                 \
  V(ImageDescriptor, bytesPerPixel, 1)                 \
  V(ImageDescriptor, dispose, 1)                       \
  V(ImageDescriptor, height, 1)                        \
  V(ImageDescriptor, instantiateCodec, 4)              \
  V(ImageDescriptor, width, 1)                         \
  V(ImageFilter, initBlur, 4)                          \
  V(ImageFilter, initColorFilter, 2)                   \
  V(ImageFilter, initComposeFilter, 3)                 \
  V(ImageFilter, initMatrix, 3)                        \
  V(ImageShader, initWithImage, 6)                     \
  V(ImmutableBuffer, dispose, 1)                       \
  V(ImmutableBuffer, length, 1)                        \
  V(ParagraphBuilder, addPlaceholder, 6)               \
  V(ParagraphBuilder, addText, 2)                      \
  V(ParagraphBuilder, build, 2)                        \
  V(ParagraphBuilder, pop, 1)                          \
  V(ParagraphBuilder, pushStyle, 15)                   \
  V(Paragraph, alphabeticBaseline, 1)                  \
  V(Paragraph, computeLineMetrics, 1)                  \
  V(Paragraph, didExceedMaxLines, 1)                   \
  V(Paragraph, getLineBoundary, 2)                     \
  V(Paragraph, getPositionForOffset, 3)                \
  V(Paragraph, getRectsForPlaceholders, 1)             \
  V(Paragraph, getRectsForRange, 5)                    \
  V(Paragraph, getWordBoundary, 2)                     \
  V(Paragraph, height, 1)                              \
  V(Paragraph, ideographicBaseline, 1)                 \
  V(Paragraph, layout, 2)                              \
  V(Paragraph, longestLine, 1)                         \
  V(Paragraph, maxIntrinsicWidth, 1)                   \
  V(Paragraph, minIntrinsicWidth, 1)                   \
  V(Paragraph, paint, 4)                               \
  V(Paragraph, width, 1)                               \
  V(PathMeasure, setPath, 3)                           \
  V(PathMeasure, getLength, 2)                         \
  V(PathMeasure, getPosTan, 3)                         \
  V(PathMeasure, getSegment, 6)                        \
  V(PathMeasure, isClosed, 2)                          \
  V(PathMeasure, nextContour, 1)                       \
  V(Path, addArc, 7)                                   \
  V(Path, addOval, 5)                                  \
  V(Path, addPath, 4)                                  \
  V(Path, addPathWithMatrixHandle, 5)                  \
  V(Path, addPolygon, 3)                               \
  V(Path, addRRect, 2)                                 \
  V(Path, addRect, 5)                                  \
  V(Path, arcTo, 8)                                    \
  V(Path, arcToPoint, 8)                               \
  V(Path, clone, 2)                                    \
  V(Path, close, 1)                                    \
  V(Path, conicTo, 6)                                  \
  V(Path, contains, 3)                                 \
  V(Path, cubicTo, 7)                                  \
  V(Path, extendWithPath, 4)                           \
  V(Path, extendWithPathAndMatrixHandle, 5)            \
  V(Path, getBounds, 1)                                \
  V(Path, getFillType, 1)                              \
  V(Path, lineTo, 3)                                   \
  V(Path, moveTo, 3)                                   \
  V(Path, op, 4)                                       \
  V(Path, quadraticBezierTo, 5)                        \
  V(Path, relativeArcToPoint, 5)                       \
  V(Path, relativeConicTo, 8)                          \
  V(Path, relativeCubicTo, 7)                          \
  V(Path, relativeLineTo, 3)                           \
  V(Path, relativeMoveTo, 3)                           \
  V(Path, relativeQuadraticBezierTo, 5)                \
  V(Path, reset, 1)                                    \
  V(Path, setFillType, 2)                              \
  V(Path, shift, 4)                                    \
  V(Path, transformHandle, 3)                          \
  V(PictureRecorder, endRecording, 2)                  \
  V(Picture, GetAllocationSize, 1)                     \
  V(Picture, dispose, 1)                               \
  V(Picture, toImage, 4)                               \
  V(SceneBuilder, addPerformanceOverlay, 6)            \
  V(SceneBuilder, addPicture, 5)                       \
  V(SceneBuilder, addPlatformView, 6)                  \
  V(SceneBuilder, addRetained, 2)                      \
  V(SceneBuilder, addTexture, 8)                       \
  V(SceneBuilder, build, 2)                            \
  V(SceneBuilder, pop, 1)                              \
  V(SceneBuilder, pushBackdropFilter, 5)               \
  V(SceneBuilder, pushClipPath, 5)                     \
  V(SceneBuilder, pushClipRRect, 5)                    \
  V(SceneBuilder, pushClipRect, 8)                     \
  V(SceneBuilder, pushColorFilter, 4)                  \
  V(SceneBuilder, pushImageFilter, 4)                  \
  V(SceneBuilder, pushOffset, 5)                       \
  V(SceneBuilder, pushOpacity, 6)                      \
  V(SceneBuilder, pushPhysicalShape, 8)                \
  V(SceneBuilder, pushShaderMask, 10)                  \
  V(SceneBuilder, pushTransformHandle, 4)              \
  V(SceneBuilder, setCheckerboardOffscreenLayers, 2)   \
  V(SceneBuilder, setCheckerboardRasterCacheImages, 2) \
  V(SceneBuilder, setRasterizerTracingThreshold, 2)    \
  V(Scene, dispose, 1)                                 \
  V(Scene, toImage, 4)                                 \
  V(SemanticsUpdateBuilder, build, 2)                  \
  V(SemanticsUpdateBuilder, updateCustomAction, 5)     \
  V(SemanticsUpdateBuilder, updateNode, 36)            \
  V(SemanticsUpdate, dispose, 1)

#define FFI_FUNCTION_MATCH(FUNCTION, ARGS)                                 \
  if (strcmp(name, #FUNCTION) == 0 && args == ARGS) {                      \
    return reinterpret_cast<void*>(                                        \
        tonic::FfiDispatcher<void, decltype(&FUNCTION), &FUNCTION>::Call); \
  }

#define FFI_METHOD_MATCH(CLASS, METHOD, ARGS)                   \
  if (strcmp(name, #CLASS "::" #METHOD) == 0 && args == ARGS) { \
    return reinterpret_cast<void*>(                             \
        tonic::FfiDispatcher<CLASS, decltype(&CLASS::METHOD),   \
                             &CLASS::METHOD>::Call);            \
  }

void* ResolveFfiNativeFunction(const char* name, uintptr_t args) {
  FFI_FUNCTION_LIST(FFI_FUNCTION_MATCH)
  FFI_METHOD_LIST(FFI_METHOD_MATCH)
  return nullptr;
}

void DartUI::InitForIsolate() {
  FML_DCHECK(g_natives);
  auto lib = Dart_LookupLibrary(ToDart("dart:ui"));
  Dart_Handle result =
      Dart_SetNativeResolver(lib, GetNativeFunction, GetSymbol);
  if (Dart_IsError(result)) {
    Dart_PropagateError(result);
  }
  // Set up FFI Native resolver for dart:ui.
  result = Dart_SetFfiNativeResolver(lib, ResolveFfiNativeFunction);
  if (Dart_IsError(result)) {
    Dart_PropagateError(result);
  }
}

}  // namespace flutter
