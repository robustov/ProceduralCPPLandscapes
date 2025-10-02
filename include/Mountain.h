#pragma once
#include "Color.h"
#include <cstdint>
#include <random>
#include <vector>

class Mountain {
public:
  // Parameters you can tweak
  int targetWidth; // how many horizontal samples we want (usually window width)
  double leftHeight;  // normalized starting endpoint [0..1]
  double rightHeight; // normalized ending endpoint [0..1]
  double initialDisp; // initial displacement amplitude (bigger => taller/more
                      // dramatic)
  double roughness;   // factor multiplied each recursion depth (0..1). Lower =>
                      // smoother
  double minHeight;   // minimum effective normalized height for mapping [0..1]
  double maxHeight;   // maximum effective normalized height for mapping [0..1]
  double
      verticalSpan; // fraction of window height the mountain may occupy (0..1)
  int verticalOffset;  // pixel offset applied to topY (positive moves mountain
                       // up)
  uint32_t colorPixel; // pixel value in ARGB8888 format
  uint32_t seed;       // RNG seed

  Mountain(int width = 800, uint32_t seed_ = 12345, double leftH = 0.2,
           double rightH = 0.2, double initialDisp_ = 0.8,
           double roughness_ = 0.5, double minH = 0.0, double maxH = 1.0,
           double verticalSpan_ = 0.75, int verticalOffset_ = 0,
           Color color = {30, 30, 30, 255});

  void generate();
  void regenerate(uint32_t newSeed);
  void paint(uint32_t *pixels, int rowStride, int winW, int winH) const;

private:
  std::vector<double> samples;
  static void midpoint_displace(std::vector<double> &h, int left, int right,
                                double disp, std::mt19937 &rng);
};
