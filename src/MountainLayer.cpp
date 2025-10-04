#include "MountainLayer.h"

MountainLayer::MountainLayer(std::vector<Mountain> mountains)
    : mountains_(std::move(mountains)) {}

void MountainLayer::update(double /*dt*/) {
  // placeholder for animation
}

void MountainLayer::render(const RenderContext &ctx) {
  for (const auto &m : mountains_)
    m.paint(ctx.pixels, ctx.rowStride, ctx.winW, ctx.winH);
}
