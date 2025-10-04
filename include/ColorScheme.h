#include "Mountain.h" // your existing Mountain class
#include <SDL2/SDL.h>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct ColorScheme {
  uint32_t skyTop = 0xFF2878DC;
  uint32_t skyBottom = 0xFFE8F4FF;
  uint32_t mountainColor = 0xFF2E2E2E;
  uint32_t cloudColor = 0xFFFFFFFF;
};
