#include "Color.h"

uint32_t Color::toPixelARGB() const {
  return (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) |
         uint32_t(b);
}
