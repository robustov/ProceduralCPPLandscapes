#pragma once
#include <cstdint>

struct RenderContext {
  uint32_t *pixels;
  int rowStride; // pixels per row
  int winW;
  int winH;
};

struct Layer {
  virtual ~Layer() = default;
  virtual void update(double dt) = 0;
  virtual void render(const RenderContext &ctx) = 0;
};
