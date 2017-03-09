// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FlutterStandardCodec_Internal.h"

#pragma mark - Codec for basic message channel

@implementation FlutterStandardMessageCodec
+ (instancetype) shared {
  static id _shared = nil;
  if (!_shared) {
     _shared = [FlutterStandardMessageCodec new];
  }
  return _shared;
}

- (NSData*) encode:(id)message {
  NSMutableData* data = [NSMutableData dataWithCapacity:32];
  FlutterStandardWriter* writer = [FlutterStandardWriter withData:data];
  [writer writeValue:message];
  return data;
}

- (id) decode:(NSData*)message {
  FlutterStandardReader* reader = [FlutterStandardReader withData:message];
  id value = [reader readValue];
  if ([reader hasMore]) {
    @throw [NSException exceptionWithName:NSInvalidArgumentException
                                   reason:@"Message corrupted"
                                 userInfo:nil];
  }
  return value;
}
@end

#pragma mark - Codec for method channel

@implementation FlutterStandardMethodCodec
+ (instancetype) shared {
  static id _shared = nil;
  if (!_shared) {
     _shared = [FlutterStandardMethodCodec new];
  }
  return _shared;
}

- (NSData*) encodeSuccessEnvelope:(id)result {
  NSMutableData* data = [NSMutableData dataWithCapacity:32];
  FlutterStandardWriter* writer = [FlutterStandardWriter withData:data];
  [writer writeByte:0];
  [writer writeValue:result];
  return data;
}

- (NSData*) encodeErrorEnvelope:(FlutterError*)error {
  NSMutableData* data = [NSMutableData dataWithCapacity:32];
  FlutterStandardWriter* writer = [FlutterStandardWriter withData:data];
  [writer writeByte:1];
  [writer writeValue:error.code];
  [writer writeValue:error.message];
  [writer writeValue:error.details];
  return data;
}

- (FlutterMethodCall*) decodeMethodCall:(NSData*)message {
  FlutterStandardReader* reader = [FlutterStandardReader withData:message];
  id value1 = [reader readValue];
  id value2 = [reader readValue];
  if ([reader hasMore]) {
    @throw [NSException exceptionWithName:NSInvalidArgumentException
                                   reason:@"Message corrupted"
                                 userInfo:nil];
  }
  if (![value1 isKindOfClass:[NSString class]]) {
    @throw [NSException exceptionWithName:NSInvalidArgumentException
                                   reason:@"Method call corrupted"
                                 userInfo:nil];
  }
  return [FlutterMethodCall withMethod:value1 andArguments:value2];
}
@end

using namespace shell;

#pragma mark - Standard serializable types

@implementation FlutterStandardTypedData
+ (instancetype)withBytes:(NSData*)data {
  return [[[FlutterStandardTypedData alloc] initWithData:data ofType:kBYTE_ARRAY elementSize:1] autorelease];
}

+ (instancetype)withInt32:(NSData*)data {
  return [[[FlutterStandardTypedData alloc] initWithData:data ofType:kINT32_ARRAY elementSize:4] autorelease];
}

+ (instancetype)withInt64:(NSData*)data {
  return [[[FlutterStandardTypedData alloc] initWithData:data ofType:kINT64_ARRAY elementSize:8] autorelease];
}

+ (instancetype)withFloat64:(NSData*)data {
  return [[[FlutterStandardTypedData alloc] initWithData:data ofType:kFLOAT64_ARRAY elementSize:8] autorelease];
}

+ (instancetype)withData:(NSData*)data ofType:(UInt8)type {
  switch (type) {
    case kBYTE_ARRAY: return [FlutterStandardTypedData withBytes:data];
    case kINT32_ARRAY: return [FlutterStandardTypedData withInt32:data];
    case kINT64_ARRAY: return [FlutterStandardTypedData withInt64:data];
    case kFLOAT64_ARRAY: return [FlutterStandardTypedData withFloat64:data];
    default:
      @throw [NSException exceptionWithName:NSInvalidArgumentException
                                     reason:@"Invalid type"
                                   userInfo:nil];

  }
}

- (instancetype)initWithData:(NSData*)data ofType:(UInt8)type elementSize:(UInt8)elementSize {
  if (self = [super init]) {
    _data = [data retain];
    _type = type;
    _elementSize = elementSize;
    if (_data.length % _elementSize) {
      @throw [NSException exceptionWithName:NSInvalidArgumentException
                                     reason:@"Invalid byte count for element size"
                                   userInfo:nil];
    }
  }
  return self;
}

