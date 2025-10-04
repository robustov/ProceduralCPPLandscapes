#pragma once
#include <cstdint>

struct Color {
  uint8_t r{0}, g{0}, b{0}, a{255};
  constexpr Color() noexcept = default;
  constexpr Color(uint8_t rr, uint8_t gg, uint8_t bb, uint8_t aa = 255) noexcept
      : r(rr), g(gg), b(bb), a(aa) {}
  [[nodiscard]] uint32_t toPixelARGB() const noexcept;
};
