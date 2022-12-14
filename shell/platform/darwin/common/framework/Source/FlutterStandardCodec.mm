// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <CoreFoundation/CoreFoundation.h>

#import "flutter/shell/platform/darwin/common/framework/Source/FlutterStandardCodec_Internal.h"

FLUTTER_ASSERT_ARC

#pragma mark - Codec for basic message channel

@implementation FlutterStandardMessageCodec {
  FlutterStandardReaderWriter* _readerWriter;
}
+ (instancetype)sharedInstance {
  static id _sharedInstance = nil;
  if (!_sharedInstance) {
    FlutterStandardReaderWriter* readerWriter = [[FlutterStandardReaderWriter alloc] init];
    _sharedInstance = [[FlutterStandardMessageCodec alloc] initWithReaderWriter:readerWriter];
  }
  return _sharedInstance;
}

+ (instancetype)codecWithReaderWriter:(FlutterStandardReaderWriter*)readerWriter {
  return [[FlutterStandardMessageCodec alloc] initWithReaderWriter:readerWriter];
}

- (instancetype)initWithReaderWriter:(FlutterStandardReaderWriter*)readerWriter {
  self = [super init];
  NSAssert(self, @"Super init cannot be nil");
  _readerWriter = readerWriter;
  return self;
}

- (NSData*)encode:(id)message {
  if (message == nil) {
    return nil;
  }
  NSMutableData* data = [NSMutableData dataWithCapacity:32];
  FlutterStandardWriter* writer = [_readerWriter writerWithData:data];
  [writer writeValue:message];
  return data;
}

- (id)decode:(NSData*)message {
  if ([message length] == 0) {
    return nil;
  }
  FlutterStandardReader* reader = [_readerWriter readerWithData:message];
  id value = [reader readValue];
  NSAssert(![reader hasMore], @"Corrupted standard message");
  return value;
}
@end

#pragma mark - Codec for method channel

@implementation FlutterStandardMethodCodec {
  FlutterStandardReaderWriter* _readerWriter;
}
+ (instancetype)sharedInstance {
  static id _sharedInstance = nil;
  if (!_sharedInstance) {
    FlutterStandardReaderWriter* readerWriter = [[FlutterStandardReaderWriter alloc] init];
    _sharedInstance = [[FlutterStandardMethodCodec alloc] initWithReaderWriter:readerWriter];
  }
  return _sharedInstance;
}

+ (instancetype)codecWithReaderWriter:(FlutterStandardReaderWriter*)readerWriter {
  return [[FlutterStandardMethodCodec alloc] initWithReaderWriter:readerWriter];
}

- (instancetype)initWithReaderWriter:(FlutterStandardReaderWriter*)readerWriter {
  self = [super init];
  NSAssert(self, @"Super init cannot be nil");
  _readerWriter = readerWriter;
  return self;
}

- (NSData*)encodeMethodCall:(FlutterMethodCall*)call {
  NSMutableData* data = [NSMutableData dataWithCapacity:32];
  FlutterStandardWriter* writer = [_readerWriter writerWithData:data];
  [writer writeValue:call.method];
  [writer writeValue:call.arguments];
  return data;
}

- (NSData*)encodeSuccessEnvelope:(id)result {
  NSMutableData* data = [NSMutableData dataWithCapacity:32];
  FlutterStandardWriter* writer = [_readerWriter writerWithData:data];
  [writer writeByte:0];
  [writer writeValue:result];
  return data;
}

- (NSData*)encodeErrorEnvelope:(FlutterError*)error {
  NSMutableData* data = [NSMutableData dataWithCapacity:32];
  FlutterStandardWriter* writer = [_readerWriter writerWithData:data];
  [writer writeByte:1];
  [writer writeValue:error.code];
  [writer writeValue:error.message];
  [writer writeValue:error.details];
  return data;
}

- (FlutterMethodCall*)decodeMethodCall:(NSData*)message {
  FlutterStandardReader* reader = [_readerWriter readerWithData:message];
  id value1 = [reader readValue];
  id value2 = [reader readValue];
  NSAssert(![reader hasMore], @"Corrupted standard method call");
  NSAssert([value1 isKindOfClass:[NSString class]], @"Corrupted standard method call");
  return [FlutterMethodCall methodCallWithMethodName:value1 arguments:value2];
}

