#include <chrono>
#include <cstdint>
#include <iostream>
#include "function_ref.h"

namespace {
uint64_t measureTime(const std::string& name,
                     fml::FunctionRef<void(void)> func) {
  auto start = std::chrono::system_clock::now();
  func();
  auto end = std::chrono::system_clock::now();

  uint64_t elapsed_time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << name << ": elapsed time:" << elapsed_time << "ns" << std::endl;
  return elapsed_time;
}

void fibonacciFunc(uint64_t count,
                   uint64_t start0,
                   uint64_t start1,
                   std::function<void(uint64_t)> func) {
  if (start1 < count) {
    func(start0);
    fibonacciFunc(count, start1, start1 + start0, func);
  } else {
    func(start1);
  }
}

void fibonacciRef(uint64_t count,
                  uint64_t start0,
                  uint64_t start1,
                  fml::FunctionRef<void(uint64_t)> func) {
  if (start1 < count) {
    func(start0);
    fibonacciFunc(count, start1, start1 + start0, func);
  } else {
    func(start1);
  }
}

void fibonacciConstRef(uint64_t count,
                       uint64_t start0,
                       uint64_t start1,
                       const std::function<void(uint64_t)>& func) {
  if (start1 < count) {
    func(start0);
    fibonacciFunc(count, start1, start1 + start0, func);
  } else {
    func(start1);
  }
}

}  // namespace

int main() {
  int count = 10000;
  uint64_t func_time = measureTime("std::function", [&] {
    bool is_greater = false;
    fibonacciFunc(count, 0, 1, [&](uint64_t x) { is_greater = (x > 1000LL); });
    std::cout << "is_greater: " << is_greater << std::endl;
  });
  uint64_t ref_time = measureTime("fml::FunctionRef", [&] {
    bool is_greater = false;
    fibonacciRef(count, 0, 1, [&](uint64_t x) { is_greater = (x > 1000LL); });
    std::cout << "is_greater: " << is_greater << std::endl;
  });
  std::cout << "faster: " << static_cast<double>(func_time) / ref_time << "x"
            << std::endl;
  measureTime("const std::function&", [&] {
    bool is_greater = false;
    fibonacciConstRef(count, 0, 1, [&](uint64_t x) { is_greater = (x > 1000LL); });
    std::cout << "is_greater: " << is_greater << std::endl;
  });
}
