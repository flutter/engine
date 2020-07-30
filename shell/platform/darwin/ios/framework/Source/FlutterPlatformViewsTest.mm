// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterBinaryMessenger.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"
#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterPlatformViews.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterPlatformViews_Internal.h"
#import "flutter/shell/platform/darwin/ios/platform_view_ios.h"
#import "third_party/ocmock/Source/OCMock/OCMock.h"

FLUTTER_ASSERT_NOT_ARC
@class FlutterPlatformViewsTestMockPlatformView;
static FlutterPlatformViewsTestMockPlatformView* gMockPlatformView = nil;
const float kFloatCompareEpsilon = 0.001;

@interface FlutterPlatformViewsTestMockPlatformView : UIView
@end
@implementation FlutterPlatformViewsTestMockPlatformView

- (instancetype)init {
  self = [super init];
  if (self) {
    gMockPlatformView = self;
  }
  return self;
}

- (void)dealloc {
  gMockPlatformView = nil;
  [super dealloc];
}

@end

@interface FlutterPlatformViewsTestMockFlutterPlatformView : NSObject <FlutterPlatformView>
@property(nonatomic, strong) UIView* view;
@end

@implementation FlutterPlatformViewsTestMockFlutterPlatformView

- (instancetype)init {
  if (self = [super init]) {
    _view = [[FlutterPlatformViewsTestMockPlatformView alloc] init];
  }
  return self;
}

- (void)dealloc {
  [_view release];
  _view = nil;
  [super dealloc];
}

@end

@interface FlutterPlatformViewsTestMockFlutterPlatformFactory
    : NSObject <FlutterPlatformViewFactory>
@end

@implementation FlutterPlatformViewsTestMockFlutterPlatformFactory
- (NSObject<FlutterPlatformView>*)createWithFrame:(CGRect)frame
                                   viewIdentifier:(int64_t)viewId
                                        arguments:(id _Nullable)args {
  return [[[FlutterPlatformViewsTestMockFlutterPlatformView alloc] init] autorelease];
}

@end

namespace flutter {
namespace {
class FlutterPlatformViewsTestMockPlatformViewDelegate : public PlatformView::Delegate {
  void OnPlatformViewCreated(std::unique_ptr<Surface> surface) override {}
  void OnPlatformViewDestroyed() override {}
  void OnPlatformViewSetNextFrameCallback(const fml::closure& closure) override {}
  void OnPlatformViewSetViewportMetrics(const ViewportMetrics& metrics) override {}
  void OnPlatformViewDispatchPlatformMessage(fml::RefPtr<PlatformMessage> message) override {}
  void OnPlatformViewDispatchPointerDataPacket(std::unique_ptr<PointerDataPacket> packet) override {
  }
  void OnPlatformViewDispatchSemanticsAction(int32_t id,
                                             SemanticsAction action,
                                             std::vector<uint8_t> args) override {}
  void OnPlatformViewSetSemanticsEnabled(bool enabled) override {}
  void OnPlatformViewSetAccessibilityFeatures(int32_t flags) override {}
  void OnPlatformViewRegisterTexture(std::shared_ptr<Texture> texture) override {}
  void OnPlatformViewUnregisterTexture(int64_t texture_id) override {}
  void OnPlatformViewMarkTextureFrameAvailable(int64_t texture_id) override {}

  std::unique_ptr<std::vector<std::string>> ComputePlatformViewResolvedLocale(
      const std::vector<std::string>& supported_locale_data) override {
    std::unique_ptr<std::vector<std::string>> out = std::make_unique<std::vector<std::string>>();
    return out;
  }
};

}  // namespace
}  // namespace flutter

namespace {
fml::RefPtr<fml::TaskRunner> CreateNewThread(std::string name) {
  auto thread = std::make_unique<fml::Thread>(name);
  auto runner = thread->GetTaskRunner();
  return runner;
}
}  // namespace

@interface FlutterPlatformViewsTest : XCTestCase
@end

@implementation FlutterPlatformViewsTest

