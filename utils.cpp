#include "utils.h"

namespace timer {
Timer::Timer() : t1_(std::chrono::high_resolution_clock::now()) {}

double Timer::elapsed() {
  auto t2 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = t2 - t1_;
  return elapsed.count();
}
}
