// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "TextPlatformView.h"

@protocol TestGestureRecognizerDelegate <NSObject>

- (void)gestureTouchesBegan;
- (void)gestureTouchesEnded;

@end

@interface TestTapGestureRecognizer : UITapGestureRecognizer

@property(weak, nonatomic)
    NSObject<TestGestureRecognizerDelegate>* testTapGestureRecognizerDelegate;

@end

@implementation TestTapGestureRecognizer

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
  [self.testTapGestureRecognizerDelegate gestureTouchesBegan];
  [super touchesBegan:touches withEvent:event];
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
  [self.testTapGestureRecognizerDelegate gestureTouchesEnded];
  [super touchesEnded:touches withEvent:event];
}

@end

@implementation TextPlatformViewFactory {
  NSObject<FlutterBinaryMessenger>* _messenger;
}

- (instancetype)initWithMessenger:(NSObject<FlutterBinaryMessenger>*)messenger {
  self = [super init];
  if (self) {
    _messenger = messenger;
  }
  return self;
}

- (NSObject<FlutterPlatformView>*)createWithFrame:(CGRect)frame
                                   viewIdentifier:(int64_t)viewId
                                        arguments:(id _Nullable)args {
  TextPlatformView* textPlatformView = [[TextPlatformView alloc] initWithFrame:frame
                                                                viewIdentifier:viewId
                                                                     arguments:args
                                                               binaryMessenger:_messenger];
  return textPlatformView;
}

- (NSObject<FlutterMessageCodec>*)createArgsCodec {
  return [FlutterStringCodec sharedInstance];
}

@end

@interface TextPlatformView () <TestGestureRecognizerDelegate>

@end

@implementation TextPlatformView {
  UIView* _containerView;
  FlutterMethodChannel* _channel;
  BOOL _viewCreated;
}

- (instancetype)initWithFrame:(CGRect)frame
               viewIdentifier:(int64_t)viewId
                    arguments:(id _Nullable)args
              binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger {
  if ([super init]) {
    _containerView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 250, 100)];
    _containerView.backgroundColor = UIColor.lightGrayColor;
    _containerView.clipsToBounds = YES;
    _containerView.accessibilityIdentifier = @"platform_view";

    UITextView* textView = [[UITextView alloc] initWithFrame:CGRectMake(50.0, 50.0, 250, 100)];
    textView.backgroundColor = UIColor.lightGrayColor;
    textView.textColor = UIColor.blueColor;
    [textView setFont:[UIFont systemFontOfSize:52]];
    textView.text = args;
    textView.autoresizingMask =
        (UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight);
    [_containerView addSubview:textView];

    TestTapGestureRecognizer* gestureRecognizer =
        [[TestTapGestureRecognizer alloc] initWithTarget:self action:@selector(platformViewTapped)];
    gestureRecognizer.testTapGestureRecognizerDelegate = self;

    [_containerView addGestureRecognizer:gestureRecognizer];
    _containerView.accessibilityLabel = @"";

    _viewCreated = NO;
  }
  return self;
}

- (UIView*)view {
  // Makes sure the engine only calls this method once.
  if (_viewCreated) {
    abort();
  }
  _viewCreated = YES;
  return _containerView;
}

- (void)platformViewTapped {
  _containerView.accessibilityLabel =
      [_containerView.accessibilityLabel stringByAppendingString:@"-platformViewTapped"];
}

- (void)gestureTouchesBegan {
  _containerView.accessibilityLabel =
      [_containerView.accessibilityLabel stringByAppendingString:@"-gestureTouchesBegan"];
}

- (void)gestureTouchesEnded {
  _containerView.accessibilityLabel =
      [_containerView.accessibilityLabel stringByAppendingString:@"-gestureTouchesEnded"];
}

@end
