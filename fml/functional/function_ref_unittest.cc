// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Fork of webrtc function_view_unittest:
// https://github.com/webrtc-uwp/webrtc/blob/master/api/function_view_unittest.cc

#include <memory>
#include <utility>

#include "function_ref.h"
#include "gtest/gtest.h"

namespace fml {

namespace {

int CallWith33(fml::FunctionRef<int(int)> fv) {
  return fv ? fv(33) : -1;
}

int Add33(int x) {
  return x + 33;
}

}  // namespace

// Test the main use case of FunctionView: implicitly converting a callable
// argument.
TEST(FunctionRefTest, ImplicitConversion) {
  EXPECT_EQ(38, CallWith33([](int x) { return x + 5; }));
  EXPECT_EQ(66, CallWith33(Add33));
  EXPECT_EQ(-1, CallWith33(nullptr));
}

TEST(FunctionRefTest, IntIntLambdaWithoutState) {
  auto f = [](int x) { return x + 1; };
  EXPECT_EQ(18, f(17));
  fml::FunctionRef<int(int)> fv(f);
  EXPECT_TRUE(fv);
  EXPECT_EQ(18, fv(17));
}

TEST(FunctionRefTest, IntVoidLambdaWithState) {
  int x = 13;
  auto f = [x]() mutable { return ++x; };
  fml::FunctionRef<int()> fv(f);
  EXPECT_TRUE(fv);
  EXPECT_EQ(14, f());
  EXPECT_EQ(15, fv());
  EXPECT_EQ(16, f());
  EXPECT_EQ(17, fv());
}

TEST(FunctionRefTest, IntIntFunction) {
  fml::FunctionRef<int(int)> fv(Add33);
  EXPECT_TRUE(fv);
  EXPECT_EQ(50, fv(17));
}

TEST(FunctionRefTest, IntIntFunctionPointer) {
  fml::FunctionRef<int(int)> fv(&Add33);
  EXPECT_TRUE(fv);
  EXPECT_EQ(50, fv(17));
}

TEST(FunctionRefTest, Null) {
  // These two call constructors that statically construct null FunctionViews.
  EXPECT_FALSE(fml::FunctionRef<int()>());
  EXPECT_FALSE(fml::FunctionRef<int()>(nullptr));

  // This calls the constructor for function pointers.
  EXPECT_FALSE(fml::FunctionRef<int()>(reinterpret_cast<int (*)()>(0)));
}

// Ensure that FunctionView handles move-only arguments and return values.
TEST(FunctionRefTest, UniquePtrPassthrough) {
  auto f = [](std::unique_ptr<int> x) { return x; };
  fml::FunctionRef<std::unique_ptr<int>(std::unique_ptr<int>)> fv(f);
  std::unique_ptr<int> x(new int);
  int* x_addr = x.get();
  auto y = fv(std::move(x));
  EXPECT_EQ(x_addr, y.get());
}

TEST(FunctionRefTest, CopyConstructor) {
  auto f17 = [] { return 17; };
  fml::FunctionRef<int()> fv1(f17);
  fml::FunctionRef<int()> fv2(fv1);
  EXPECT_EQ(17, fv1());
  EXPECT_EQ(17, fv2());
}

TEST(FunctionRefTest, MoveConstructorIsCopy) {
  auto f17 = [] { return 17; };
  fml::FunctionRef<int()> fv1(f17);
  fml::FunctionRef<int()> fv2(std::move(fv1));  // NOLINT
  EXPECT_EQ(17, fv1());
  EXPECT_EQ(17, fv2());
}

TEST(FunctionRefTest, CopyAssignment) {
  auto f17 = [] { return 17; };
  fml::FunctionRef<int()> fv1(f17);
  auto f23 = [] { return 23; };
  fml::FunctionRef<int()> fv2(f23);
  EXPECT_EQ(17, fv1());
  EXPECT_EQ(23, fv2());
  fv2 = fv1;
  EXPECT_EQ(17, fv1());
  EXPECT_EQ(17, fv2());
}

TEST(FunctionRefTest, MoveAssignmentIsCopy) {
  auto f17 = [] { return 17; };
  fml::FunctionRef<int()> fv1(f17);
  auto f23 = [] { return 23; };
  fml::FunctionRef<int()> fv2(f23);
  EXPECT_EQ(17, fv1());
  EXPECT_EQ(23, fv2());
  fv2 = std::move(fv1);  // NOLINT
  EXPECT_EQ(17, fv1());
  EXPECT_EQ(17, fv2());
}

TEST(FunctionRefTest, Swap) {
  auto f17 = [] { return 17; };
  fml::FunctionRef<int()> fv1(f17);
  auto f23 = [] { return 23; };
  fml::FunctionRef<int()> fv2(f23);
  EXPECT_EQ(17, fv1());
  EXPECT_EQ(23, fv2());
  using std::swap;
  swap(fv1, fv2);
  EXPECT_EQ(23, fv1());
  EXPECT_EQ(17, fv2());
}

// Ensure that when you copy-construct a FunctionView, the new object points to
// the same function as the old one (as opposed to the new object pointing to
// the old one).
TEST(FunctionRefTest, CopyConstructorChaining) {
  auto f17 = [] { return 17; };
  fml::FunctionRef<int()> fv1(f17);
  fml::FunctionRef<int()> fv2(fv1);
  EXPECT_EQ(17, fv1());
  EXPECT_EQ(17, fv2());
  auto f23 = [] { return 23; };
  fv1 = f23;
  EXPECT_EQ(23, fv1());
  EXPECT_EQ(17, fv2());
}

// Ensure that when you assign one FunctionView to another, we actually make a
// copy (as opposed to making the second FunctionView point to the first one).
TEST(FunctionRefTest, CopyAssignmentChaining) {
  auto f17 = [] { return 17; };
  fml::FunctionRef<int()> fv1(f17);
  fml::FunctionRef<int()> fv2;
  EXPECT_TRUE(fv1);
  EXPECT_EQ(17, fv1());
  EXPECT_FALSE(fv2);
  fv2 = fv1;
  EXPECT_EQ(17, fv1());
  EXPECT_EQ(17, fv2());
  auto f23 = [] { return 23; };
  fv1 = f23;
  EXPECT_EQ(23, fv1());
  EXPECT_EQ(17, fv2());
}

}  // namespace fml
