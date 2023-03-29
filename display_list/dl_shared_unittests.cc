// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/dl_paint.h"

#include "flutter/display_list/utils/dl_comparable.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

class DlShareableTest : public DlShareable {
 public:
  static uint32_t GetTotalRefCount(DlShareableTest* test) {
    return test->total_ref_count();
  }
  static uint32_t GetTotalRefCount(
      const dl_shared<const DlShareableTest>& test) {
    return test->total_ref_count();
  }
  static uint32_t GetTotalRefCount(const dl_shared<DlShareableTest>& test) {
    return test->total_ref_count();
  }
};

class DlShareableHolder {
 public:
  explicit DlShareableHolder(const DlShareableTest* test) : shareable_(test) {
    total_ref_count_during_constructor_ = test->total_ref_count();
  }
  explicit DlShareableHolder(const dl_shared<DlShareableTest>& test)
      : shareable_(test) {
    total_ref_count_during_constructor_ = test->total_ref_count();
  }

  uint32_t total_ref_count() const { return shareable_->total_ref_count(); }
  uint32_t strong_ref_count() const { return shareable_->strong_ref_count(); }
  uint32_t weak_ref_count() const { return shareable_->weak_ref_count(); }

  uint32_t total_ref_count_during_constructor() const {
    return total_ref_count_during_constructor_;
  }

 private:
  const dl_shared<const DlShareableTest> shareable_;
  uint32_t total_ref_count_during_constructor_;
};

TEST(DlShared, NullShared) {
  dl_shared<DlShareableTest> empty;

  ASSERT_FALSE(empty);
  ASSERT_FALSE(empty.get());
  ASSERT_EQ(empty, nullptr);
  ASSERT_EQ(empty.get(), nullptr);
}

TEST(DlShared, MakeShared) {
  auto shareable = dl_make_shared<DlShareableTest>();
  ASSERT_TRUE(shareable);
  ASSERT_TRUE(shareable.get());
  ASSERT_EQ(shareable->total_ref_count(), 1u);
  ASSERT_EQ(shareable->strong_ref_count(), 1u);
  ASSERT_EQ(shareable->weak_ref_count(), 0u);
  ASSERT_EQ(DlShareableTest::GetTotalRefCount(shareable.get()), 1u);
  ASSERT_EQ(DlShareableTest::GetTotalRefCount(shareable), 1u);
}

TEST(DlShared, MakeSharedAsMethodArg) {
  ASSERT_EQ(dl_make_shared<DlShareableTest>()->total_ref_count(), 1u);
  ASSERT_EQ(DlShareableTest::GetTotalRefCount(
                dl_make_shared<DlShareableTest>().get()),
            1u);
  ASSERT_EQ(
      DlShareableTest::GetTotalRefCount(dl_make_shared<DlShareableTest>()), 1u);
}

TEST(DlShared, PointerAssignment) {
  auto shareable = dl_make_shared<DlShareableTest>();
  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);

  {
    dl_shared<DlShareableTest> copy;
    copy = shareable.get();
    ASSERT_TRUE(shareable);
    ASSERT_EQ(shareable->total_ref_count(), 2u);
    ASSERT_TRUE(copy);
    ASSERT_EQ(copy->total_ref_count(), 2u);
  }

  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);
}

TEST(DlShared, CopyConstructor) {
  struct Container {
    dl_shared<DlShareableTest> shareable;
  };

  {
    Container container1;
    container1.shareable = dl_make_shared<DlShareableTest>();
    ASSERT_EQ(container1.shareable->total_ref_count(), 1u);
    Container container2 = container1;
    ASSERT_EQ(container1.shareable->total_ref_count(), 2u);
    ASSERT_EQ(container2.shareable->total_ref_count(), 2u);
  }

  {
    Container container1;
    container1.shareable = dl_make_shared<DlShareableTest>();
    ASSERT_EQ(container1.shareable->total_ref_count(), 1u);
    Container container2 = container1;
    container1 = {};
    ASSERT_EQ(container2.shareable->total_ref_count(), 1u);
    container1 = container2;
    ASSERT_EQ(container1.shareable->total_ref_count(), 2u);
    ASSERT_EQ(container2.shareable->total_ref_count(), 2u);
  }
}

TEST(DlShared, SharedAssignment) {
  auto shareable = dl_make_shared<DlShareableTest>();
  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);

  {
    dl_shared<DlShareableTest> copy;
    copy = shareable;
    ASSERT_TRUE(shareable);
    ASSERT_EQ(shareable->total_ref_count(), 2u);
    ASSERT_TRUE(copy);
    ASSERT_EQ(copy->total_ref_count(), 2u);
  }

  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);
}