- (void)testCanCreatePlatformViewWithoutFlutterView {
  flutter::FlutterPlatformViewsTestMockPlatformViewDelegate mock_delegate;
  auto thread_task_runner = CreateNewThread("FlutterPlatformViewsTest");
  flutter::TaskRunners runners(/*label=*/self.name.UTF8String,
                               /*platform=*/thread_task_runner,
                               /*raster=*/thread_task_runner,
                               /*ui=*/thread_task_runner,
                               /*io=*/thread_task_runner);
  auto platform_view = std::make_unique<flutter::PlatformViewIOS>(
      /*delegate=*/mock_delegate,
      /*rendering_api=*/flutter::IOSRenderingAPI::kSoftware,
      /*task_runners=*/runners);

  auto flutterPlatformViewsController = std::make_unique<flutter::FlutterPlatformViewsController>();

  FlutterPlatformViewsTestMockFlutterPlatformFactory* factory =
      [[FlutterPlatformViewsTestMockFlutterPlatformFactory new] autorelease];
  flutterPlatformViewsController->RegisterViewFactory(
      factory, @"MockFlutterPlatformView",
      FlutterPlatformViewGestureRecognizersBlockingPolicyEager);
  FlutterResult result = ^(id result) {
  };
  flutterPlatformViewsController->OnMethodCall(
      [FlutterMethodCall
          methodCallWithMethodName:@"create"
                         arguments:@{@"id" : @2, @"viewType" : @"MockFlutterPlatformView"}],
      result);

  XCTAssertNotNil(gMockPlatformView);

  flutterPlatformViewsController->Reset();
}

- (void)testChildClippingViewHitTests {
  ChildClippingView *childCilppingView = [[[ChildClippingView alloc] initWithFrame:CGRectMake(0, 0, 500, 500)] autorelease];
  UIView *childView = [[[UIView alloc] initWithFrame:CGRectMake(100, 100, 100, 100)] autorelease];
  [childCilppingView addSubview:childView];

  XCTAssertFalse([childCilppingView pointInside:CGPointMake(50, 50) withEvent:nil]);
  XCTAssertTrue([childCilppingView pointInside:CGPointMake(150, 150) withEvent:nil]);
}

- (void)testCompositePlatformView {
  flutter::FlutterPlatformViewsTestMockPlatformViewDelegate mock_delegate;
  auto thread_task_runner = CreateNewThread("FlutterPlatformViewsTest");
  flutter::TaskRunners runners(/*label=*/self.name.UTF8String,
                               /*platform=*/thread_task_runner,
                               /*raster=*/thread_task_runner,
                               /*ui=*/thread_task_runner,
                               /*io=*/thread_task_runner);
  auto platform_view = std::make_unique<flutter::PlatformViewIOS>(
      /*delegate=*/mock_delegate,
      /*rendering_api=*/flutter::IOSRenderingAPI::kSoftware,
      /*task_runners=*/runners);

  auto flutterPlatformViewsController = std::make_unique<flutter::FlutterPlatformViewsController>();

  FlutterPlatformViewsTestMockFlutterPlatformFactory* factory =
      [[FlutterPlatformViewsTestMockFlutterPlatformFactory new] autorelease];
  flutterPlatformViewsController->RegisterViewFactory(
      factory, @"MockFlutterPlatformView",
      FlutterPlatformViewGestureRecognizersBlockingPolicyEager);
  FlutterResult result = ^(id result) {
  };
  flutterPlatformViewsController->OnMethodCall(
      [FlutterMethodCall
          methodCallWithMethodName:@"create"
                         arguments:@{@"id" : @2, @"viewType" : @"MockFlutterPlatformView"}],
      result);

  XCTAssertNotNil(gMockPlatformView);

  UIView *mockFlutterView = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, 500, 500)] autorelease];
  flutterPlatformViewsController->SetFlutterView(mockFlutterView);
  // Create embedded view params
  flutter::MutatorsStack stack;
  // Layer tree always pushes a screen scale factor to the stack
  SkMatrix screenScaleMatrix = SkMatrix::MakeScale([UIScreen mainScreen].scale, [UIScreen mainScreen].scale);
  stack.PushTransform(screenScaleMatrix);
  // Push a translate matrix
  SkMatrix translateMatrix = SkMatrix::MakeTrans(100, 100);
  stack.PushTransform(translateMatrix);
  SkMatrix finalMatrix;
  finalMatrix.setConcat(screenScaleMatrix, translateMatrix);

  auto embeddedViewParams = std::make_unique<flutter::EmbeddedViewParams>(finalMatrix, SkSize::Make(300, 300), stack);

  flutterPlatformViewsController->PrerollCompositeEmbeddedView(2, std::move(embeddedViewParams));
  flutterPlatformViewsController->CompositeEmbeddedView(2);
  CGRect platformViewRectInFlutterView = [gMockPlatformView convertRect:gMockPlatformView.bounds
                                                    toView:mockFlutterView];
  XCTAssertTrue(CGRectEqualToRect(platformViewRectInFlutterView, CGRectMake(100, 100, 300, 300)));
  flutterPlatformViewsController->Reset();
}