- (void) dealloc {
  [_data release];
  [super dealloc];
}
@end

@implementation FlutterStandardBigInteger
+ (instancetype) withHex:(NSString*)hex {
  return [[[FlutterStandardBigInteger alloc] initWithHex:hex] autorelease];
}

- (instancetype) initWithHex:(NSString*)hex {
  if (self = [super init]) {
    _hex = [hex retain];
  }
  return self;
}

- (void) dealloc {
  [_hex release];
  [super dealloc];
}
@end

#pragma mark - Writer and reader of standard codec

@implementation FlutterStandardWriter {
  NSMutableData* _data;
}

+ (instancetype) withData:(NSMutableData*)data {
  FlutterStandardWriter* writer = [[FlutterStandardWriter alloc] initWithData:data];
  [writer autorelease];
  return writer;
}

- (instancetype)initWithData:(NSMutableData*)data {
  if (self = [super init]) {
    _data = [data retain];
  }
  return self;
}

- (void) dealloc {
  [_data release];
  [super dealloc];
}

- (void) writeByte:(UInt8)value {
  [_data appendBytes:&value length:1];
}

- (void) writeSize:(UInt32)size {
  if (size < 254) {
    [self writeByte:(UInt8)size];
  } else if (size <= 0xffff) {
    [self writeByte:254];
    UInt16 value = (UInt16) size;
    [_data appendBytes:&value length:2];
  } else {
    [self writeByte:255];
    [_data appendBytes:&size length:4];
  }
}

- (void) writeAlignment:(UInt8)alignment {
  UInt8 mod = _data.length % alignment;
  if (mod) {
    for (int i = 0; i < (alignment - mod); i++) {
      [self writeByte:0];
    }
  }
}