TEST(DlShared, FieldInitialization) {
  auto shareable = dl_make_shared<DlShareableTest>();
  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);

  {
    DlShareableHolder holder(shareable.get());
    ASSERT_TRUE(shareable);
    ASSERT_EQ(shareable->total_ref_count(), 2u);
    ASSERT_EQ(holder.total_ref_count(), 2u);
    // Field initialization from a raw pointer involves fewer inc/decrefs
    ASSERT_EQ(holder.total_ref_count_during_constructor(), 2u);
  }

  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);

  {
    DlShareableHolder holder(shareable);
    ASSERT_TRUE(shareable);
    ASSERT_EQ(shareable->total_ref_count(), 2u);
    ASSERT_EQ(holder.total_ref_count(), 2u);
    // Field initialization from a shared pointer reference
    // can avoid extra inc/decrefs
    ASSERT_EQ(holder.total_ref_count_during_constructor(), 2u);
  }

  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);
}

TEST(DlShared, MakeSharedAsConstructorArg) {
  DlShareableHolder holder(dl_make_shared<DlShareableTest>());
  ASSERT_EQ(holder.total_ref_count(), 1u);
  ASSERT_EQ(holder.total_ref_count_during_constructor(), 2u);
}

TEST(DlShared, MakeSharedAsPointerConstructorArg) {
  DlShareableHolder holder(dl_make_shared<DlShareableTest>().get());
  ASSERT_EQ(holder.total_ref_count(), 1u);
  ASSERT_EQ(holder.total_ref_count_during_constructor(), 2u);
}

TEST(DlShared, NullWeakShared) {
  dl_weak_shared<DlShareableTest> empty;

  ASSERT_FALSE(empty);
  ASSERT_EQ(empty, nullptr);
}

TEST(DlShared, PointerAssignmentToWeak) {
  auto shareable = dl_make_shared<DlShareableTest>();
  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);

  {
    dl_weak_shared<DlShareableTest> weak_copy;
    ASSERT_FALSE(weak_copy);
    weak_copy = shareable.get();
    ASSERT_TRUE(shareable);
    ASSERT_EQ(shareable->total_ref_count(), 2u);
    ASSERT_EQ(shareable->weak_ref_count(), 1u);
    ASSERT_TRUE(weak_copy);
    ASSERT_FALSE(weak_copy.is_weakly_held());
  }

  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);

  {
    dl_weak_shared<DlShareableTest> weak_copy;
    ASSERT_FALSE(weak_copy);
    weak_copy = shareable.get();
    ASSERT_TRUE(shareable);
    ASSERT_EQ(shareable->total_ref_count(), 2u);
    ASSERT_EQ(shareable->weak_ref_count(), 1u);
    ASSERT_TRUE(weak_copy);
    ASSERT_FALSE(weak_copy.is_weakly_held());
    weak_copy = nullptr;
    ASSERT_TRUE(shareable);
    ASSERT_EQ(shareable->total_ref_count(), 1u);
    ASSERT_EQ(shareable->weak_ref_count(), 0u);
    ASSERT_FALSE(weak_copy);
  }

  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);
}

TEST(DlShared, SharedAssignmentToWeak) {
  auto shareable = dl_make_shared<DlShareableTest>();
  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);

  {
    dl_weak_shared<DlShareableTest> weak_copy;
    ASSERT_FALSE(weak_copy);
    weak_copy = shareable;
    ASSERT_TRUE(shareable);
    ASSERT_EQ(shareable->total_ref_count(), 2u);
    ASSERT_EQ(shareable->weak_ref_count(), 1u);
    ASSERT_TRUE(weak_copy);
    ASSERT_FALSE(weak_copy.is_weakly_held());
  }

  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);

  {
    dl_weak_shared<DlShareableTest> weak_copy;
    ASSERT_FALSE(weak_copy);
    weak_copy = shareable;
    ASSERT_TRUE(shareable);
    ASSERT_EQ(shareable->total_ref_count(), 2u);
    ASSERT_EQ(shareable->weak_ref_count(), 1u);
    ASSERT_TRUE(weak_copy);
    ASSERT_FALSE(weak_copy.is_weakly_held());
    weak_copy = nullptr;
    ASSERT_TRUE(shareable);
    ASSERT_EQ(shareable->total_ref_count(), 1u);
    ASSERT_EQ(shareable->weak_ref_count(), 0u);
    ASSERT_FALSE(weak_copy);
  }

  ASSERT_TRUE(shareable);
  ASSERT_EQ(shareable->total_ref_count(), 1u);
}

}  // namespace testing
}  // namespace flutter