- (void)testChildClippingViewShouldBeTheBoundingRectOfPlatformView {
  flutter::FlutterPlatformViewsTestMockPlatformViewDelegate mock_delegate;
  auto thread_task_runner = CreateNewThread("FlutterPlatformViewsTest");
  flutter::TaskRunners runners(/*label=*/self.name.UTF8String,
                               /*platform=*/thread_task_runner,
                               /*raster=*/thread_task_runner,
                               /*ui=*/thread_task_runner,
                               /*io=*/thread_task_runner);
  auto platform_view = std::make_unique<flutter::PlatformViewIOS>(
      /*delegate=*/mock_delegate,
      /*rendering_api=*/flutter::IOSRenderingAPI::kSoftware,
      /*task_runners=*/runners);

  auto flutterPlatformViewsController = std::make_unique<flutter::FlutterPlatformViewsController>();

  FlutterPlatformViewsTestMockFlutterPlatformFactory* factory =
      [[FlutterPlatformViewsTestMockFlutterPlatformFactory new] autorelease];
  flutterPlatformViewsController->RegisterViewFactory(
      factory, @"MockFlutterPlatformView",
      FlutterPlatformViewGestureRecognizersBlockingPolicyEager);
  FlutterResult result = ^(id result) {
  };
  flutterPlatformViewsController->OnMethodCall(
      [FlutterMethodCall
          methodCallWithMethodName:@"create"
                         arguments:@{@"id" : @2, @"viewType" : @"MockFlutterPlatformView"}],
      result);

  XCTAssertNotNil(gMockPlatformView);

  UIView *mockFlutterView = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, 500, 500)] autorelease];
  flutterPlatformViewsController->SetFlutterView(mockFlutterView);
  // Create embedded view params
  flutter::MutatorsStack stack;
  // Layer tree always pushes a screen scale factor to the stack
  SkMatrix screenScaleMatrix = SkMatrix::MakeScale([UIScreen mainScreen].scale, [UIScreen mainScreen].scale);
  stack.PushTransform(screenScaleMatrix);
  // Push a rotate matrix
  SkMatrix rotateMatrix;
  rotateMatrix.setRotate(10);
  stack.PushTransform(rotateMatrix);
  SkMatrix finalMatrix;
  finalMatrix.setConcat(screenScaleMatrix, rotateMatrix);

  auto embeddedViewParams = std::make_unique<flutter::EmbeddedViewParams>(finalMatrix, SkSize::Make(300, 300), stack);

  flutterPlatformViewsController->PrerollCompositeEmbeddedView(2, std::move(embeddedViewParams));
  flutterPlatformViewsController->CompositeEmbeddedView(2);
  CGRect platformViewRectInFlutterView = [gMockPlatformView convertRect:gMockPlatformView.bounds
                                                    toView:mockFlutterView];
  XCTAssertTrue([gMockPlatformView.superview.superview isKindOfClass: ChildClippingView.class]);
  ChildClippingView *childClippingView = (ChildClippingView *)gMockPlatformView.superview.superview;
  // The childclippingview's frame is set based on flow, but the platform view's frame is set based on quartz.
  // Although they should be the same, but we should tolerate small floating point errors.
  XCTAssertTrue(fabs(platformViewRectInFlutterView.origin.x - childClippingView.frame.origin.x) < kFloatCompareEpsilon);
  XCTAssertTrue(fabs(platformViewRectInFlutterView.origin.y - childClippingView.frame.origin.y) < kFloatCompareEpsilon);
  XCTAssertTrue(fabs(platformViewRectInFlutterView.size.width - childClippingView.frame.size.width) < kFloatCompareEpsilon);
  XCTAssertTrue(fabs(platformViewRectInFlutterView.size.height - childClippingView.frame.size.height) < kFloatCompareEpsilon);

  flutterPlatformViewsController->Reset();
}

@end
