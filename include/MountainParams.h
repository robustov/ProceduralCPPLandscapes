#pragma once
#include <cassert>
#include <cstdint>
#include <string>

struct MountainParams {
  int width = 1024; // number of horizontal samples (usually window width)
  uint32_t seed = 12345u;
  double leftHeight = 0.2;
  double rightHeight = 0.2;
  double initialDisplacement = 0.8;
  double roughness = 0.5; // multiplier per recursion
  double minHeight = 0.0;
  double maxHeight = 1.0;
  double verticalSpan = 0.75; // fraction of window height
  int verticalOffset = 0;
  uint32_t colorARGB = 0xFF1E1E1E; // default mountain color
  std::string name;

  void validate() const {
    assert(width >= 2);
    assert(minHeight <= maxHeight);
    assert(verticalSpan > 0.0 && verticalSpan <= 1.0);
    assert(roughness > 0.0 && roughness < 1.5);
  }
};
