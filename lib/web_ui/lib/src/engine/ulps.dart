// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

// This is a small library to handle stability for floating point operations.
//
// Since we are representing an infinite number of real numbers in finite
// number of bits, when we perform comparisons of coordinates for paths for
// example, we want to make sure that line and curve sections that are too
// close to each other (number of floating point numbers
// representable in bits between two numbers) are handled correctly and
// don't cause algorithms to fail when we perform operations such as
// subtraction or between checks.
//
// Small introduction into floating point comparison:
//
// For some good articles on the topic, see
// https://randomascii.wordpress.com/category/floating-point/page/2/
//
// Here is the 32 bit IEEE representation:
//   uint32_t mantissa : 23;
//   uint32_t exponent : 8;
//   uint32_t sign : 1;
// As you can see it was carefully designed to be reinterpreted as an integer.
//
// Ulps stands for unit in the last place. ulp(x) is the gap between two
// floating point numbers nearest x.

/// Converts a sign-bit int (float interpreted as int) into a 2s complement
/// int. Also converts 0x80000000 to 0. Allows result to be compared using
/// int comparison.
int signBitTo2sCompliment(int x) => (x & 0x80000000) != 0 ? (-(x & 0x7fffffff)) : x;

/// Convert a 2s compliment int to a sign-bit (i.e. int interpreted as float).
int twosComplimentToSignBit(int x) {
  if ((x & 0x80000000) == 0) {
    return x;
  }
  x = ~x + 1;
  x |= 0x80000000;
  return x;
}

class _FloatBitConverter {
  final Float32List float32List = Float32List(1);
  late Int32List int32List;
  _FloatBitConverter() {
    int32List = float32List.buffer.asInt32List(0, 1);
  }
  int toInt(Float32List source, int index) {
    float32List[0] = source[index];
    return int32List[0];
  }
  int toBits(double x) {
    float32List[0] = x;
    return int32List[0];
  }
  double toFloat(int bits) {
    int32List[0] = bits;
    return float32List[0];
  }
}

// Singleton bit converter to prevent typed array allocations.
_FloatBitConverter _floatBitConverter = _FloatBitConverter();

// Converts float to bits.
int float2Bits(Float32List source, int index) {
  return _floatBitConverter.toInt(source, index);
}

// Converts bits to float.
double bitsToFloat(int bits) {
  return _floatBitConverter.toFloat(bits);
}

const int floatBitsExponentMask = 0x7F800000;
const int floatBitsMatissaMask  = 0x007FFFFF;

/// Returns a float as 2s compliment int to be able to compare floats to each
/// other.
int floatFromListAs2sCompliment(Float32List source, int index) =>
    signBitTo2sCompliment(float2Bits(source, index));

int floatAs2sCompliment(double x) =>
    signBitTo2sCompliment(_floatBitConverter.toBits(x));

double twosComplimentAsFloat(int x) =>
    bitsToFloat(twosComplimentToSignBit(x));

bool _argumentsDenormalized(double a, double b, int epsilon) {
  double denormalizedCheck = kFltEpsilon * epsilon / 2;
  return a.abs() <= denormalizedCheck && b.abs() <= denormalizedCheck;
}

bool equalUlps(double a, double b, int epsilon, int depsilon) {
  if (_argumentsDenormalized(a, b, depsilon)) {
    return true;
  }
  int aBits = floatAs2sCompliment(a);
  int bBits = floatAs2sCompliment(b);
  // Find the difference in ULPs.
  return aBits < bBits + epsilon && bBits < aBits + epsilon;
}

bool almostEqualUlps(double a, double b) {
  const int kUlpsEpsilon = 16;
  return equalUlps(a, b, kUlpsEpsilon, kUlpsEpsilon);
}

// Equality using the same error term as between
bool almostBequalUlps(double a, double b) {
  const int kUlpsEpsilon = 2;
  return equalUlps(a, b, kUlpsEpsilon, kUlpsEpsilon);
}

bool almostPequalUlps(double a, double b) {
  const int kUlpsEpsilon = 8;
  return equalUlps(a, b, kUlpsEpsilon, kUlpsEpsilon);
}

bool almostDequalUlps(double a, double b) {
  const int kUlpsEpsilon = 16;
  return equalUlps(a, b, kUlpsEpsilon, kUlpsEpsilon);
}

bool approximatelyEqual(double ax, double ay, double bx, double by) {
  if (approximatelyEqualT(ax, bx) && approximatelyEqualT(ay, by)) {
    return true;
  }
  if (!roughlyEqualUlps(ax, bx) || !roughlyEqualUlps(ay, by)) {
    return false;
  }
  final double dx = (ax - bx);
  final double dy = (ay - by);
  double dist = math.sqrt(dx * dx + dy * dy);
  double tiniest = math.min(math.min(math.min(ax, bx), ay), by);
  double largest = math.max(math.max(math.max(ax, bx), ay), by);
  largest = math.max(largest, -tiniest);
  return almostDequalUlps(largest, largest + dist);
}

// Use this for comparing Ts in the range of 0 to 1. For general numbers (larger and smaller) use
// AlmostEqualUlps instead.
bool approximatelyEqualT(double t1, double t2) {
  return approximatelyZero(t1 - t2);
}

bool approximatelyZero(double value) => value.abs() < kFltEpsilon;

bool roughlyEqualUlps(double a, double b) {
  const int UlpsEpsilon = 256;
  const int DUlpsEpsilon = 1024;
  return equalUlps(a, b, UlpsEpsilon, DUlpsEpsilon);
}

bool dEqualUlpsEpsilon(double a, double b, int epsilon) {
  int aBits = floatAs2sCompliment(a);
  int bBits = floatAs2sCompliment(b);
  // Find the difference in ULPs.
  return aBits < bBits + epsilon && bBits < aBits + epsilon;
}

bool almostDequalUlpsDouble(double a, double b) {
  if (a.abs() < kScalarMax && b.abs() < kScalarMax) {
    return almostDequalUlps(a, b);
  }
  return (a - b).abs() / math.max(a.abs(), b.abs()) < kFltEpsilon * 16;
}

const double kFltEpsilon = 1.19209290E-07; // == 1 / (2 ^ 23)
const double kDblEpsilon = 2.22045e-16;
const double kFltEpsilonCubed = kFltEpsilon * kFltEpsilon * kFltEpsilon;
const double kFltEpsilonHalf = kFltEpsilon / 2;
const double kFltEpsilonDouble = kFltEpsilon * 2;
const double kFltEpsilonOrderableErr = kFltEpsilon * 16;
const double kFltEpsilonSquared = kFltEpsilon * kFltEpsilon;
// Use a compile-time constant for FLT_EPSILON_SQRT to avoid initializers.
// A 17 digit constant guarantees exact results.
const double kFltEpsilonSqrt = 0.00034526697709225118; // sqrt(kFltEpsilon);
const double kFltEpsilonInverse = 1 / kFltEpsilon;
const double kDblEpsilonErr = kDblEpsilon * 4;
const double kDblEpsilonSubdivideErr = kDblEpsilon * 16;
const double kRoughEpsilon = kFltEpsilon * 64;
const double kMoreRoughEpsilon = kFltEpsilon * 256;
const double kWayRoughEpsilon = kFltEpsilon * 2048;
const double kBumpEpsilon = kFltEpsilon * 4096;

// Scalar max is based on 32 bit float since [PathRef] stores values in
// Float32List.
const double kScalarMax = 3.402823466e+38;
const double kScalarMin = -kScalarMax;
