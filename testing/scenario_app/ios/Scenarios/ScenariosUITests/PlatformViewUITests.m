// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/sysctl.h>
#import <Flutter/flutter.h>
#import <XCTest/XCTest.h>

#import "../Scenarios/TextPlatformView.h"



@interface PlatformViewUITests : XCTestCase
@property(nonatomic, strong)XCUIApplication* application;
@end

@implementation PlatformViewUITests

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;

  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[ @"--platform-view" ];
  [self.application launch];
  
}

- (void)testPlatformView {
  NSBundle* bundle = [NSBundle bundleForClass:[self class]];
  NSString* goldenName = [NSString stringWithFormat:@"golden_platform_view_%@", [self platformName]];
  NSString* path = [bundle pathForResource:goldenName ofType:@"png"];
  UIImage* golden = [[UIImage alloc] initWithContentsOfFile:path];
  
  XCUIScreenshot* screenshot = [[XCUIScreen mainScreen] screenshot];
  if (golden) {
    XCTAttachment* goldenAttachment = [XCTAttachment attachmentWithImage:golden];
    goldenAttachment.lifetime = XCTAttachmentLifetimeKeepAlways;
    [self addAttachment:goldenAttachment];
  }

  XCTAttachment* attachment = [XCTAttachment attachmentWithScreenshot:screenshot];
  attachment.lifetime = XCTAttachmentLifetimeKeepAlways;
  [self addAttachment:attachment];
  XCTAssertTrue([self compareImage:golden toOther:screenshot.image]);
}

- (NSString*) platformName {
  NSString* simulatorName = [[NSProcessInfo processInfo].environment objectForKey:@"SIMULATOR_DEVICE_NAME"];
  if (simulatorName) {
    return [NSString stringWithFormat:@"%@_simulator", simulatorName];
  }
  
  size_t size;
  sysctlbyname("hw.model", NULL, &size, NULL, 0);
  char* answer = malloc(size);
  sysctlbyname("hw.model", answer, &size, NULL, 0);
  
  NSString *results = [NSString stringWithCString:answer encoding: NSUTF8StringEncoding];
  free(answer);
  return results;
}

- (BOOL) compareImage:(UIImage*)a toOther:(UIImage*)b {
  CGImageRef imageRefA = [a CGImage];
  CGImageRef imageRefB = [b CGImage];
  
  NSUInteger widthA = CGImageGetWidth(imageRefA);
  NSUInteger heightA = CGImageGetHeight(imageRefA);
  NSUInteger widthB = CGImageGetWidth(imageRefB);
  NSUInteger heightB = CGImageGetHeight(imageRefB);
  
  if (widthA != widthB || heightA != heightB) {
    return NO;
  }
  NSUInteger bytesPerPixel = 4;
  NSUInteger size = widthA * heightA * bytesPerPixel;
  unsigned char* rawA = (unsigned char*) calloc(size, sizeof(unsigned char));
  unsigned char* rawB = (unsigned char*) calloc(size, sizeof(unsigned char));
  
  if (!rawA || !rawB) {
    free(rawA);
    free(rawB);
    return NO;
  }
  
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

  NSUInteger bytesPerRow = bytesPerPixel * widthA;
  NSUInteger bitsPerComponent = 8;
  CGContextRef contextA = CGBitmapContextCreate(rawA, widthA, heightA,
                                               bitsPerComponent, bytesPerRow, colorSpace,
                                               kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
  
  CGContextDrawImage(contextA, CGRectMake(0, 0, widthA, heightA), imageRefA);
  CGContextRelease(contextA);
  
  CGContextRef contextB = CGBitmapContextCreate(rawB, widthA, heightA,
                                                bitsPerComponent, bytesPerRow, colorSpace,
                                                kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
  CGColorSpaceRelease(colorSpace);
  
  CGContextDrawImage(contextB, CGRectMake(0, 0, widthA, heightA), imageRefB);
  CGContextRelease(contextB);
  
  for (int i = 0 ; i < size; ++i)
  {
    if (rawA[i] != rawB[i]) {
      free(rawA);
      free(rawB);
      return NO;
    }
  }
  
  free(rawA);
  free(rawB);
  return YES;
}

@end
