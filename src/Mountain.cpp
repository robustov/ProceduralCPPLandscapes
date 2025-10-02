#include "Mountain.h"
#include <algorithm>
#include <cassert>
#include <random>

Mountain::Mountain(int width, uint32_t seed_, double leftH, double rightH,
                   double initialDisp_, double roughness_, double minH,
                   double maxH, double verticalSpan_, int verticalOffset_,
                   Color color)
    : targetWidth(width), leftHeight(leftH), rightHeight(rightH),
      initialDisp(initialDisp_), roughness(roughness_), minHeight(minH),
      maxHeight(maxH), verticalSpan(verticalSpan_),
      verticalOffset(verticalOffset_), colorPixel(color.toPixelARGB()),
      seed(seed_) {
  assert(minHeight <= maxHeight);
  generate();
}

void Mountain::generate() {
  if (targetWidth < 3)
    targetWidth = 3;
  // choose k so that (1<<k) + 1 >= targetWidth
  int k = 1;
  while (((1 << k) + 1) < targetWidth)
    ++k;
  int n = (1 << k) + 1;
  samples.assign(n, 0.0);
  samples.front() = leftHeight;
  samples.back() = rightHeight;
  std::mt19937 rng(seed);
  midpoint_displace(samples, 0, n - 1, initialDisp, rng);
  // normalize to [0..1]
  double mn = *std::min_element(samples.begin(), samples.end());
  double mx = *std::max_element(samples.begin(), samples.end());
  double r = mx - mn;
  if (r <= 0.0)
    r = 1.0;
  for (double &v : samples)
    v = (v - mn) / r;
  // we only store first targetWidth samples for convenience (window sampling)
  if (n != targetWidth)
    samples.resize(targetWidth);
}

void Mountain::regenerate(uint32_t newSeed) {
  seed = newSeed;
  generate();
}

void Mountain::paint(uint32_t *pixels, int rowStride, int winW,
                     int winH) const {
  if (!pixels)
    return;
  // If our samples are not exactly winW wide, sample proportionally.
  // But normally we generate with targetWidth == winW.
  const int sampW = (int)samples.size();
  for (int x = 0; x < winW; ++x) {
    // sample index (clamp)
    int si = (sampW == winW) ? x : int((double(x) / double(winW)) * sampW);
    if (si < 0)
      si = 0;
    if (si >= sampW)
      si = sampW - 1;
    double s = samples[si]; // normalized [0..1]
    // map to effective height in [minHeight, maxHeight]
    double eff = minHeight + s * (maxHeight - minHeight); // still [..]
    // compute topY
    // higher eff => taller mountain => smaller y (closer to top)
    double scaled = eff * verticalSpan; // fraction of window height
    int topY = int((1.0 - scaled) * double(winH)) - verticalOffset;
    if (topY < 0)
      topY = 0;
    if (topY >= winH)
      topY = winH - 1;
    // Fill column x from topY down to bottom
    uint32_t pxColor = colorPixel;
    for (int y = topY; y < winH; ++y) {
      pixels[y * rowStride + x] = pxColor;
    }
  }
}

void Mountain::midpoint_displace(std::vector<double> &h, int left, int right,
                                 double disp, std::mt19937 &rng) {
  if (right - left <= 1)
    return;
  int mid = (left + right) / 2;
  std::uniform_real_distribution<double> dist(-disp, disp);
  double midVal = 0.5 * (h[left] + h[right]) + dist(rng);
  h[mid] = midVal;
  // The roughness multiplier will be handled by caller by passing smaller
  // `disp` into recursive calls.
  double nextDisp = disp * 0.5; // we will later expose a different roughness by
                                // changing initialDisp or recursion multiplier
  midpoint_displace(h, left, mid, nextDisp, rng);
  midpoint_displace(h, mid, right, nextDisp, rng);
}
