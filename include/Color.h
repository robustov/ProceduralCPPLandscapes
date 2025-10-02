#pragma once
#include <cstdint>

struct Color {
  uint8_t r, g, b, a;
  uint32_t toPixelARGB() const;
};
