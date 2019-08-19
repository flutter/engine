// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Foundation/Foundation.h>

#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/testing/testing.h"

@interface TestObject : NSObject
@end

static size_t sTestCreateCount = 0;
static size_t sTestCollectCount = 0;

@implementation TestObject

- (instancetype)init {
  self = [super init];

  if (self) {
    sTestCreateCount++;
  }

  return self;
}

- (void)dealloc {
  sTestCollectCount++;
}

@end

namespace fml {
namespace testing {

static_assert(__has_feature(objc_arc), "ARC must be enabled.");

TEST(ScopedNSObjectTest, TestObjectWorks) {
  auto create_count = sTestCreateCount;
  auto collect_count = sTestCollectCount;
  @autoreleasepool {
    (void)[[TestObject alloc] init];
  }
  ASSERT_EQ(create_count + 1, sTestCreateCount);
  ASSERT_EQ(collect_count + 1, sTestCollectCount);
}

TEST(ScopedNSObjectTest, ScopedNSObjectRetainsObject) {
  auto create_count = sTestCreateCount;
  auto collect_count = sTestCollectCount;

  {
    fml::scoped_nsobject<TestObject> object;

    // Nothing should happen as the object is nil.
    ASSERT_EQ(create_count, sTestCreateCount);
    ASSERT_EQ(collect_count, sTestCollectCount);

    {
      @autoreleasepool {
        object.reset([[TestObject alloc] init]);
      }  // the intermediate should be collected.

      ASSERT_EQ(create_count + 1, sTestCreateCount);
      ASSERT_EQ(collect_count, sTestCollectCount);
    }
  }  // RAII wrapper goes out of scope, thus releasing our test object.
  ASSERT_EQ(create_count + 1, sTestCreateCount);
  ASSERT_EQ(collect_count + 1, sTestCollectCount);
}

TEST(ScopedNSObjectTest, ScopedNSObjectCanBeReset) {
  auto create_count = sTestCreateCount;
  auto collect_count = sTestCollectCount;

  fml::scoped_nsobject<TestObject> object;

  @autoreleasepool {
    object.reset([[TestObject alloc] init]);
  }  // the intermediate should be collected.

  ASSERT_EQ(create_count + 1, sTestCreateCount);
  ASSERT_EQ(collect_count, sTestCollectCount);

  object.reset();

  ASSERT_EQ(create_count + 1, sTestCreateCount);
  ASSERT_EQ(collect_count + 1, sTestCollectCount);
}

TEST(ScopedNSObjectTest, SameObjectCanBeManagedByRAIIWrapper) {
  auto create_count = sTestCreateCount;
  auto collect_count = sTestCollectCount;

  fml::scoped_nsobject<TestObject> object1;
  fml::scoped_nsobject<TestObject> object2;

  @autoreleasepool {
    object1.reset([[TestObject alloc] init]);
    object2 = object1;
  }  // the intermediate should be collected.

  ASSERT_EQ(create_count + 1, sTestCreateCount);
  ASSERT_EQ(collect_count, sTestCollectCount);

  object1.reset();

  ASSERT_EQ(create_count + 1, sTestCreateCount);
  ASSERT_EQ(collect_count, sTestCollectCount);

  object2.reset();

  ASSERT_EQ(create_count + 1, sTestCreateCount);
  ASSERT_EQ(collect_count + 1, sTestCollectCount);
}

TEST(ScopedNSObjectTest, EqualityChecksUnderlyingPointer) {
  fml::scoped_nsobject<TestObject> object1;
  fml::scoped_nsobject<TestObject> object2;

  object1.reset([[TestObject alloc] init]);

  ASSERT_NE(object1, object2);

  object2 = object1;

  ASSERT_EQ(object1, object2);
}

}  // namespace testing
}  // namespace fml
