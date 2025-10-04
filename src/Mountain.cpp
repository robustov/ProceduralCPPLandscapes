#include "Mountain.h"
#include <algorithm>
#include <cassert>

Mountain::Mountain(MountainParams params) : params_(std::move(params)) {
  params_.validate();
  generate();
}

void Mountain::generate() {
  int targetWidth = params_.width;
  if (targetWidth < 3)
    targetWidth = 3;
  int k = 1;
  while (((1 << k) + 1) < targetWidth)
    ++k;
  int n = (1 << k) + 1;
  samples_.assign(n, 0.0);
  samples_.front() = params_.leftHeight;
  samples_.back() = params_.rightHeight;
  std::mt19937 rng(params_.seed);
  midpoint_displace(samples_, 0, n - 1, params_.initialDisplacement, rng,
                    params_.roughness);
  double mn = *std::min_element(samples_.begin(), samples_.end());
  double mx = *std::max_element(samples_.begin(), samples_.end());
  double r = mx - mn;
  if (r <= 0.0)
    r = 1.0;
  for (double &v : samples_)
    v = (v - mn) / r;
  if (n != targetWidth)
    samples_.resize(targetWidth);
}

void Mountain::regenerate(uint32_t newSeed) {
  params_.seed = newSeed;
  generate();
}

void Mountain::paint(uint32_t *pixels, int rowStride, int winW,
                     int winH) const {
  if (!pixels)
    return;
  int sampW = static_cast<int>(samples_.size());
  // simple constant color painting (one color per mountain)
  uint32_t pxColor = params_.colorARGB;
  for (int x = 0; x < winW; ++x) {
    int si = (sampW == winW)
                 ? x
                 : static_cast<int>((double(x) / double(winW)) * sampW);
    if (si < 0)
      si = 0;
    if (si >= sampW)
      si = sampW - 1;
    double s = samples_[si];
    double eff =
        params_.minHeight + s * (params_.maxHeight - params_.minHeight);
    double scaled = eff * params_.verticalSpan;
    int topY = static_cast<int>((1.0 - scaled) * double(winH)) -
               params_.verticalOffset;
    if (topY < 0)
      topY = 0;
    if (topY >= winH)
      topY = winH - 1;
    for (int y = topY; y < winH; ++y)
      pixels[y * rowStride + x] = pxColor;
  }
}

void Mountain::midpoint_displace(std::vector<double> &h, int left, int right,
                                 double disp, std::mt19937 &rng,
                                 double roughness) {
  if (right - left <= 1)
    return;
  int mid = (left + right) / 2;
  std::uniform_real_distribution<double> dist(-disp, disp);
  double midVal = 0.5 * (h[left] + h[right]) + dist(rng);
  h[mid] = midVal;
  double nextDisp = disp * roughness; // use roughness multiplier
  midpoint_displace(h, left, mid, nextDisp, rng, roughness);
  midpoint_displace(h, mid, right, nextDisp, rng, roughness);
}
