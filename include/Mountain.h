#pragma once
#include "Color.h"
#include "MountainParams.h"
#include <cstdint>
#include <random>
#include <vector>

class Mountain {
public:
  explicit Mountain(MountainParams params);
  void generate();
  void regenerate(uint32_t newSeed);
  void paint(uint32_t *pixels, int rowStride, int winW, int winH) const;
  const MountainParams &params() const noexcept { return params_; }

private:
  static void midpoint_displace(std::vector<double> &h, int left, int right,
                                double disp, std::mt19937 &rng,
                                double roughness);
  MountainParams params_;
  std::vector<double> samples_;
};
