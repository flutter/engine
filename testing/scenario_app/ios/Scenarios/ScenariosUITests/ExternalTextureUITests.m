//
//  ExternalTextureUITests.m
//  ScenariosUITests
//
//  Created by lujunchen on 2019/9/4.
//  Copyright © 2019 flutter. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Flutter/flutter.h>
#include <sys/sysctl.h>

@interface ExternalTextureUITests : XCTestCase
@property(nonatomic, strong) XCUIApplication* application;
@end

@implementation ExternalTextureUITests

- (void)setUp {
  // Put setup code here. This method is called before the invocation of each test method in the class.
  
  // In UI tests it is usually best to stop immediately when a failure occurs.
  self.continueAfterFailure = NO;

  // UI tests must launch the application that they test. Doing this in setup will make sure it happens for each test method.
  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[ @"--external-texture" ];
  [self.application launch];

  // In UI tests it’s important to set the initial state - such as interface orientation - required for your tests before they run. The setUp method is a good place to do this.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample {
  // Use recording to get started writing UI tests.
  // Use XCTAssert and related functions to verify your tests produce the correct results.
  NSBundle* bundle = [NSBundle bundleForClass:[self class]];
  NSString* goldenName =
  [NSString stringWithFormat:@"golden_external_texture_%@", [self platformName]];
  NSString* path = [bundle pathForResource:goldenName ofType:@"png"];
  UIImage* golden = [[UIImage alloc] initWithContentsOfFile:path];
  
  XCUIScreenshot* screenshot = [[XCUIScreen mainScreen] screenshot];
  XCTAttachment* attachment = [XCTAttachment attachmentWithScreenshot:screenshot];
  attachment.lifetime = XCTAttachmentLifetimeKeepAlways;
  [self addAttachment:attachment];
  
  if (golden) {
    XCTAttachment* goldenAttachment = [XCTAttachment attachmentWithImage:golden];
    goldenAttachment.lifetime = XCTAttachmentLifetimeKeepAlways;
    [self addAttachment:goldenAttachment];
  } else {
    
    NSString *folder = [NSString stringWithFormat:@"%@/",NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)[0]];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:folder]) {
      NSError *error = nil;
      [fileManager createDirectoryAtPath:folder withIntermediateDirectories:YES attributes:nil error:&error];
      if (error) {
      }
    }
    NSString *imgPath = [[folder stringByAppendingPathComponent:goldenName] stringByAppendingPathExtension:@"png"];
    [self writeImageToFile:screenshot.image atPath:imgPath];
    
    XCTFail(@"This test will fail - no golden named %@ found. Follow the steps in the "
            @"README to add a new golden.",
            goldenName);
  }
  
  XCTAssertTrue([self compareImage:golden toOther:screenshot.image]);
}

- (NSString*)platformName {
  NSString* simulatorName =
  [[NSProcessInfo processInfo].environment objectForKey:@"SIMULATOR_DEVICE_NAME"];
  if (simulatorName) {
    return [NSString stringWithFormat:@"%@_simulator", simulatorName];
  }
  
  size_t size;
  sysctlbyname("hw.model", NULL, &size, NULL, 0);
  char* answer = malloc(size);
  sysctlbyname("hw.model", answer, &size, NULL, 0);
  
  NSString* results = [NSString stringWithCString:answer encoding:NSUTF8StringEncoding];
  free(answer);
  return results;
}

- (BOOL)writeImageToFile:(UIImage *)image atPath:(NSString *)aPath {
  if ((image == nil) || (aPath == nil) || ([aPath isEqualToString:@""])) {
    return NO;
  }
  
  NSFileManager * fileManager = [NSFileManager defaultManager];
  BOOL isDirectory;
  NSString * directory = [aPath stringByDeletingLastPathComponent];
  BOOL exist = [fileManager fileExistsAtPath:directory isDirectory:&isDirectory];
  if (!exist) {
    [fileManager createDirectoryAtPath:directory withIntermediateDirectories:YES attributes:nil error:nil];
  }
  
  @try {
    NSData *imageData = nil;
    NSString *ext = [aPath pathExtension];
    if ([ext isEqualToString:@"png"]) {
      imageData = UIImagePNGRepresentation(image);
    } else {
      imageData = UIImageJPEGRepresentation(image, 1.0);
    }
    
    if ((imageData == nil) || ([imageData length] <= 0)) {
      return NO;
    }
    
    BOOL success = [imageData writeToFile:aPath atomically:YES];
    return success;
  } @catch (NSException *e) {
    //NSLog(@"create thumbnail exception.");
  }
  return NO;
}

- (BOOL)compareImage:(UIImage*)a toOther:(UIImage*)b {
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
  NSMutableData* rawA = [NSMutableData dataWithLength:size];
  NSMutableData* rawB = [NSMutableData dataWithLength:size];
  
  if (!rawA || !rawB) {
    return NO;
  }
  
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  
  NSUInteger bytesPerRow = bytesPerPixel * widthA;
  NSUInteger bitsPerComponent = 8;
  CGContextRef contextA =
  CGBitmapContextCreate(rawA.mutableBytes, widthA, heightA, bitsPerComponent, bytesPerRow,
                        colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
  
  CGContextDrawImage(contextA, CGRectMake(0, 0, widthA, heightA), imageRefA);
  CGContextRelease(contextA);
  
  CGContextRef contextB =
  CGBitmapContextCreate(rawB.mutableBytes, widthA, heightA, bitsPerComponent, bytesPerRow,
                        colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
  CGColorSpaceRelease(colorSpace);
  
  CGContextDrawImage(contextB, CGRectMake(0, 0, widthA, heightA), imageRefB);
  CGContextRelease(contextB);
  
  if (memcmp(rawA.bytes, rawB.bytes, rawA.length)) {
    return NO;
  }
  
  return YES;
}

@end