- (id)decodeEnvelope:(NSData*)envelope {
  FlutterStandardReader* reader = [_readerWriter readerWithData:envelope];
  UInt8 flag = [reader readByte];
  NSAssert(flag <= 1, @"Corrupted standard envelope");
  id result;
  switch (flag) {
    case 0: {
      result = [reader readValue];
      NSAssert(![reader hasMore], @"Corrupted standard envelope");
    } break;
    case 1: {
      id code = [reader readValue];
      id message = [reader readValue];
      id details = [reader readValue];
      NSAssert(![reader hasMore], @"Corrupted standard envelope");
      NSAssert([code isKindOfClass:[NSString class]], @"Invalid standard envelope");
      NSAssert(message == nil || [message isKindOfClass:[NSString class]],
               @"Invalid standard envelope");
      result = [FlutterError errorWithCode:code message:message details:details];
    } break;
  }
  return result;
}
@end

using namespace flutter;

#pragma mark - Standard serializable types

@implementation FlutterStandardTypedData
+ (instancetype)typedDataWithBytes:(NSData*)data {
  return [FlutterStandardTypedData typedDataWithData:data type:FlutterStandardDataTypeUInt8];
}

+ (instancetype)typedDataWithInt32:(NSData*)data {
  return [FlutterStandardTypedData typedDataWithData:data type:FlutterStandardDataTypeInt32];
}

+ (instancetype)typedDataWithInt64:(NSData*)data {
  return [FlutterStandardTypedData typedDataWithData:data type:FlutterStandardDataTypeInt64];
}

+ (instancetype)typedDataWithFloat32:(NSData*)data {
  return [FlutterStandardTypedData typedDataWithData:data type:FlutterStandardDataTypeFloat32];
}

+ (instancetype)typedDataWithFloat64:(NSData*)data {
  return [FlutterStandardTypedData typedDataWithData:data type:FlutterStandardDataTypeFloat64];
}

+ (instancetype)typedDataWithData:(NSData*)data type:(FlutterStandardDataType)type {
  return [[FlutterStandardTypedData alloc] initWithData:data type:type];
}

- (instancetype)initWithData:(NSData*)data type:(FlutterStandardDataType)type {
  UInt8 elementSize = elementSizeForFlutterStandardDataType(type);
  NSAssert(data, @"Data cannot be nil");
  NSAssert(data.length % elementSize == 0, @"Data must contain integral number of elements");
  self = [super init];
  NSAssert(self, @"Super init cannot be nil");
  _data = [data copy];
  _type = type;
  _elementSize = elementSize;
  _elementCount = data.length / elementSize;
  return self;
}

- (BOOL)isEqual:(id)object {
  if (self == object) {
    return YES;
  }
  if (![object isKindOfClass:[FlutterStandardTypedData class]]) {
    return NO;
  }
  FlutterStandardTypedData* other = (FlutterStandardTypedData*)object;
  return self.type == other.type && self.elementCount == other.elementCount &&
         [self.data isEqual:other.data];
}

- (NSUInteger)hash {
  return [self.data hash] ^ self.type;
}
@end

#pragma mark - Writer and reader of standard codec

@implementation FlutterStandardWriter {
  NSMutableData* _data;
}

- (instancetype)initWithData:(NSMutableData*)data {
  self = [super init];
  NSAssert(self, @"Super init cannot be nil");
  _data = data;
  return self;
}

- (void)writeByte:(UInt8)value {
  [_data appendBytes:&value length:1];
}

- (void)writeBytes:(const void*)bytes length:(NSUInteger)length {
  [_data appendBytes:bytes length:length];
}

- (void)writeData:(NSData*)data {
  [_data appendData:data];
}

- (void)writeSize:(UInt32)size {
  if (size < 254) {
    [self writeByte:(UInt8)size];
  } else if (size <= 0xffff) {
    [self writeByte:254];
    UInt16 value = (UInt16)size;
    [self writeBytes:&value length:2];
  } else {
    [self writeByte:255];
    [self writeBytes:&size length:4];
  }
}

- (void)writeAlignment:(UInt8)alignment {
  UInt8 mod = _data.length % alignment;
  if (mod) {
    for (int i = 0; i < (alignment - mod); i++) {
      [self writeByte:0];
    }
  }
}

