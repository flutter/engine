// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "TextPlatformView.h"

@protocol CustomGestureRecognizerDelegate;

@interface CustomGestureRecognizer: UITapGestureRecognizer

@property (weak, nonatomic) NSObject<CustomGestureRecognizerDelegate> *customGestureRecognizerDelegate;

@end

@protocol CustomGestureRecognizerDelegate<NSObject>

- (void)gestureRecognizer:(CustomGestureRecognizer *)gestureRecognizer touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event;
- (void)gestureRecognizer:(CustomGestureRecognizer *)gestureRecognizer touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event;

@end

@implementation CustomGestureRecognizer

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  [self.customGestureRecognizerDelegate gestureRecognizer:self touchesBegan:touches withEvent:event];
  [super touchesBegan:touches withEvent:event];
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  [self.customGestureRecognizerDelegate gestureRecognizer:self touchesEnded:touches withEvent:event];
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

@interface TextPlatformView()<CustomGestureRecognizerDelegate>

@end

@implementation TextPlatformView {
  int64_t _viewId;
  UITextView* _textView;
  FlutterMethodChannel* _channel;
}

- (instancetype)initWithFrame:(CGRect)frame
               viewIdentifier:(int64_t)viewId
                    arguments:(id _Nullable)args
              binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger {
  if ([super init]) {
    _viewId = viewId;
    _textView = [[UITextView alloc] initWithFrame:CGRectMake(50.0, 50.0, 250.0, 100.0)];
    _textView.textColor = UIColor.blueColor;
    _textView.backgroundColor = UIColor.lightGrayColor;
    _textView.userInteractionEnabled = YES;
    [_textView setFont:[UIFont systemFontOfSize:52]];
    _textView.text = args;
    CustomGestureRecognizer *gestureRecognizer = [[CustomGestureRecognizer alloc] initWithTarget:self action:@selector(gestureDidRecognize:)];
    gestureRecognizer.customGestureRecognizerDelegate = self;
    [_textView addGestureRecognizer:gestureRecognizer];
  }
  return self;
}

- (UIView*)view {
  return _textView;
}

- (void)gestureDidRecognize:(UIGestureRecognizer *)gestureRecognizer {
  // An easy way to communicate with XCUITest target
  self.accessibilityLabel = [self.accessibilityLabel stringByAppendingString:@"-gestureRecognized-"];
}

- (void)gestureRecognizer:(CustomGestureRecognizer *)gestureRecognizer touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  // An easy way to communicate with XCUITest target
  self.accessibilityLabel = [self.accessibilityLabel stringByAppendingString:@"-touchesBegan-"];
}

- (void)gestureRecognizer:(CustomGestureRecognizer *)gestureRecognizer touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  // An easy way to communicate with XCUITest target
  self.accessibilityLabel = [self.accessibilityLabel stringByAppendingString:@"-touchesEnded-"];
}

@end
