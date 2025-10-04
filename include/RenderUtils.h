#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

static inline uint32_t argb(uint8_t a, uint8_t r, uint8_t g,
                            uint8_t b) noexcept {
  return (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) |
         uint32_t(b);
}

static inline void unpack_argb(uint32_t px, int &a, int &r, int &g,
                               int &b) noexcept {
  a = (px >> 24) & 0xFF;
  r = (px >> 16) & 0xFF;
  g = (px >> 8) & 0xFF;
  b = px & 0xFF;
}

static inline double clampd(double v, double lo, double hi) noexcept {
  return v < lo ? lo : (v > hi ? hi : v);
}

static inline uint32_t modulate_color(uint32_t colorARGB,
                                      double bright) noexcept {
  int a, r, g, b;
  unpack_argb(colorARGB, a, r, g, b);
  r = int(clampd(r * bright, 0.0, 255.0));
  g = int(clampd(g * bright, 0.0, 255.0));
  b = int(clampd(b * bright, 0.0, 255.0));
  return argb((uint8_t)a, (uint8_t)r, (uint8_t)g, (uint8_t)b);
}

static inline void fill_sky_gradient(uint32_t *pixels, int rowStride, int winW,
                                     int winH, uint32_t topColor,
                                     uint32_t bottomColor) {
  for (int y = 0; y < winH; ++y) {
    double t = double(y) / double(std::max(1, winH - 1));
    int a1 = (topColor >> 24) & 0xFF, r1 = (topColor >> 16) & 0xFF,
        g1 = (topColor >> 8) & 0xFF, b1 = topColor & 0xFF;
    int a2 = (bottomColor >> 24) & 0xFF, r2 = (bottomColor >> 16) & 0xFF,
        g2 = (bottomColor >> 8) & 0xFF, b2 = bottomColor & 0xFF;
    uint32_t a = uint32_t((1.0 - t) * a1 + t * a2 + 0.5);
    uint32_t r = uint32_t((1.0 - t) * r1 + t * r2 + 0.5);
    uint32_t g = uint32_t((1.0 - t) * g1 + t * g2 + 0.5);
    uint32_t b = uint32_t((1.0 - t) * b1 + t * b2 + 0.5);
    uint32_t rowColor = (a << 24) | (r << 16) | (g << 8) | b;
    uint32_t *row = pixels + y * rowStride;
    for (int x = 0; x < winW; ++x)
      row[x] = rowColor;
  }
}