- (void)writeUTF8:(NSString*)value {
  UInt32 length = [value lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
  [self writeSize:length];
  [self writeBytes:value.UTF8String length:length];
}

static void WriteKeyValuePair(const void* key, const void* value, void* context) {
  [(__bridge FlutterStandardWriter*)context writeValue:(__bridge id)key];
  [(__bridge FlutterStandardWriter*)context writeValue:(__bridge id)value];
}

static void WriteArrayValue(const void* value, void* context) {
  [(__bridge FlutterStandardWriter*)context writeValue:(__bridge id)value];
}

static void WriteValue(FlutterStandardWriter* writer, CFTypeRef value) {
  if (value == nullptr || CFGetTypeID(value) == CFNullGetTypeID()) {
    [writer writeByte:FlutterStandardFieldNil];
  } else if (CFGetTypeID(value) == CFBooleanGetTypeID()) {
    [writer writeByte:(CFBooleanGetValue((CFBooleanRef)value) ? FlutterStandardFieldTrue
                                                              : FlutterStandardFieldFalse)];
  } else if (CFGetTypeID(value) == CFNumberGetTypeID()) {
    CFNumberRef number = (CFNumberRef)value;
    BOOL success = NO;
    if (CFNumberIsFloatType(number)) {
      Float64 f;
      success = CFNumberGetValue(number, kCFNumberFloat64Type, &f);
      if (success) {
        [writer writeByte:FlutterStandardFieldFloat64];
        [writer writeAlignment:8];
        [writer writeBytes:(UInt8*)&f length:8];
      }
    } else if (CFNumberGetByteSize(number) <= 4) {
      SInt32 n;
      success = CFNumberGetValue(number, kCFNumberSInt32Type, &n);
      if (success) {
        [writer writeByte:FlutterStandardFieldInt32];
        [writer writeBytes:(UInt8*)&n length:4];
      }
    } else if (CFNumberGetByteSize(number) <= 8) {
      SInt64 n;
      success = CFNumberGetValue(number, kCFNumberSInt64Type, &n);
      if (success) {
        [writer writeByte:FlutterStandardFieldInt64];
        [writer writeBytes:(UInt8*)&n length:8];
      }
    }
    if (!success) {
      NSLog(@"Unsupported value: %@ of number type %ld", (__bridge id)value,
            CFNumberGetType(number));
      NSCAssert(NO, @"Unsupported value for standard codec");
    }
  } else if (CFGetTypeID(value) == CFStringGetTypeID()) {
    [writer writeByte:FlutterStandardFieldString];
    [writer writeUTF8:(__bridge NSString*)value];
  } else if ([(__bridge id)value isKindOfClass:[FlutterStandardTypedData class]]) {
    FlutterStandardTypedData* typedData = (__bridge id)value;
    [writer writeByte:FlutterStandardFieldForDataType(typedData.type)];
    [writer writeSize:typedData.elementCount];
    [writer writeAlignment:typedData.elementSize];
    [writer writeData:typedData.data];
  } else if (CFGetTypeID(value) == CFDataGetTypeID()) {
    [writer writeValue:[FlutterStandardTypedData typedDataWithBytes:(__bridge NSData*)value]];
  } else if (CFGetTypeID(value) == CFArrayGetTypeID()) {
    CFArrayRef array = (CFArrayRef)value;
    long count = CFArrayGetCount(array);
    [writer writeByte:FlutterStandardFieldList];
    [writer writeSize:count];
    CFArrayApplyFunction(array, CFRangeMake(0, count), WriteArrayValue, (void*)writer);
  } else if (CFGetTypeID(value) == CFDictionaryGetTypeID()) {
    CFDictionaryRef dict = (CFDictionaryRef)value;
    [writer writeByte:FlutterStandardFieldMap];
    [writer writeSize:CFDictionaryGetCount(dict)];
    CFDictionaryApplyFunction(dict, WriteKeyValuePair, (void*)writer);
  } else {
    NSLog(@"Unsupported value: %@ of type %@", (__bridge id)value, [(__bridge id)value class]);
    NSCAssert(NO, @"Unsupported value for standard codec");
  }
}

- (void)writeValue:(id)value {
  WriteValue(self, (__bridge CFTypeRef)value);
}

@end

@implementation FlutterStandardReader {
  NSData* _data;
  NSRange _range;
}

- (instancetype)initWithData:(NSData*)data {
  self = [super init];
  NSAssert(self, @"Super init cannot be nil");
  _data = [data copy];
  _range = NSMakeRange(0, 0);
  return self;
}

- (BOOL)hasMore {
  return _range.location < _data.length;
}

- (void)readBytes:(void*)destination length:(NSUInteger)length {
  _range.length = length;
  [_data getBytes:destination range:_range];
  _range.location += _range.length;
}

- (UInt8)readByte {
  UInt8 value;
  [self readBytes:&value length:1];
  return value;
}

- (UInt32)readSize {
  UInt8 byte = [self readByte];
  if (byte < 254) {
    return (UInt32)byte;
  } else if (byte == 254) {
    UInt16 value;
    [self readBytes:&value length:2];
    return value;
  } else {
    UInt32 value;
    [self readBytes:&value length:4];
    return value;
  }
}

- (CFDataRef)readDataRef:(NSUInteger)length {
  _range.length = length;
  CFDataRef bytes = (__bridge CFDataRef)[_data subdataWithRange:_range];
  _range.location += _range.length;
  return bytes;
}

- (NSData*)readData:(NSUInteger)length {
  return (__bridge NSData*)[self readDataRef:length];
}

static CFTypeRef ReadUTF8(FlutterStandardReader* reader) {
  CFDataRef bytes = [reader readDataRef:[reader readSize]];
  CFStringRef string =
      CFStringCreateWithBytes(kCFAllocatorDefault, CFDataGetBytePtr(bytes), CFDataGetLength(bytes),
                              kCFStringEncodingUTF8, false);
  return CFAutorelease(string);
}

- (NSString*)readUTF8 {
  return (__bridge NSString*)ReadUTF8(self);
}

- (void)readAlignment:(UInt8)alignment {
  UInt8 mod = _range.location % alignment;
  if (mod) {
    _range.location += (alignment - mod);
  }
}

- (FlutterStandardTypedData*)readTypedDataOfType:(FlutterStandardDataType)type {
  UInt32 elementCount = [self readSize];
  UInt8 elementSize = elementSizeForFlutterStandardDataType(type);
  [self readAlignment:elementSize];
  return [FlutterStandardTypedData
      typedDataWithData:(__bridge NSData*)[self readDataRef:elementCount * elementSize]
                   type:type];
}

- (nullable id)readValue {
  return (__bridge id)ReadValue(self);
}

static CFTypeRef ReadValue(FlutterStandardReader* codec) {
  return ReadValueOfType(codec, [codec readByte]);
}

- (nullable id)readValueOfType:(UInt8)type {
  return (__bridge id)ReadValueOfType(self, type);
}

static CFTypeRef ReadValueOfType(FlutterStandardReader* codec, UInt8 type) {
  FlutterStandardField field = (FlutterStandardField)type;
  switch (field) {
    case FlutterStandardFieldNil:
      return nil;
    case FlutterStandardFieldTrue:
      return (__bridge CFBooleanRef) @YES;
    case FlutterStandardFieldFalse:
      return (__bridge CFBooleanRef) @NO;
    case FlutterStandardFieldInt32: {
      SInt32 value;
      [codec readBytes:&value length:4];
      return (__bridge CFNumberRef) @(value);
    }
    case FlutterStandardFieldInt64: {
      SInt64 value;
      [codec readBytes:&value length:8];
      return (__bridge CFNumberRef) @(value);
    }
    case FlutterStandardFieldFloat64: {
      Float64 value;
      [codec readAlignment:8];
      [codec readBytes:&value length:8];
      return (__bridge CFNumberRef)[NSNumber numberWithDouble:value];
    }
    case FlutterStandardFieldIntHex:
    case FlutterStandardFieldString:
      return ReadUTF8(codec);
    case FlutterStandardFieldUInt8Data:
    case FlutterStandardFieldInt32Data:
    case FlutterStandardFieldInt64Data:
    case FlutterStandardFieldFloat32Data:
    case FlutterStandardFieldFloat64Data:
      return (__bridge CFTypeRef)[codec readTypedDataOfType:FlutterStandardDataTypeForField(field)];
    case FlutterStandardFieldList: {
      UInt32 length = [codec readSize];
      CFMutableArrayRef array =
          CFArrayCreateMutable(kCFAllocatorDefault, length, &kCFTypeArrayCallBacks);
      for (UInt32 i = 0; i < length; i++) {
        CFTypeRef value = ReadValue(codec);
        CFArrayAppendValue(array, (value == nil ? kCFNull : value));
      }
      return CFAutorelease(array);
    }
    case FlutterStandardFieldMap: {
      UInt32 size = [codec readSize];
      CFMutableDictionaryRef dict =
          CFDictionaryCreateMutable(kCFAllocatorDefault, size, &kCFTypeDictionaryKeyCallBacks,
                                    &kCFTypeDictionaryValueCallBacks);
      for (UInt32 i = 0; i < size; i++) {
        CFTypeRef key = ReadValue(codec);
        CFTypeRef val = ReadValue(codec);
        CFDictionaryAddValue(dict, (key == nil ? kCFNull : key), (val == nil ? kCFNull : val));
      }
      return CFAutorelease(dict);
    }
    default:
      NSCAssert(NO, @"Corrupted standard message");
  }
}
@end

@implementation FlutterStandardReaderWriter
- (FlutterStandardWriter*)writerWithData:(NSMutableData*)data {
  return [[FlutterStandardWriter alloc] initWithData:data];
}

- (FlutterStandardReader*)readerWithData:(NSData*)data {
  return [[FlutterStandardReader alloc] initWithData:data];
}
@end
