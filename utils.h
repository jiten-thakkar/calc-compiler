#ifndef UTILS_H
#define UTILS_H

#include <chrono>

namespace timer {
class Timer {
public:
  Timer();

  // Returns the number of seconds (as a high-precision floating point number)
  // elapsed since the construction of this Timer object.
  double elapsed();

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> t1_;
};
}
#endif