- (void) writeUTF8:(NSString*)value {
  UInt32 length = [value lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
  [self writeSize:length];
  [_data appendBytes:value.UTF8String length:length];
}

- (void) writeValue:(id)value {
  if (value == nil || value == [NSNull null]) {
    [self writeByte:kNIL];
  } else if ([value isKindOfClass:[NSNumber class]]) {
    NSNumber* number = value;
    const char* type = [number objCType];
    if (strcmp(type, @encode(BOOL)) == 0) {
      BOOL b = number.boolValue;
      [self writeByte:(b ? kTRUE : kFALSE)];
    } else if (strcmp(type, @encode(int)) == 0) {
      SInt32 n = number.intValue;
      [self writeByte:kINT32];
      [_data appendBytes:(UInt8*)&n length:4];
    } else if (strcmp(type, @encode(long)) == 0) {
      SInt64 n = number.longValue;
      [self writeByte:kINT64];
      [_data appendBytes:(UInt8*)&n length:8];
    } else if (strcmp(type, @encode(double)) == 0) {
      Float64 f = number.doubleValue;
      [self writeByte:kFLOAT64];
      [_data appendBytes:(UInt8*)&f length:8];
    } else {
      @throw [NSException exceptionWithName:NSInvalidArgumentException
                                     reason:@"Unsupported value"
                                   userInfo:nil];
    }
  } else if ([value isKindOfClass:[NSString class]]) {
    NSString* string = value;
    [self writeByte:kSTRING];
    [self writeUTF8:string];
  } else if ([value isKindOfClass:[FlutterStandardBigInteger class]]) {
    FlutterStandardBigInteger* bigInt = value;
    [self writeByte:kINTHEX];
    [self writeUTF8:bigInt.hex];
  } else if ([value isKindOfClass:[FlutterStandardTypedData class]]) {
    FlutterStandardTypedData* typedData = value;
    [self writeByte:typedData.type];
    [self writeSize:typedData.length];
    [self writeAlignment:typedData.elementSize];
    [_data appendData:typedData.data];
  } else if ([value isKindOfClass:[NSArray class]]) {
    NSArray* array = value;
    [self writeByte:kLIST];
    [self writeSize:array.count];
    for (id object in array) {
      [self writeValue:object];
    }
  } else if ([value isKindOfClass:[NSArray class]]) {
    NSDictionary* dict = value;
    [self writeByte:kMAP];
    [self writeSize:dict.count];
    for (id key in dict) {
      [self writeValue:key];
      [self writeValue:[dict objectForKey:key]];
    }
  } else {
    @throw [NSException exceptionWithName:NSInvalidArgumentException
                                   reason:@"Unsupported value"
                                 userInfo:nil];
  }
}
@end

@implementation FlutterStandardReader {
  NSData* _data;
  NSRange _range;
}

+ (instancetype) withData:(NSData*)data {
  FlutterStandardReader* reader = [[FlutterStandardReader alloc] initWithData:data];
  [reader autorelease];
  return reader;
}

- (instancetype) initWithData:(NSData*)data {
  if (self = [super init]) {
    _data = [data retain];
    _range = NSMakeRange(0, 0);
  }
  return self;
}

- (void) dealloc {
  [_data release];
  [super dealloc];
}

- (BOOL) hasMore {
  return _range.location < _data.length;
}

- (void) readBytes:(void*)destination length:(int)length {
  _range.length = length;
  [_data getBytes:destination range:_range];
  _range.location += _range.length;
}

- (UInt8) readByte {
  UInt8 value;
  [self readBytes:&value length:1];
  return value;
}

- (UInt32) readSize {
  UInt8 byte = [self readByte];
  if (byte < 254) {
    return (UInt32) byte;
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

- (NSData*) readData:(int)length {
  _range.length = length;
  NSData* data = [_data subdataWithRange:_range];
  _range.location += _range.length;
  return data;
}

- (NSString*) readUTF8 {
  NSData* bytes = [self readData:[self readSize]];
  NSString* string = [[NSString alloc] initWithData:bytes encoding:NSUTF8StringEncoding];
  [string autorelease];
  return string;
}

- (void) readAlignment:(UInt8)alignment {
  UInt8 mod = _range.location % alignment;
  if (mod) {
    _range.location += (alignment - mod);
  }
}

- (FlutterStandardTypedData*) readTypedDataOfType:(UInt8)type withElementSize:(UInt8)elementSize {
  UInt32 elementCount = [self readSize];
  [self readAlignment:elementSize];
  NSData* data = [self readData:elementCount*elementSize];
  return [FlutterStandardTypedData withData:data ofType:type];
}

- (id) readValue {
  UInt8 type = [self readByte];
  switch (type) {
  case kNIL: return nil;
  case kTRUE: return @YES;
  case kFALSE: return @NO;
  case kINT32: {
    SInt32 value;
    [self readBytes:&value length: 4];
    return [NSNumber numberWithInt:value];
  }
  case kINT64: {
    SInt64 value;
    [self readBytes:&value length: 8];
    return [NSNumber numberWithLong:value];
  }
  case kFLOAT64: {
    Float64 value;
    [self readBytes:&value length: 8];
    return [NSNumber numberWithDouble:value];
  }
  case kINTHEX: return [FlutterStandardBigInteger withHex:[self readUTF8]];
  case kSTRING: return [self readUTF8];
  case kBYTE_ARRAY: return [self readTypedDataOfType:kBYTE_ARRAY withElementSize:1];
  case kINT32_ARRAY: return [self readTypedDataOfType:kINT32_ARRAY withElementSize:4];
  case kINT64_ARRAY: return [self readTypedDataOfType:kINT64_ARRAY withElementSize:8];
  case kFLOAT64_ARRAY:  return [self readTypedDataOfType:kFLOAT64_ARRAY withElementSize:8];
  case kLIST: {
    UInt32 length = [self readSize];
    NSMutableArray* array = [NSMutableArray arrayWithCapacity:length];
    for (UInt32 i = 0; i < length; i++) {
      [array addObject:[self readValue]];
    }
    return array;
  }
  case kMAP: {
    UInt32 size = [self readSize];
    NSMutableDictionary* dict = [NSMutableDictionary dictionaryWithCapacity:size];
    for (UInt32 i = 0; i < size; i++) {
      id key = [self readValue];
      id val = [self readValue];
      [dict setObject:(val == nil ? [NSNull null] : val)
               forKey:(key == nil ? [NSNull null] : key)];
    }
    return dict;
  }
  default:
    @throw [NSException exceptionWithName:NSInvalidArgumentException
                                   reason:@"Message corrupted"
                                 userInfo:nil];
  }
}
@end
